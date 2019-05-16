#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <map>
#define LANDINGPOINT 0

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

const int _f = 10;

struct EnemyInfo
{
	OtherInfo info;
	XYPosition last_seen_pos;
	int seen_frame; // 最后见到的帧，目前下面逻辑：10帧不见删，下同，存于_f中
	int flag; // 0为视觉，1为脚步声，2为枪声
};

struct ItemInfo
{
	Item info;
	XYPosition pos;
	int seen_frame;
};

const double PI = acos(-1);

inline XYPosition polar_to_XY(const PolarPosition polar)
{
	// cast PolarPosition seen by the player to XYPosition on map
	double angle = (info.self.view_angle + polar.angle) * PI / 180;
	return XYPosition{ info.self.xy_pos.x + polar.distance * cos(angle), info.self.xy_pos.y + polar.distance * sin(angle) };
}

inline PolarPosition XY_to_polar(const XYPosition dst)
{
	// cast XYPosition to PolarPosition seen by self
	double d_x = dst.x - info.self.xy_pos.x, d_y = dst.y - info.self.xy_pos.y;
	double angle = atan2(d_y, d_x) * 180 / PI - info.self.view_angle; // -540 ~ 180
	while (angle < 0)
		angle += 360;
	return PolarPosition{ hypot(d_x, d_y), angle };
}

extern const std::vector<block> _CITY;

void walk(std::ofstream &fout, PolarPosition polar, double view_angle = NAN)
{
	static XYPosition last_pos = info.self.xy_pos;
	const XYPosition cur_pos = info.self.xy_pos;
	polar.angle += info.self.view_angle;
	if (polar.angle < 0)
		polar.angle += 360;
	else if (polar.angle > 360)
		polar.angle -= 360;
	XYPosition dst = { cur_pos.x + VOCATION_DATA[info.self.vocation].move * cos(polar.angle * PI / 180), cur_pos.y + VOCATION_DATA[info.self.vocation].move * sin(polar.angle * PI / 180) };
	bool not_moved = fabs(cur_pos.x - last_pos.x) < 0.05 && fabs(cur_pos.y - last_pos.y) < 0.05, movable = true;
	static int cnt = 0;
	if (std::isnan(view_angle))
		view_angle = polar.angle;
	if (not_moved)
	{
		fout << "DEBUG _ Point 1" << std::endl;
		if (!cnt)
			cnt = rand() % 2 ? -1 : 1;
		else
		{
			if (cnt > 0)
				++cnt;
			else
				--cnt;
		}
		double _a = polar.angle + cnt * 50;
		while (_a < 0)
			_a += 360;
		while (_a > 360)
			_a -= 360;
		move(_a, view_angle);
		last_pos = cur_pos;
		return;
	}
	else
		cnt = 0;
	if (info.self.move_cd > 0)
	{
		last_pos = cur_pos;
		return;
	}
	double _r = VOCATION_DATA[info.self.vocation].radius;
	AREA dst_landform = MAP[int(dst.x) / 100 + int(dst.y) / 100 * 10];
	if ((int(dst.x) / 100 != int(cur_pos.x) / 100 || int(dst.y) / 100 != int(cur_pos.y) / 100) && (dst_landform == SEA || dst_landform == POOL))
	{
		fout << "DEBUG _ Point 2" << std::endl;
		// move into new landform
		// landform that we don't go into, currently only SEA
		movable = false;
		if (int(dst.x) / 100 != int(cur_pos.x) / 100)
		{
			fout << "DEBUG _ Point 3" << std::endl;
			static int flag = 0;
			if (flag == 0)
				flag = sin(polar.angle * PI / 180) > 0 ? 1 : -1;
			double move_angle = flag == 1 > 0 ? 90 : 270;
			move_angle -= info.self.view_angle;
			if (move_angle < 0)
				move_angle += 360;
			else if (move_angle > 360)
				move_angle -= 360;
			move(move_angle, view_angle);
			if (int(dst.y) / 100 != int(cur_pos.y) / 100)
				flag = 0;
			// dst = { cur_pos.x + cos(polar.angle) > 0 ? 1 : -1, cur_pos.y };
		}
		else
		{
			fout << "DEBUG _ Point 4" << std::endl;
			static int flag = 0;
			if (flag == 0)
				flag = cos(polar.angle * PI / 180) > 0 ? 1 : -1;
			double move_angle = flag == 1 ? 0 : 180;
			move_angle -= info.self.view_angle;
			if (move_angle < 0)
				move_angle += 360;
			else if (move_angle > 360)
				move_angle -= 360;
			move(move_angle, view_angle);
			if (int(dst.x) / 100 != int(cur_pos.x) / 100)
				flag = 0;
			// dst = { cur_pos.x, cur_pos.y + sin(polar.angle) > 0 ? 1 : -1 };
		}
	}
	else
	{
		fout << "DEBUG _ Point 5" << std::endl;
		double _x = dst.x - 100 * int(dst.x / 100), _y = dst.y - 100 * int(dst.y / 100);
		double cur_pos_x = cur_pos.x - 100 * int(cur_pos.x / 100), cur_pos_y = cur_pos.y - 100 * int(cur_pos.y / 100);
		const std::vector<block> &_block = dst_landform == CITY ? _CITY : AREA_DATA[dst_landform];
		for (auto &a : _block)
		{
			if (a.type == SHALLOW_WATER || a.type == CIRCLE_GRASS || a.type == RECTANGLE_GRASS)
				continue;
			if (a.shape == CIRCLE && hypot(_x - a.x0, _y - a.y0) < a.r + _r + 0.1)
			{
				fout << "DEBUG _ Point 6" << std::endl;
				movable = false;
				PolarPosition obs_polar = XY_to_polar(XYPosition{ double(a.x0), double(a.y0) });
				double _a = obs_polar.angle - polar.angle + info.self.view_angle, _fa = asin((a.r + _r) / obs_polar.distance) * 180 / PI + 5;
				double move_angle = obs_polar.angle;
				if (_a > 0 && _a < 180 || _a > -180)
					move_angle += _fa;
				else
					move_angle -= _fa;
				if (move_angle < 0)
					move_angle += 360;
				else if (move_angle > 360)
					move_angle -= 360;
				move(move_angle, view_angle);
				break;
			}
			fout << _x << '#' << _y << '#' << _r << std::endl;
			if (a.shape == RECTANGLE && _x + _r + 0.1 > a.x0 && _x - _r - 0.1 < a.x1 && _y - _r - 0.1 < a.y0 && _y + _r + 0.1 > a.y1)
			{
				fout << "DEBUG _ Point 7" << std::endl;
				if (cur_pos_x > a.x0 - _r - 0.1 && cur_pos_x < a.x1 + _r + 0.1)
				{
					fout << "DEBUG _ Point 8" << std::endl;
					movable = false;
					static int flag = 0;
					if (flag == 0)
						flag = cos(polar.angle * PI / 180) > 0 ? 1 : -1;
					double move_angle = flag == 1 ? 0 : 180;
					move_angle -= info.self.view_angle;
					if (move_angle < 0)
						move_angle += 360;
					else if (move_angle > 360)
						move_angle -= 360;
					move(move_angle, view_angle);
					if (cur_pos_x + flag >= a.x1 + _r + 0.1 || cur_pos_x + flag <= a.x0 - _r - 0.1)
						flag = 0;
					break;
				}
				else
				{
					fout << "DEBUG _ Point 9" << std::endl;
					movable = false;
					static int flag = 0;
					if (flag == 0)
						flag = sin(polar.angle * PI / 180) > 0 ? 1 : -1;
					double move_angle = flag == 1 ? 90 : 270;
					move_angle -= info.self.view_angle;
					if (move_angle < 0)
						move_angle += 360;
					else if (move_angle > 360)
						move_angle -= 360;
					move(move_angle, view_angle);
					if (cur_pos_y + flag >= a.y0 + _r + 0.1 || cur_pos_y + flag <= a.y1 - _r - 0.1)
						flag = 0;
					break;
				}
			}
		}
	}
	if (movable)
	{
		fout << "DEBUG _ Point 10" << std::endl;
		polar.angle -= info.self.view_angle;
		if (polar.angle < 0)
			polar.angle += 360;
		else if (polar.angle > 360)
			polar.angle -= 360;
		move(polar.angle, view_angle);
	}
	last_pos = cur_pos;
}

