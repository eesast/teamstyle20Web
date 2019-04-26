#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <cmath>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

const int INF = 100000;
const double PI = 3.14159265358979;
const double Dir[4] = { 0., 90., 180., 270. };
const ITEM WeaponPriority_Fatal[10] = { SNIPER_BARRETT, SNIPER_RILFE, TIGER_BILLOW_HAMMER, MACHINE_GUN, ASSAULT_RIFLE, CROSSBOW, SEMI_AUTOMATIC_RILE, FIST, SUBMACHINE_GUN, HAND_GUN };
const ITEM WeaponPriority_Normal[10] = { SNIPER_BARRETT, SNIPER_RILFE, MACHINE_GUN, SUBMACHINE_GUN, ASSAULT_RIFLE, TIGER_BILLOW_HAMMER, SEMI_AUTOMATIC_RILE, FIST, HAND_GUN, CROSSBOW };
const std::string Name = "Hack";
Item target_item;
ITEM weapon;
OtherInfo target_enemy;
int run_time = 0;
int last_shoot = -1;
int max_range = 1;
int Attack_Count = 0;
int now_policy = 0;
int ifblock = 0;
double now_target_dir;
double new_target_dir;
double now_poison_dis;
double new_view_angle;
double last_pos_x = -1;
double last_pos_y = -1;

void debug(std::string cmd)
{
	std::fstream f(Name + ".txt", std::ios::app);
	if (cmd == "Start")
	{
		f << "=====   " << frame << "   =====" << std::endl;
		f << "HP  " << info.self.hp << "   POS  " << info.self.xy_pos.x
			<< "  " << info.self.xy_pos.y << "   STATUS  " << info.self.status << std::endl;
		f << "Run time  " << run_time << "  Attack CD  " << info.self.attack_cd << std::endl;
		f << "View Angle  " << info.self.view_angle << "  Move Angle  " << info.self.move_angle << std::endl;
		f << "View Width  " << info.self.view_width << "  View Distance  " << info.self.view_distance << std::endl;
		f << "Poison Center Radius  " << info.poison.current_radius << "  Poison Distance  " << now_poison_dis << std::endl;
		f << "Poison Center " << info.poison.current_center.x << "  " << info.poison.current_center.y << std::endl;
		f << "Ifblock  " << ifblock << std::endl;
		for (int i = 0; i < info.self.bag.size(); ++i)
			f << "Item  " << i << "  " << info.self.bag[i].type
			<< "  durability  " << info.self.bag[i].durability << std::endl;
	}
	if (cmd == "END")
		f << "=====   END   =====" << std::endl << std::endl;
	if (cmd == "MOVE")
		f << "** MOVE **" << std::endl;
	if (cmd == "ATTACK")
		f << "** ATTACK " << target_enemy.player_ID << " WITH " << weapon << "  **" << std::endl;
	if (cmd == "PICK")
		f << "** PICK  " << target_item.item_ID << "  **" << std::endl;
	if (cmd == "HEAL")
		f << "** HEAL **" << std::endl;
	if (cmd == "DIRECT")
		f << "** DIRECT **  " << now_target_dir << "  " << new_target_dir << std::endl;
}

inline void Calibrate_Angle(double & angle)
{
	while (angle >= 360) angle -= 360;
	while (angle < 0) angle += 360;
}

inline double Get_Absolute_Direction(double target_angle)
{
	double real_angle = target_angle + info.self.view_angle;
	Calibrate_Angle(real_angle);
	return real_angle;
}

inline double Get_Relate_Direction(double target_angle)
{
	double real_angle = target_angle - info.self.view_angle;
	Calibrate_Angle(real_angle);
	return real_angle;
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

inline bool Check_Weapon(ITEM & this_weapon)
{
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		if (info.self.bag[i].type == this_weapon && info.self.bag[i].durability > 0)
		{
			max_range = std::max(max_range, ITEM_DATA[this_weapon].range);
			return true;
		}
	}
	return false;
}

