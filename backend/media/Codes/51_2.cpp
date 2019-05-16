#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <cmath>

//#define VOCATION_SNIPER 1
//#define VOCATION_HACK 1
#define VOCATION_MEDIC 1
//#define VOCATION_SIGNALMAN 1	

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

struct ei
{
	int player_id;
	int info_frame;
	double x;
	double y;
};

const int INF = 1000000;
const double PI = 3.14159265358979;
const ITEM WeaponPriority_Fatal[10] = { SNIPER_BARRETT, SNIPER_RILFE, TIGER_BILLOW_HAMMER, MACHINE_GUN, ASSAULT_RIFLE, CROSSBOW, SEMI_AUTOMATIC_RILE, FIST, SUBMACHINE_GUN, HAND_GUN };
const ITEM WeaponPriority_Normal[10] = { SNIPER_BARRETT, SNIPER_RILFE, MACHINE_GUN, SUBMACHINE_GUN, ASSAULT_RIFLE, TIGER_BILLOW_HAMMER, SEMI_AUTOMATIC_RILE, FIST, HAND_GUN, CROSSBOW };
std::string Name;
Item target_item;
ITEM weapon;
OtherInfo target_enemy;
std::string now_policy;
std::string last_policy;
std::vector<ei> enemy_info;
std::vector<ei> t_enemy_info;
int heal_item = 0;
int last_shoot = -1;
int max_range = 1;
int Attack_Count = 0;
int sound_time = 0;
int signal_man_ID = -1;
int medic_man_ID = -1;
int block_time = 0;
double now_target_dir;
double new_target_dir;
double now_poison_dis;
double new_view_angle;
double new_move_angle;
double tem_pos_x = -1;
double tem_pos_y = -1;
double last_pos_x = -1;
double last_pos_y = -1;
double last_hp;
bool ifsendID = false;
bool ifnewinfo = false;
bool ifchecksound = false;
bool ifchangeview = true;
bool ifaction = false;
Sound current_sound;

inline double Trans(double angle)
{
	while (angle >= 360) angle -= 360;
	while (angle < 0) angle += 360;
	return angle;
}

inline double Get_Absolute_Direction(double target_angle)
{
	return Trans(target_angle + info.self.view_angle);
}


void debug(std::string cmd)
{
	std::fstream f(Name + ".txt", std::ios::app);
	if (cmd == "Start")
	{
		f << "=====   " << frame << "   =====" << std::endl;
		f << "HP  " << info.self.hp << "   POS  " << info.self.xy_pos.x
			<< "  " << info.self.xy_pos.y << "   STATUS  " << info.self.status << std::endl;
		f << "Attack CD  " << info.self.attack_cd << std::endl;
		f << "View Angle  " << info.self.view_angle << "  Move Angle  " << info.self.move_angle << std::endl;
		f << "View Width  " << info.self.view_width << "  View Distance  " << info.self.view_distance << std::endl;
		f << "Poison Center Radius  " << info.poison.current_radius << "  Poison Distance  " << now_poison_dis << std::endl;
		f << "Poison Center " << info.poison.current_center.x << "  " << info.poison.current_center.y << std::endl;
		f << "LastPolicy  " << now_policy << std::endl;
		f << "BlockTime  " << block_time << std::endl;
		if (ifchecksound) f << "CheckSound  " << Get_Absolute_Direction(new_view_angle) << std::endl;
		for (int i = 0; i < info.self.bag.size(); ++i)
			f << "Item  " << i << "  " << info.self.bag[i].type
			<< "  Durability  " << info.self.bag[i].durability << std::endl;
	}
	if (cmd == "END")
		f << "=====   END   =====" << std::endl << std::endl;
	if (cmd == "MOVE")
		f << "** MOVE  WITH  " << "  MoveAngle  " << Trans(new_move_angle + info.self.view_angle) << "  ViewAngle  " << Trans(new_view_angle + info.self.view_angle) << std::endl;
	if (cmd == "ATTACK")
		f << "** ATTACK  " << target_enemy.player_ID << " WITH " << weapon << "  **" << std::endl;
	if (cmd == "PICK")
		f << "** PICK  " << target_item.type << "  **" << std::endl;
	if (cmd == "HEAL")
		f << "** HEAL **" << std::endl;
	if (cmd == "DIRECT")
		f << "** DIRECT **  " << now_target_dir << "  " << new_target_dir << std::endl;
	if (cmd == "RADIO")
		f << "** RADIO **  " << current_sound.sender << "  " << current_sound.parameter << std::endl;
}