void walk_shake_head(std::ofstream &fout, PolarPosition polar)
{
	double view_angle = (frame % 3 - 1) * 80 + polar.angle;
	if (view_angle > 360)
		view_angle -= 360;
	if (view_angle < 0)
		view_angle += 360;
	walk(fout, polar, view_angle);
}

const std::vector<block> _CITY =
{
	//{ RECTANGLE,                WALL,    5,    6,    0,   35,    5},//
	//{ RECTANGLE,                WALL,    5,   35,    0,    6,    6},//
	//{ RECTANGLE,                WALL,   65,    6,    0,   94,    5},//
	//{ RECTANGLE,                WALL,   94,   35,    0,   95,    5},//
	//{ RECTANGLE,                WALL,    5,   95,    0,    6,   65},//
	//{ RECTANGLE,                WALL,    6,   95,    0,   35,   94},//
	//{ RECTANGLE,                WALL,   94,   94,    0,   95,   65},//
	//{ RECTANGLE,                WALL,   65,   95,    0,   95,   94},//
	{ RECTANGLE,  RECTANGLE_BUILDING,   5,    36,    0,   36,   5},
	{ RECTANGLE,  RECTANGLE_BUILDING,   40,   30,    0,   60,   10},
	{ RECTANGLE,  RECTANGLE_BUILDING,   64,   36,    0,   95,   5},
	{ RECTANGLE,  RECTANGLE_BUILDING,   5,    95,    0,   36,   64},
	{ RECTANGLE,  RECTANGLE_BUILDING,   40,   90,    0,   60,   70},
	{ RECTANGLE,  RECTANGLE_BUILDING,   64,   95,    0,   95,   64},
	//{    CIRCLE,                TREE,   10,   35,    1,    0,    0},//
	//{    CIRCLE,                TREE,   35,   35,    1,    0,    0},//
	//{    CIRCLE,                TREE,   65,   35,    1,    0,    0},//
	//{    CIRCLE,                TREE,   90,   35,    1,    0,    0},//
	//{    CIRCLE,                TREE,   10,   65,    1,    0,    0},//
	//{    CIRCLE,                TREE,   35,   65,    1,    0,    0},//
	//{    CIRCLE,                TREE,   65,   65,    1,    0,    0},//
	//{    CIRCLE,                TREE,   90,   65,    1,    0,    0},//
};