inline bool Pick_Check()
{
	if (target_item.polar_pos.distance < 1)
	{
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
		if (info.self.bag[i].type == FIRST_AID_CASE && info.self.bag[i].durability > 0 && info.self.hp < 30)
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
	if (info.self.status == MOVING && !ifblock)
	{
		if (run_time > 0) run_time--;
		return;
	}
	double new_view_angle = info.self.view_angle;
	double random_angle = 0;
	srand((time(nullptr) * (info.player_ID + frame)) % 1007);
	if (ifblock) random_angle = 30 * ifblock + rand() % 10;
	if (cmd == "RUN")
	{
		if (run_time > 0) run_time--;
		now_target_dir = now_target_dir + random_angle;
		new_target_dir = new_target_dir + random_angle;
		Calibrate_Angle(new_target_dir);
		Calibrate_Angle(now_target_dir);
		if (now_policy == 1) new_view_angle = now_target_dir;
		if (now_policy == 2) new_view_angle = new_target_dir;
		move(new_view_angle, new_view_angle);
	}
	if (cmd == "SLOW MOVE")
	{
		new_view_angle += 90;
		Calibrate_Angle(new_view_angle);
		now_target_dir = now_target_dir + random_angle;
		Calibrate_Angle(now_target_dir);
		move(new_target_dir, new_view_angle);
	}
	if (cmd == "GET ITEM")
	{
		new_target_dir = target_item.polar_pos.angle + random_angle;
		Calibrate_Angle(new_target_dir);
		move(new_target_dir, new_target_dir);
	}
	if (cmd == "FIGHT MOVE")
	{
		if (max_range >= target_enemy.polar_pos.distance && !(info.self.attack_cd > 3 && info.self.hp < 50 && Attack_Count == 0))
		{
			double target_move = target_enemy.polar_pos.angle + 90 + random_angle;
			Calibrate_Angle(target_move);
			move(target_move, target_enemy.polar_pos.angle);
		}
		else
		{
			run_time = 9;
			double target_move = target_enemy.polar_pos.angle - 90 + random_angle;
			Calibrate_Angle(target_move);
			move(target_move, target_move);
		}
	}
	debug("MOVE");
}

inline int Attack_Check()
{
	bool ifattack = false;
	for (int i = 0; i < 10; ++i)
	{
		weapon = WeaponPriority_Normal[i];
		if (!Check_Weapon(weapon)) continue;
		if (ITEM_DATA[weapon].range < target_enemy.polar_pos.distance) continue;
		ifattack = true;
		break;
	}
	if (ifattack && info.self.attack_cd == 0)
	{
		double target_angle = target_enemy.polar_pos.angle;
		target_angle += Shift_Angle_Calc();
		Calibrate_Angle(target_angle);
		shoot(weapon, target_angle);
		last_shoot = 0;
		Attack_Count++;
		debug("ATTACK");
		return 0;
	}
	return 1;
}

inline void Block_Check()
{
	if (info.self.status == MOVING && last_pos_x == info.self.xy_pos.x && last_pos_y == info.self.xy_pos.y)
		ifblock++;
	else ifblock = 0;
	last_pos_x = info.self.xy_pos.x;
	last_pos_y = info.self.xy_pos.y;
}

inline void Item_Check()
{
	target_item.polar_pos.distance = INF;
	for (int i = 0; i < info.items.size(); ++i)
		if (info.items[i].polar_pos.distance < target_item.polar_pos.distance)
			target_item = info.items[i];
}

inline void Poison_Check()
{
	int cx, cy, nx, ny;
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
	now_target_dir = atan((cy - info.self.xy_pos.y) / (cx - info.self.xy_pos.x)) / PI * 180;
	if (info.self.xy_pos.x > cx) now_target_dir += 180;
	now_target_dir = Get_Relate_Direction(now_target_dir);
	Calibrate_Angle(now_target_dir);
	new_target_dir = atan((ny - info.self.xy_pos.y) / (nx - info.self.xy_pos.x)) / PI * 180;
	if (info.self.xy_pos.x > nx) new_target_dir += 180;
	new_target_dir = Get_Relate_Direction(new_target_dir);
	Calibrate_Angle(new_target_dir);
	now_poison_dis = (cy - info.self.xy_pos.y) * (cy - info.self.xy_pos.y)
		+ (cx - info.self.xy_pos.x) * (cx - info.self.xy_pos.x);
	now_poison_dis = sqrt(now_poison_dis);
	debug("DIRECT");
}

inline void Poison_Warning()
{
	if (frame >= 300 && (info.poison.current_radius - now_poison_dis < 400))
	{
		if (info.poison.move_flag == 1) now_policy = 2;
		else now_policy = 1;
		if (run_time == 0) run_time = 9;
		return;
	}
	now_policy = 0;
	return;
}

inline bool Check_Enemy(int i)
{
	for (int j = 0; j < teammates.size(); ++j)
		if (info.others[i].player_ID == teammates[j])
			return false;
	return true;
}

void play_game()
{
	update_info();
	debug("Start");
	if (frame == 0)
	{
		srand((time(nullptr) * (info.player_ID + frame)) % 1007);
		XYPosition landing_point = { 750 + rand() % 10, 450 + rand() % 10 };
		parachute(HACK, landing_point);
		return;
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING || Heal_Check())
	{
		debug("END");
		return;
	}
	Item_Check();
	Poison_Check();
	Block_Check();
	Poison_Warning();
	if (info.others.empty())
	{
		if (run_time > 0) Move_Check("RUN");
		else if (!info.items.empty()) Move_Check("GET ITEM");
		else Move_Check("SLOW MOVE");
	}
	else
	{
		target_enemy.player_ID = -1;
		target_enemy.polar_pos.distance = INF;
		for (int i = 0; i < info.others.size(); ++i)
		{
			if (!Check_Enemy(i)) continue;
			if (info.others[i].polar_pos.distance < target_enemy.polar_pos.distance)
				target_enemy = info.others[i];
		}
		if (target_enemy.player_ID != -1)
		{
			int Policy = Attack_Check();
			if (Policy && now_policy == 0) Move_Check("FIGHT MOVE");
			if (Policy && now_policy != 0) Move_Check("RUN");
		}
		else
		{
			if (run_time > 0) Move_Check("RUN");
			else if (!info.items.empty()) Move_Check("GET ITEM");
			else Move_Check("SLOW MOVE");
		}
	}
	debug("END");
	return;
}