inline double Get_Relate_Direction(double target_angle)
{
	return Trans(target_angle - info.self.view_angle);
}

inline double Get_Distance(double x1, double y1, double x2, double y2)
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

inline double Get_New_Move_Direction(double x, double y)
{
	double dir;
	dir = atan((y - info.self.xy_pos.y) / (x - info.self.xy_pos.x)) / PI * 180;
	if (info.self.xy_pos.x > x) dir += 180;
	return Get_Relate_Direction(dir);
}

inline XYPosition Get_Absolute_Position(double angle, double distance)
{
	XYPosition P;
	angle = Get_Absolute_Direction(angle);
	P.x = info.self.xy_pos.x + cos(angle) * distance;
	P.y = info.self.xy_pos.x + sin(angle) * distance;
	return P;
}

inline int Get_Area()
{
	return int(info.self.xy_pos.y / 10) + int(info.self.xy_pos.x / 100) + 1;
}

inline double Shift_Angle_Calc()
{
	double d1 = target_enemy.polar_pos.distance;
	double d2 = target_enemy.move_speed;
	double sum = d1 + d2;
	double angle_1 = Get_Relate_Direction(target_enemy.move_angle) - target_enemy.polar_pos.angle;
	double s = (d2 / sum) * sin((angle_1 / 180) * PI);
	return (asin(s) / PI) * 180;
}

inline void Radio_Send()
{
	std::vector<ei> enemy_INFO = enemy_info;
	std::sort(enemy_INFO.begin(), enemy_INFO.end(), [](ei A, ei B)
	{
		return A.info_frame > B.info_frame;
	});
	int msg = 0;
	msg += (frame - enemy_INFO[0].info_frame) * 100000000;
	msg += int(enemy_INFO[0].x) * 100000;
	msg += int(enemy_INFO[0].y) * 100;
	msg += enemy_INFO[0].player_id;
	if (info.self.vocation != SIGNALMAN) radio(signal_man_ID, msg);
	else
	{
		for (int i = 0; i < teammates.size(); ++i)
		{
			if (teammates[i] == info.player_ID) continue;
			radio(teammates[i], msg);
		}
	}
	ifnewinfo = false;
}

inline void Block_Check()
{
	for (int i = 0; i < info.landform_IDs.size(); ++i)
	{
		block b = get_landform(info.landform_IDs[i]);
		double disX, disY;
		XYPosition new_pos;
		if (b.shape == RECTANGLE && tem_pos_x == -1 && tem_pos_y == -1)
		{
			disX = info.self.xy_pos.x + 2 * cos(Get_Absolute_Direction(new_move_angle));
			disY = info.self.xy_pos.y + 2 * sin(Get_Absolute_Direction(new_move_angle));
			if (b.x0 <= disX && disX <= b.x1 && b.y1 <= disY && disY <= b.y0)
			{
				if (info.self.xy_pos.x < b.x0)
				{
					if (Get_Absolute_Direction(new_move_angle) < 180) new_pos = {double(b.x0 - 5), double(b.y0 + 10)};
					else new_pos = { double(b.x0 - 5), double(b.y1 - 10) };
				}
				if (info.self.xy_pos.x > b.x1)
				{
					if (Get_Absolute_Direction(new_move_angle) < 180) new_pos = { double(b.x1 + 5), double(b.y0 + 10) };
					else new_pos = { double(b.x1 + 5), double(b.y1 - 10) };
				}
				if (info.self.xy_pos.y > b.y0)
				{
					if (Get_Absolute_Direction(new_move_angle) < 270) new_pos = { double(b.x0 - 10), double(b.y0 + 5) };
					else new_pos = { double(b.x1 + 10), double(b.y0 + 5) };
				}
				if (info.self.xy_pos.y < b.y1)
				{
					if (Get_Absolute_Direction(new_move_angle) < 90) new_pos = { double(b.x1 + 5), double(b.y1 - 10)};
					else new_pos = { double(b.x0 - 10), double(b.y1 - 5) };
				}
				tem_pos_x = new_pos.x;
				tem_pos_y = new_pos.y;
			}
		}
		else if (b.shape == CIRCLE && block_time == 0)
		{
			if (Get_Distance(disX, disY, b.x0, b.y0) - b.r < 3)
			{
				tem_pos_x = b.x0 - b.r - 5;
				tem_pos_y = b.y0;
			}
		}
	}
}