bool com(XYPosition a, XYPosition b)
{
	if (a.x == b.x && a.y == b.y)
		return true;
	else
		return false;
}
double distance(XYPosition a, XYPosition b)
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
void wander(std::ofstream &fout, XYPosition a, double view_angle)
{
	double randx = a.x + double(rand() % 20) - 10;
	double randy = a.y + double(rand() % 20) - 10;
	XYPosition rand = XYPosition{ randx, randy };
	walk(fout, XY_to_polar(rand), view_angle);
}

void heal(bool &need_heal, bool &healing);
void _attack(std::ofstream &fout, const std::map<int, EnemyInfo> &_enemy, int &attacking, bool healing);

const char* const STATUS_N[] =
{
	"RELAX",
	"ON_PLANE",
	"JUMPING",
	"MOVING",
	"SHOOTING",
	"PICKUP",
	"MOVING_SHOOTING",
	"DEAD",
	"REAL_DEAD"
};

const char* const VOCATION_N[] =
{
	"MEDIC",
	"SIGNALMAN",
	"HACK",
	"SNIPER",
	"VOCATION_SZ"
};

const char* const ITEM_TYPE_N[]
{
	"WEAPON",
	"ARMOR",
	"GOODS",
	"ITEM_TYPE_SZ"
};

const char* const ITEM_MODE_N[] =
{
	"PORTABLE",
	"SPENDABLE",
	"TRIGGERED",
	"ITEM_MODE_SZ"
};

const char* const ITEM_N[] =
{
	"FIST",
	"HAND_GUN",
	"SUBMACHINE_GUN",
	"SEMI_AUTOMATIC_RILE",
	"ASSAULT_RIFLE",
	"MACHINE_GUN",
	"SNIPER_RILFE",
	"SNIPER_BARRETT",
	"TIGER_BILLOW_HAMMER",
	"CROSSBOW",
	"VEST_1",
	"VEST_2",
	"VEST_3",
	"INSULATED_CLOTHING",
	"MUFFLER",
	"BONDAGE",
	"FIRST_AID_CASE",
	"CODE_CASE",
	"SCOPE_2",
	"SCOPE_4",
	"SCOPE_8",
	"ITEM_SZ"
};

const char* const SOUND_N[] =
{
	"RADIO_VOICE",
	"FOOTSTEP_SOUND",
	"GUN_SOUND",
	"BOOM_SOUND",
	"SOUND_SZ"
};

const char* const BLOCK_SHAPE_N[] =
{
	"CIRCLE",
	"RECTANGLE",
	"BLOCK_SHAPE_SZ"
};

const char* const BLOCK_TYPE_N[] =
{
	"DEEP_WATER",
	"RECTANGLE_BUILDING",
	"SHALLOW_WATER",
	"CIRCLE_BUILDING",
	"WALL",
	"TREE",
	"RECTANGLE_GRASS",
	"CIRCLE_GRASS",
	"BLOCK_TYPE_SZ"
};

const char* const AREA_N[] =
{
	"SEA",
	"HILL",
	"POOL",
	"BEACH",
	"CITY",
	"FARMLAND",
	"ROADA",
	"ROADB",
	"GRASS",
	"FOREST",
	"AREA_SZ"
};