inline void Weapon_Update()
{
	heal_item = 0;
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		if (info.self.bag[i].type < 10 && info.self.bag[i].durability > 0)
			max_range = std::max(max_range, ITEM_DATA[info.self.bag[i].type].range);
		if (info.self.bag[i].type == FIRST_AID_CASE || info.self.bag[i].type == BONDAGE)
			heal_item += info.self.bag[i].durability;
	}
}

inline bool Weapon_Check(ITEM & this_weapon)	
{
	for (int i = 0; i < info.self.bag.size(); ++i)
		if (info.self.bag[i].type == this_weapon && info.self.bag[i].durability > 0)
			return true;
	return false;
}

inline bool Pick_Check()
{
	if (target_item.polar_pos.distance < 1)
	{
		if (target_item.type == CODE_CASE && info.self.vocation != HACK) return false;
		pickup(target_item.item_ID);
		debug("PICK");
		return true;
	}
	return false;
}

inline bool Heal_Check()
{
	if (info.self.attack_cd > 0) return false;
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		if (info.self.bag[i].type == FIRST_AID_CASE && info.self.bag[i].durability > 0 &&  info.self.hp < 30) 
		{
			shoot(FIRST_AID_CASE, info.self.view_angle);
			last_shoot = 1;
			debug("HEAL");
			return true;
		}
	}
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		if (info.self.bag[i].type == BONDAGE && info.self.bag[i].durability > 0 && info.self.hp < info.self.hp_limit - 20)
		{
			shoot(BONDAGE, info.self.view_angle);
			last_shoot = 1;
			debug("HEAL");
			return true;
		}
	}
	return false;
}

inline void Move_Check(std::string cmd)
{
	if (Pick_Check()) return;
	if ((info.self.status == MOVING || info.self.status == MOVING_SHOOTING) && (last_policy == now_policy) && (block_time == 0))
	{
		if (info.self.move_cd > 1 && ifnewinfo) Radio_Send();
		else if (info.self.move_cd > 1 && ifchangeview) move(new_move_angle, new_view_angle, 0);
		else if (last_policy == "FIGHT MOVE") move(new_move_angle, Trans(target_enemy.polar_pos.angle), 0);
		else move(new_move_angle, new_move_angle, 0);
		return;
	}
	ifchangeview = true;
	if (cmd == "RUN")
	{
		if (now_policy == "ToNowCenter" || now_policy == "NoSpecialPolicy") new_move_angle = now_target_dir;
		if (now_policy == "ToNewCenter") new_move_angle = new_target_dir;
		if (now_policy == "FindEnemy")
		{
			new_move_angle = Get_New_Move_Direction(t_enemy_info[0].x, t_enemy_info[0].y);
			if (!ifchecksound) new_view_angle = new_move_angle;
			if (Get_Distance(t_enemy_info[0].x, t_enemy_info[0].y, info.self.xy_pos.x, info.self.xy_pos.y) - max_range < 10) ifchangeview = false;
		}
	}
	if (cmd == "GET ITEM")
	{
		new_move_angle = Trans(target_item.polar_pos.angle);
		new_view_angle = new_move_angle;
		if (target_item.polar_pos.distance < 3) ifchangeview = false;
	}
	if (cmd == "FIGHT MOVE")
	{
		new_move_angle = Trans(target_enemy.polar_pos.angle - 90);
		new_view_angle = Trans(target_enemy.polar_pos.angle);
		ifchangeview = false;
	}
	Block_Check();
	if (tem_pos_x != -1 && tem_pos_y != -1) new_move_angle = Get_New_Move_Direction(tem_pos_x, tem_pos_y);
	new_move_angle = Trans(new_move_angle + double(22 * block_time));
	move(new_move_angle, new_view_angle);
	debug("MOVE");
}