void play_game()
{
	static int attacking = -1; // 当前正在攻击的player_id，不在攻击为-1
	static bool healing = true; // 正在回血，需要回血后面need_heal
	static std::map<int, EnemyInfo> _enemy;
	static std::map<int, ItemInfo> _item;
	static bool pick = false;//pick和attack用于跑毒时候的判断
	double _view_angle = NAN;
	update_info();

	std::ofstream fout;
	const std::string out_name = std::to_string(info.player_ID) + std::string(".txt");
	fout.open(out_name, std::ios::app);
	fout << "\nplayer:frame" << frame << "\nhp:" << info.self.hp << ' ' << VOCATION_N[info.self.vocation] << std::endl;
	fout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	fout << "bag info: [";
	for (auto& a : info.self.bag)
		fout << ITEM_N[a.type] << '#' << a.durability <<',';
	fout << ']' << std::endl;

	if (frame == 0)
	{
		srand(time(nullptr) + 134673 + teammates[0]);
		XYPosition landing_point;
		switch (LANDINGPOINT)
		{
		case 0:
			landing_point = { 200.0 + rand() % 100, 400.0 + rand() % 200 };
			break;
		case 1:
			landing_point = { 500.0 + rand() % 100, 400.0 + rand() % 100 };
			break;
		case 2:
			landing_point = { 200.0 + rand() % 200, 100.0 + rand() % 100 };
			break;
		case 3:
			landing_point = { 100.0 + rand() % 100, 300.0 + rand() % 100 };
			break;
		}
		//srand(time(nullptr) + 123432 + info.player_ID);
		//landing_point.x += rand() % 40 - 20;
		//landing_point.y += rand() % 40 - 20;
		parachute(HACK, landing_point);
		return;
	}
	else
	{
		srand(time(nullptr) + 173425 + info.player_ID * frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		fout << "jumping" << std::endl;
		return;
	}


	//第二个坑,决定什么物品该捡

	int level[21] = { 0,7,14,11,13,15,10,12,9,8,4,5,6,3,0,1,2,0,0,0,0 };//武器优先级

	//第7个坑，决定先捡谁
	static Item target;//目标物品
	static XYPosition target_pos = { 100000, 100000 };

	int max = target_pos.x == 100000 ? 0 : level[target.type];
	for (auto& entry : info.items)
	{
		if (target_pos.x != 100000 && (entry.polar_pos.distance > 2 * XY_to_polar(target_pos).distance || (level[entry.type] < max + 5 && entry.polar_pos.distance > 1.5 * XY_to_polar(target_pos).distance)))
			continue;
		//
		XYPosition _pos = polar_to_XY(entry.polar_pos);
		if (MAP[int(_pos.x) / 100 + int(_pos.y) / 100 * 10] == CITY)
		{
			_pos.x -= int(_pos.x) / 100 * 100;
			_pos.y -= int(_pos.y) / 100 * 100;
			bool flag = false;
			for (auto &a : _CITY)
			{
				if (a.shape == CIRCLE)
					continue;
				if (_pos.x >= a.x0 && _pos.x <= a.x1 && _pos.x <= a.y0 && _pos.x >= a.y1)
				{
					flag = true;
					break;
				}
			}
			if (flag)
				continue;
		}
		if (level[entry.type] > max)
		{
			max = level[entry.type];
			target = entry;
			target_pos = polar_to_XY(entry.polar_pos);
		}
	}
	if (target_pos.x != 100000)
	{
		pick = true;
		target.polar_pos = XY_to_polar(target_pos);
	}

	//判断是否要回血
	bool need_heal = false;
	if (info.self.hp_limit - info.self.hp > 30)
		need_heal = true;

	//敌人处理
	for (auto& enemy : info.others)
	{
		const int id = enemy.player_ID;
		bool is_friend = false;
		for (int t : teammates)
		{
			if (t == id)
			{
				is_friend = true;
				break;
			}
		}
		if (is_friend)
			continue;
		if (_enemy.count(id)) {
			_enemy[id].seen_frame = frame;
			_enemy[id].info = enemy;
			_enemy[id].last_seen_pos = polar_to_XY(enemy.polar_pos);
		}
		else
			_enemy[id] = EnemyInfo{ enemy, polar_to_XY(enemy.polar_pos), frame, 0 };
		if (true)
		{
			// 逻辑上判断一通这个敌人要不要打以及后续处理，坑no.3
			if (_enemy.find(id)->second.info.status > 6)
				_enemy.erase(id);
			if (_enemy.find(id)->second.info.polar_pos.distance > 150)
				_enemy.erase(id);
		}
	}

	int closest_enemy_id = -1;
	if (!_enemy.empty())
	{
		for (auto iter = _enemy.begin(); iter != _enemy.end();)
		{
			if (frame - iter->second.seen_frame >= _f)
				_enemy.erase(iter++);
			else
			{
				if (iter->first == -1) {
					++iter;
					continue;
				}
				if (closest_enemy_id == -1) {
					closest_enemy_id = iter->first;
				}
				else {
					if (iter->second.info.polar_pos.distance < _enemy.find(closest_enemy_id)->second.info.polar_pos.distance && iter->second.info.status < 7)
						closest_enemy_id = iter->first;
				}
				if (attacking == -1 && iter->second.info.status < 7) {
					attacking = iter->first;
				}
				else
				{
					//如果较远，打近的
					if (_enemy.find(attacking)->second.info.polar_pos.distance > _enemy[closest_enemy_id].info.polar_pos.distance + 15)
						attacking = closest_enemy_id;
					//距离接近的，打MEDIC
					if (_enemy.find(closest_enemy_id)->second.info.vocation == MEDIC && _enemy.find(closest_enemy_id)->second.info.status < 7)
						attacking = closest_enemy_id;
					else if (_enemy.find(attacking)->second.info.status > 6) {
						attacking = closest_enemy_id;
					}
					else {
						attacking;
					}
				}
				fout << "enemy id:" << iter->first << " " << iter->second.info.polar_pos.distance << " " << iter->second.info.polar_pos.angle << " status" << iter->second.info.status << std::endl;
				++iter;
			}
		}
	}
	if (_enemy.empty())
	{
		//_enemy为空
		attacking = -1;
		closest_enemy_id = -1;
	}
	fout << "attacking:" << attacking << " closest:" << closest_enemy_id << " cd:" << info.self.attack_cd << " move_cd:" << info.self.move_cd << std::endl;

	//跑毒

	double r[10] = { 750, 450, 225, 112.5, 56.25, 28.13, 14.06, 7.03, 0, 0 };//各阶段毒圈的末半径
	double t0[9] = { 600, 380, 360, 340, 320, 300, 280, 260, 240 };
	int status;//阶段，用于判断末半径
	if (frame < 600)
		status = 0;
	else if (frame < 980)
		status = 1;
	else if (frame < 1340)
		status = 2;
	else if (frame < 1680)
		status = 3;
	else if (frame < 2000)
		status = 4;
	else if (frame < 2300)
		status = 5;
	else if (frame < 2580)
		status = 6;
	else if (frame < 2840)
		status = 7;
	else if (frame < 3080)
		status = 8;


	//回血，可能需要更改位置
	if (attacking == -1 || closest_enemy_id == -1 || (need_heal && _enemy.find(closest_enemy_id)->second.info.polar_pos.distance > 5))
		heal(need_heal, healing);

	if (frame > 400)
	{
		if (distance(info.self.xy_pos, info.poison.current_center) > info.poison.current_radius)//在圈外
		{

			walk(fout, XY_to_polar(info.poison.current_center));
		}
		else//在圈内
		{
			if (info.poison.move_flag == 1)//在缩圈
			{
				if (distance(info.self.xy_pos, info.poison.current_center) <= r[status])//在圈的末半径内
				{
					if (pick)
					{
						if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120)//且在射程内
						{
							fout << "DEBUG point 1" << std::endl;
							//射击
							_view_angle = _enemy[attacking].info.polar_pos.angle;
							_attack(fout, _enemy, attacking, healing);
						}
						if (distance(target_pos, info.poison.current_center) <= r[status])//东西在圈的末半径内
						{
							//捡东西
							fout << "**target item angle:" << target.polar_pos.angle << " distance" << target.polar_pos.distance << " ID: " << target.item_ID << "**" << std::endl;
							if (target.polar_pos.distance < 1)
							{
								fout << target.item_ID << std::endl;
								pickup(target.item_ID);
								pick = false;
								target_pos = { 100000, 100000 };
								_view_angle = NAN;
								fout << "try pickup" << target.item_ID << ' ' << ITEM_N[target.type] << std::endl;
							}
							else
							{
								_view_angle = target.polar_pos.angle;
								if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120 && target.polar_pos.distance > 1.5) {
									_view_angle = _enemy[attacking].info.polar_pos.angle;
									_attack(fout, _enemy, attacking, healing);
								}
								walk(fout, target.polar_pos, _view_angle);
							}
						}
						else//东西在圈的末半径外
						{
							wander(fout, info.poison.next_center, _view_angle);
						}
					}
					else if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120)
					{
						if (true)//在射程内
						{
							fout << "DEBUG point 2" << std::endl;
							//射击
							_view_angle = _enemy[attacking].info.polar_pos.angle;
							_attack(fout, _enemy, attacking, healing);
							wander(fout, info.poison.next_center, _view_angle);
						}
					}
					else
					{
						fout << "DEBUG point 3" << std::endl;
						wander(fout, info.poison.next_center, _view_angle);
					}
				}
				else//在圈的末半径外
				{
					if (pick || (attacking != -1))
					{
						if (distance(target_pos, info.poison.current_center) <= r[status])//东西在圈的末半径内
						{
							//捡东西
							fout << "**target item angle:" << target.polar_pos.angle << " distance" << target.polar_pos.distance << " ID: " << target.item_ID << "**" << std::endl;
							if (target.polar_pos.distance < 1)
							{
								fout << target.item_ID << std::endl;
								pickup(target.item_ID);
								pick = false;
								target_pos = { 100000, 100000 };
								_view_angle = NAN;
								fout << "try pickup" << target.item_ID << ' ' << ITEM_N[target.type] << std::endl;
							}
							else
							{
								_view_angle = target.polar_pos.angle;
								if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120 && target.polar_pos.distance > 1.5) {
									_view_angle = _enemy[attacking].info.polar_pos.angle;
									_attack(fout, _enemy, attacking, healing);
								}
								walk(fout, target.polar_pos, _view_angle);
							}
						}
						else if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120)//在射程内
						{
							fout << "DEBUG point 4" << std::endl;
							//射击
							_view_angle = _enemy[attacking].info.polar_pos.angle;
							_attack(fout, _enemy, attacking, healing);
							wander(fout, info.poison.current_center, _view_angle);
						}
						else
						{
							wander(fout, info.poison.current_center, _view_angle);
						}
					}
					else
					{
						fout << "DEBUG point 5" << std::endl;
						walk(fout, XY_to_polar(info.poison.current_center), _view_angle);
					}
				}
			}
			else//不在缩圈
			{
				if (distance(info.self.xy_pos, info.poison.next_center) <= r[status + 1])//在下一个圈的末半径内
				{
					if (pick && (distance(target_pos, info.poison.next_center) <= r[status + 1]))//东西在下一个圈的末半径内
					{
						//捡
						fout << "**target item angle:" << target.polar_pos.angle << " distance" << target.polar_pos.distance << " ID: " << target.item_ID << "**" << std::endl;
						if (target.polar_pos.distance < 1)
						{
							fout << target.item_ID << std::endl;
							pickup(target.item_ID);
							pick = false;
							target_pos = { 100000, 100000 };
							_view_angle = NAN;
							fout << "try pickup" << target.item_ID << ' ' << ITEM_N[target.type] << std::endl;
						}
						else
						{
							_view_angle = target.polar_pos.angle;
							if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120 && target.polar_pos.distance > 1.5) {
								_view_angle = _enemy[attacking].info.polar_pos.angle;
								_attack(fout, _enemy, attacking, healing);
							}
							walk(fout, target.polar_pos, _view_angle);
						}
					}
					else if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120)//在射程内
					{
						//射击
						_view_angle = _enemy[attacking].info.polar_pos.angle;
						_attack(fout, _enemy, attacking, healing);
						wander(fout, info.poison.next_center, _view_angle);
					}
					else
					{
						wander(fout, info.poison.next_center, _view_angle);
					}
				}
				else//在下一个圈的末半径外
				{
					if (pick || (attacking != -1))
					{
						if ((distance(target_pos, info.poison.next_center) <= r[status + 1]) || (distance(target_pos, info.self.xy_pos) * fabs(target.polar_pos.angle - XY_to_polar(info.poison.next_center).angle) <= 100))//不是很影响跑毒就去捡
						{
							//捡
							fout << "**target item angle:" << target.polar_pos.angle << " distance" << target.polar_pos.distance << " ID: " << target.item_ID << "**" << std::endl;
							if (target.polar_pos.distance < 1)
							{
								fout << target.item_ID << std::endl;
								pickup(target.item_ID);
								pick = false;
								target_pos = { 100000, 100000 };
								_view_angle = NAN;
								fout << "try pickup" << target.item_ID << ' ' << ITEM_N[target.type] << std::endl;
							}
							else
							{
								_view_angle = target.polar_pos.angle;
								if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120 && target.polar_pos.distance > 1.5) {
									_view_angle = _enemy[attacking].info.polar_pos.angle;
									_attack(fout, _enemy, attacking, healing);
								}
								walk(fout, target.polar_pos, _view_angle);
							}
						}

						else if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120)//在射程内
						{
							//射击
							_view_angle = _enemy[attacking].info.polar_pos.angle;
							_attack(fout, _enemy, attacking, healing);
							wander(fout, info.poison.next_center, _view_angle);
						}
						else
						{
							wander(fout, info.poison.next_center, _view_angle);
						}
					}

					else
					{
						wander(fout, info.poison.next_center, _view_angle);
						// walk(fout, XY_to_polar(info.poison.next_center));
					}
				}
			}

		}
	}
	else
	{

		static XYPosition destination = { 500.0 , 500.0 };//随机移动的目的地
		if (frame % 20 == 1)
			destination = { double(rand() % 1000) , double(rand() % 1000) };
		if (pick)
		{
			fout << "**target item angle:" << target.polar_pos.angle << " distance" << target.polar_pos.distance << " ID: " << target.item_ID << "**" << std::endl;
			if (target.polar_pos.distance < 1)
			{
				fout << target.item_ID << std::endl;
				pickup(target.item_ID);
				pick = false;
				target_pos = { 100000, 100000 };
				_view_angle = NAN;
				fout << "try pickup" << target.item_ID << ' ' << ITEM_N[target.type] << std::endl;
			}
			else
			{
				_view_angle = target.polar_pos.angle;
				if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120 && target.polar_pos.distance > 1.5) {
					for(auto &a : info.self.bag)
					{
						if(a.type > FIST && a.type <= CROSSBOW && a.type != TIGER_BILLOW_HAMMER && a.durability > 0)
							_view_angle = _enemy[attacking].info.polar_pos.angle;
					}
					fout << "try to attack 1\n";
					_attack(fout, _enemy, attacking, healing);
				}
				walk(fout, target.polar_pos, _view_angle);
			}
		}
		else if ((attacking != -1) && _enemy[attacking].info.polar_pos.distance < 120) {
			_view_angle = _enemy[attacking].info.polar_pos.angle;
			_attack(fout, _enemy, attacking, healing);
			walk(fout, XY_to_polar(_enemy[attacking].last_seen_pos), _view_angle);
			// wander(fout, info.poison.next_center);
			fout << "try to attack 2\n";
		}
		else
		{
			walk(fout, XY_to_polar(destination), _view_angle);
		}
	}

	return;
}