inline bool Attack_Check()
{
	bool ifattack = false;
	for (int i = 0; i < 10; ++i)
	{
		weapon = WeaponPriority_Normal[i];
		if (!Weapon_Check(weapon)) continue;
		if (ITEM_DATA[weapon].range < target_enemy.polar_pos.distance) continue;
		ifattack = true;
		break;
	}
	if (ifattack && info.self.attack_cd == 0)
	{
		double target_angle = target_enemy.polar_pos.angle;
		target_angle = Trans(target_angle + Shift_Angle_Calc());
		shoot(weapon, target_angle);
		last_shoot = 0;
		Attack_Count ++;
		debug("ATTACK");
		return true;
	}
	return false;
}

inline void Item_Check()
{
	target_item.polar_pos.distance = INF;
	for (int i = 0; i < info.items.size(); ++i)
		if (info.items[i].polar_pos.distance < target_item.polar_pos.distance && (info.items[i].type != CODE_CASE || info.self.vocation == HACK))
			target_item = info.items[i];
}

inline void Poison_Check()
{
	double cx, cy, nx, ny;
	if (info.poison.next_center.x == 0)
	{
		cx = nx = 500;
		cy = ny = 500;
	}
	else
	{
		cx = info.poison.current_center.x;
		cy = info.poison.current_center.y;
		nx = info.poison.next_center.x;
		ny = info.poison.next_center.y;
	}
	now_target_dir = Get_New_Move_Direction(cx, cy);
	new_target_dir = Get_New_Move_Direction(nx, ny);
	now_poison_dis = Get_Distance(cx, cy, info.self.xy_pos.x, info.self.xy_pos.y);
	debug("DIRECT");
}

inline void Poison_Warning()
{
	if (info.poison.current_radius - now_poison_dis < 400)
	{
		if (info.poison.move_flag == 1) now_policy = "ToNewCenter";
		else now_policy = "ToNowCenter";
		return;
	}
	return;
}

inline bool Check_Enemy(int i)
{
	for (int j = 0; j < teammates.size(); ++j)
		if (info.others[i].player_ID == teammates[j])
			return false;
	return true;
}

inline void Enemy_Check()
{
	t_enemy_info = enemy_info;
	std::sort(t_enemy_info.begin(), t_enemy_info.end(), [](ei A, ei B)
	{
		return A.info_frame > B.info_frame;
	});
	if ((frame - t_enemy_info[0].info_frame < 10) && (Get_Distance(t_enemy_info[0].x, t_enemy_info[0].y, info.self.xy_pos.x, info.self.xy_pos.y)) < max_range + 10)
		now_policy = "FindEnemy";
}

inline void Sound_Check()
{
	int delay_time_foot = INF;
	int delay_time_gun = INF;
	int angle_foot, angle_gun;
	for (int i = 0; i < info.sounds.size(); ++i)
		if (info.sounds[i].type == RADIO_VOICE)
		{
			current_sound = info.sounds[i];
			debug("RADIO");
			if (info.sounds[i].parameter == 0)
			{
				if (info.self.vocation != SIGNALMAN) signal_man_ID = info.sounds[i].sender;
				else
				{
					medic_man_ID = info.sounds[i].sender;
					for (int i = 0; i < teammates.size(); ++i)
					{
						if (teammates[i] == info.player_ID) continue;
						radio(teammates[i], medic_man_ID);
					}
					ifaction = true;
				}
			}
			else if (info.sounds[i].parameter < 100)
				medic_man_ID = info.sounds[i].parameter;
			else
			{
				ei new_enemy = { info.sounds[i].parameter % 100,
					frame - int(info.sounds[i].parameter / 100000000) - 1,
					double((info.sounds[i].parameter / 100000) % 1000), 
					double((info.sounds[i].parameter / 100) % 1000),};
				enemy_info[new_enemy.player_id] = new_enemy;
				if (info.self.vocation == SIGNALMAN) ifnewinfo = true;
				if (info.self.vocation == MEDIC)
				{
					int dis = Get_Distance(new_enemy.x, new_enemy.y, info.poison.current_center.x, info.poison.current_center.y);
					if (heal_item > 0 && new_enemy.player_id == info.sounds[i].sender && Get_Distance(new_enemy.x, new_enemy.y, info.self.xy_pos.x, info.self.xy_pos.y) < 50
							&&  dis < info.poison.current_radius - 100)
					{
						if (dis < 1 && info.self.attack_cd == 0)
						{
							bool flag = false;
							if (info.others[i].player_ID != new_enemy.player_id) continue;
							for (int i = 0; i < info.self.bag.size(); ++i)
								if (info.self.bag[i].type == FIRST_AID_CASE && info.self.bag[i].durability > 0)
								{
									shoot(FIRST_AID_CASE, info.self.view_angle, new_enemy.player_id);
									last_shoot = 1;
									heal_item--;
									flag = true;
									debug("HEAL");	
								}
							if (!flag)
								for (int i = 0; i < info.self.bag.size(); ++i)
									if (info.self.bag[i].type == BONDAGE && info.self.bag[i].durability > 0)
									{
										shoot(BONDAGE, info.self.view_angle, new_enemy.player_id);
										last_shoot = 1;
										heal_item--;
										flag = true;
										debug("HEAL");
									}
						}
						else
						{
							now_policy = "HELP";
							new_move_angle = Get_New_Move_Direction(new_enemy.x, new_enemy.y);
							if (dis < 3) ifchangeview = false;
							Move_Check("HELP");
						}
						ifaction = true;
					}
				}
			}
		}
	for (int i = 0; i < info.sounds.size(); ++i)
	{
		if (info.sounds[i].type == FOOTSTEP_SOUND)
		{
			if (info.sounds[i].delay < delay_time_foot)
			{
				delay_time_foot = info.sounds[i].delay;
				angle_foot = info.sounds[i].angle;
			}
		}
		if (info.sounds[i].type == GUN_SOUND)
		{
			if (info.sounds[i].delay < delay_time_gun)
			{
				delay_time_gun = info.sounds[i].delay;
				angle_gun = info.sounds[i].angle;
			}
		}
	}
	if (delay_time_foot != INF) new_view_angle = Trans(angle_foot);
	if (delay_time_gun != INF) new_view_angle = Trans(angle_gun);
	if (sound_time == 0 && (delay_time_gun != INF || delay_time_foot != INF)) ifchecksound = true;
}

inline void View_Check()
{
	std::vector<ei> enemy_INFO = enemy_info;
	std::sort(enemy_INFO.begin(), enemy_INFO.end(), [](ei A, ei B)
	{
		return A.info_frame > B.info_frame;
	});
	if (info.others.empty())
	{
		if (Heal_Check()) return;
		if (!info.items.empty())
		{
			if (target_item.polar_pos.distance < 5 && (info.poison.current_radius - now_poison_dis > 150)) Move_Check("GET ITEM");
			else if (target_item.polar_pos.distance < 10 && frame < 800) Move_Check("GET ITEM");
			else if (target_item.polar_pos.distance < 50 && now_policy == "NoSpecialPolicy") Move_Check("GET ITEM");
			else Move_Check("RUN");
		}
		else Move_Check("RUN");
	}
	else
	{
		target_enemy.player_ID = -1;
		target_enemy.polar_pos.distance = INF;
		for (int i = 0; i < info.others.size(); ++i)
		{
			if (!Check_Enemy(i)) continue;
			XYPosition P = Get_Absolute_Position(info.others[i].polar_pos.angle, info.others[i].polar_pos.distance);
			ei new_enemy = {info.others[i].player_ID, frame, P.x, P.y };
			if (info.others[i].status != DEAD || info.others[i].status != REAL_DEAD)
			{
				enemy_info[info.others[i].player_ID] = new_enemy;
				if (info.others[i].polar_pos.distance < target_enemy.polar_pos.distance)
					target_enemy = info.others[i];
			}
			ifnewinfo = true;
		}
		if (target_enemy.player_ID != -1)
		{
			if (Attack_Check()) return;
			if (!info.items.empty())
			{
				if (target_item.polar_pos.distance < 10) Move_Check("GET ITEM");
				else Move_Check("FIGHT MOVE");
			}
			else Move_Check("FIGHT MOVE");
		}
		else
		{
			if (Heal_Check()) return;
			if (!info.items.empty())
			{
				if (target_item.polar_pos.distance < 5 && (info.poison.current_radius - now_poison_dis > 150)) Move_Check("GET ITEM");
				else if (target_item.polar_pos.distance < 10 && frame < 800) Move_Check("GET ITEM");
				else if (target_item.polar_pos.distance < 50 && now_policy == "NoSpecialPolicy") Move_Check("GET ITEM");
				else Move_Check("RUN");
			}
			else Move_Check("RUN");
		}
	}
}