void heal(bool &need_heal, bool &healing) {
	bool has_BONDAGE = false, has_FIRST_AID = false;

	if (info.self.attack_cd == 0 && need_heal) {
		for (auto &_item : info.self.bag) {
			if (_item.type == BONDAGE && _item.durability > 0)
				has_BONDAGE = true;
			if (_item.type == FIRST_AID_CASE && _item.durability > 0)
				has_FIRST_AID = true;
		}

		healing = true;

		//急救
		if (info.self.hp < 30) {
			if (has_FIRST_AID)
				shoot(FIRST_AID_CASE, 0.0, -1);
			else if (has_BONDAGE)
				shoot(BONDAGE, 0.0, -1);
			else
				healing = false;
		}

		//血不够满时回血，并可以继续捡绷带
		else {
			if (has_BONDAGE)
				shoot(BONDAGE, 0.0, -1);
			else
				healing = false;
		}
	}
	else {
		//不需要治疗或cd不允许
		healing = false;
	}
}

void _attack(std::ofstream &fout, const std::map<int, EnemyInfo> &_enemy, int &attacking, bool healing) {
	//预留parameter以备sniper有特殊的需求

	if (attacking == -1)
		return;

	//通过CD判断，无论回血中还是射击cd中均停止attack
	if (info.self.attack_cd != 0)
		return;

	if (healing)//可能需要修改
		return;

	ITEM ranged_weapon_order[6] = { MACHINE_GUN, SUBMACHINE_GUN, ASSAULT_RIFLE, SEMI_AUTOMATIC_RILE, CROSSBOW, HAND_GUN };
	//无甲时dps: MACHINE_GUN 40 SUBMACHINE_GUN 25 ASSAULT_RIFLE 20 SEMI_AUTOMATIC_RILE 10 HAND_GUN 6.25 CROSSBOW 5
	//总的来看，机枪 > 冲锋 > 突击 > 半自动/虎涛锤 > 弩 > 手枪 > 拳头，狙击cd太久且叠甲一多也可能不致命，可能需极度慎用
	//此外，近距离战远程武器可能极度容易打偏

	//ITEM sniper_order[2] = {SNIPER_BARRETT, SNIPER_RILFE};
	bool use_sniper = false; //当敌人不动时可以考虑用狙击枪崩了

	bool hasWeapon[10] = { false }; //存放是否拥有武器，可能需要更改逻辑
	// check weapon
	for (auto &_item : info.self.bag) {
		if (_item.type < 10) {
			if (_item.durability > 0)
				hasWeapon[_item.type] = true;
			else
				hasWeapon[_item.type] = false;
		}
	}

	if (_enemy.find(attacking)->second.flag == 0) {
		//看到敌人
		/*
		已知敌人的XY坐标/极坐标，行进的绝对角度、速度，shoot用的是相对于自身的极坐标位置
		创建一个assume_position，并往该处攻击
		*/
		PolarPosition assume_position = _enemy.find(attacking)->second.info.polar_pos;
		XYPosition new_XYPos = _enemy.find(attacking)->second.last_seen_pos, new_self_XYPos = info.self.xy_pos;
		if (_enemy.find(attacking)->second.info.status == 3 || _enemy.find(attacking)->second.info.status == 6) {
			//enemy可能往前走0.5、0.3个步幅，取1/3好了
			new_XYPos.x = _enemy.find(attacking)->second.last_seen_pos.x + (_enemy.find(attacking)->second.info.move_speed / 3) * cos(_enemy.find(attacking)->second.info.move_angle * PI / 180);
			new_XYPos.y = _enemy.find(attacking)->second.last_seen_pos.y + (_enemy.find(attacking)->second.info.move_speed / 3) * sin(_enemy.find(attacking)->second.info.move_angle * PI / 180);
			if (info.self.status == 3 || info.self.status == 6) {
				if (info.self.move_cd == 2) {
					new_self_XYPos.x = info.self.xy_pos.x + (info.self.move_speed / 2) * cos(info.self.move_angle * PI / 180);
					new_self_XYPos.y = info.self.xy_pos.y + (info.self.move_speed / 2) * sin(info.self.move_angle * PI / 180);
				}
				else if (info.self.move_cd == 1) {
					new_self_XYPos.x = info.self.xy_pos.x + (info.self.move_speed * 0.2) * cos(info.self.move_angle * PI / 180);
					new_self_XYPos.y = info.self.xy_pos.y + (info.self.move_speed * 0.2) * sin(info.self.move_angle * PI / 180);
				}
				else {
					new_self_XYPos.x = info.self.xy_pos.x + (info.self.move_speed * 0.3) * cos(info.self.move_angle * PI / 180);
					new_self_XYPos.y = info.self.xy_pos.y + (info.self.move_speed * 0.3) * sin(info.self.move_angle * PI / 180);
				}
			}
		}
		assume_position.distance = hypot(new_XYPos.x - new_self_XYPos.x, new_XYPos.y - new_self_XYPos.y);
		assume_position.angle = atan2(new_XYPos.y - new_self_XYPos.y, new_XYPos.x - new_self_XYPos.x) * 180 / PI - info.self.view_angle;
		while (assume_position.angle < 0) {
			assume_position.angle += 360;
		}
		fout << "assume self pos:" << new_self_XYPos.x << " " << new_self_XYPos.y << std::endl;
		fout << "attack assume_position:" << assume_position.distance << " " << assume_position.angle << std::endl;

		if (_enemy.find(attacking)->second.info.status == 0 || _enemy.find(attacking)->second.info.status == 5)
			use_sniper = true;

		//开始攻击
		//考虑散角，预估敌人取<移动速度/3>，与敌人在距离8以内，用远程武器可能会打不到，虽然远程武器普遍dps高于近战
		if (assume_position.distance <= 2) {
			//优先使用虎韬锤造成伤害 (true)为可能需要加上的判断条件
			if (hasWeapon[TIGER_BILLOW_HAMMER]) {
				shoot(TIGER_BILLOW_HAMMER, assume_position.angle);
				return;
			}
			else if (assume_position.distance <= ITEM_DATA[FIST].range) {
				shoot(FIST, assume_position.angle);
				return;
			}
			if (use_sniper) {
				if (hasWeapon[SNIPER_BARRETT]) {
					shoot(SNIPER_BARRETT, assume_position.angle);
					return;
				}
				else if (hasWeapon[SNIPER_RILFE]) {
					shoot(SNIPER_RILFE, assume_position.angle);
					return;
				}
			}
			else {
				return;
				//可能需要考虑使用CD=1的冲锋枪和机枪进行攻击，不过考虑到拳头都挥不动，估计也不用远程了
			}
		}
		else if (assume_position.distance <= info.self.view_distance) {
			//散角太小，用狙击？
			if (!use_sniper) {
				for (int i = 0; i < 6; ++i) {
					if (hasWeapon[ranged_weapon_order[i]] && assume_position.distance <= ITEM_DATA[ranged_weapon_order[i]].range) {
						shoot(ranged_weapon_order[i], assume_position.angle);
						return;
					}
				}
			}
			else if(assume_position.distance > 50 || fabs(assume_position.angle - _enemy.find(attacking)->second.info.polar_pos.angle) < 0.3){
				if (hasWeapon[SNIPER_BARRETT]) {
					shoot(SNIPER_BARRETT, assume_position.angle);
					return;
				}
				else if (hasWeapon[SNIPER_RILFE]) {
					shoot(SNIPER_RILFE, assume_position.angle);
					return;
				}
				else {
					for (int i = 0; i < 6; ++i) {
						if (hasWeapon[ranged_weapon_order[i]] && assume_position.distance <= ITEM_DATA[ranged_weapon_order[i]].range) {
							shoot(ranged_weapon_order[i], assume_position.angle);
							return;
						}
					}
				}
				return;
			}
			else {
				for (int i = 0; i < 6; ++i) {
					if (hasWeapon[ranged_weapon_order[i]] && assume_position.distance <= ITEM_DATA[ranged_weapon_order[i]].range) {
						shoot(ranged_weapon_order[i], assume_position.angle);
						return;
					}
				}
				if (hasWeapon[SNIPER_BARRETT]) {
					shoot(SNIPER_BARRETT, assume_position.angle);
					return;
				}
				else if (hasWeapon[SNIPER_RILFE]) {
					shoot(SNIPER_RILFE, assume_position.angle);
					return;
				}
			}
		}
		else {
			return;
			//按理来说这里是在看到敌人的情形下敌人在视距外，基本上除了狙击以外没有武器可以打的到
			//暂空
		}
	}

	else {
		//听见的情况
		/*
		听到的声音延迟是int型，对于枪声可能应该转头去看，脚步声可能需要盲打一下
		*/
		if (hasWeapon[MACHINE_GUN]) {
			shoot(MACHINE_GUN, _enemy.find(attacking)->second.info.polar_pos.angle);
			return;
		}
		else if (hasWeapon[SUBMACHINE_GUN]) {
			shoot(SUBMACHINE_GUN, _enemy.find(attacking)->second.info.polar_pos.angle);
			return;
		}
		else
			return;
	}
}