inline bool Dead_Check()
{
	if (info.self.hp > 0) return false;
	int msg = 0;
	msg += int(info.self.xy_pos.x) * 100000;
	msg += int(info.self.xy_pos.y) * 100;
	msg += info.player_ID;
	radio(medic_man_ID, msg);
	return true;
}

inline void State_Update()
{
	if (sound_time > 0) sound_time--;
	now_policy = "NoSpecialPolicy";
	new_move_angle = Trans(info.self.move_angle - info.self.view_angle);
	new_view_angle = 90;
	if (Get_Distance(info.self.xy_pos.x, info.self.xy_pos.y, tem_pos_x, tem_pos_y) <= 1)
	{
		tem_pos_x = -1;
		tem_pos_y = -1;
	}
	if (last_pos_x == info.self.xy_pos.x && last_pos_y == info.self.xy_pos.y) block_time++;
	else block_time = 0;
	last_policy = now_policy;
	ifchecksound = false;
	ifaction = false;
}

void play_game()
{
	update_info();
	debug("Start");
	if (frame == 0)
	{
		ei new_enemy = {-1, -100, 0, 0 };
		for (int i = 0; i < 100; ++i)
		{
			new_enemy.player_id = i;
			enemy_info.push_back(new_enemy);
		}
		#ifdef VOCATION_SNIPER
			Name = "SniperX";
			XYPosition landing_point = { 601, 899 };
			parachute(SNIPER, landing_point);
		#endif
		#ifdef VOCATION_MEDIC
			Name = "MedicX";
			XYPosition landing_point = { 666, 899 };
			parachute(MEDIC, landing_point);
		#endif
		#ifdef VOCATION_SIGNALMAN
			Name = "SignalmanX";
			XYPosition landing_point = { 725, 899 };
			parachute(SIGNALMAN, landing_point);
		#endif
		#ifdef VOCATION_HACK
			Name = "HackX";
			XYPosition landing_point = { 799, 899 };
			parachute(HACK, landing_point);
		#endif
		return;
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		if (!ifsendID && info.self.vocation == SIGNALMAN)
		{
			ifsendID = true;
			for (int i = 0; i < teammates.size(); ++i)
			{
				if (teammates[i] == info.player_ID) continue;
				radio(teammates[i], 0);
			}
		}
		if (!ifsendID && info.self.vocation == MEDIC && signal_man_ID != -1)
		{
			ifsendID = true;
			radio(signal_man_ID, 0);
		}
		Sound_Check();
		debug("END");
		return;
	}
	if (Dead_Check())
	{
		debug("END");
		return;
	}
	State_Update();
	Item_Check();
	Weapon_Update();
	Poison_Check();
	Sound_Check();
	if (!ifaction)
	{
		Enemy_Check();
		Poison_Warning();
		View_Check();
	}
	last_pos_x = info.self.xy_pos.x;
	last_pos_y = info.self.xy_pos.y;
	debug("END");
	return;
}