#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <algorithm>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;


const double PI = acos(-1);

inline XYPosition polar_to_XY(const PolarPosition polar)
{
	// cast PolarPosition seen by the player to XYPosition on map
	double angle = (info.self.view_angle + polar.angle) * 180 / PI;
	return XYPosition{ info.self.xy_pos.x + polar.distance * cos(angle), info.self.xy_pos.y + polar.distance * sin(angle) };
}

inline PolarPosition XY_to_polar(const XYPosition dst)
{
	// cast XYPosition to PolarPosition seen by self
	double d_x = dst.x - info.self.xy_pos.x, d_y = dst.y - info.self.xy_pos.y;
	double angle = atan2(d_y, d_x) * 180 / acos(-1) - info.self.view_angle; // -540 ~ 180
	while (angle < 0)
		angle += 360;
	return PolarPosition{ hypot(d_x, d_y), angle };
}

void walk(PolarPosition polar)
{
	static XYPosition last_pos = info.self.xy_pos;
	const XYPosition cur_pos = info.self.xy_pos;
	XYPosition dst = { cur_pos.x + cos(polar.angle * 180 / PI), cur_pos.y + sin(polar.angle * 180 / PI) };
	bool not_moved = fabs(cur_pos.x - last_pos.x) < 0.05 && fabs(cur_pos.y - last_pos.y) < 0.05, movable = true;
	static int cnt = 0;
	if (not_moved)
	{
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
		move(_a, polar.angle);
		last_pos = cur_pos;
		return;
	}
	else
		cnt = 0;
	double _r = VOCATION_DATA[info.self.vocation].radius;
	AREA dst_landform = MAP[int(dst.x) / 100 + int(dst.y) / 100 * 10];
	if ((int(dst.x) / 100 != int(cur_pos.x) / 100 || int(dst.y) / 100 != int(cur_pos.y) / 100) && dst_landform == SEA)
	{
		// move into new landform
		// landform that we don't go into, currently only SEA
		movable = false;
		if (int(dst.x) / 100 != int(cur_pos.x) / 100)
		{
			double move_angle = cos(polar.angle * 180 / PI) > 0 ? 90 : 270;
			move_angle -= info.self.view_angle;
			if (move_angle < 0)
				move_angle += 360;
			else if (move_angle > 360)
				move_angle -= 360;
			move(move_angle, polar.angle);
			// dst = { cur_pos.x + cos(polar.angle) > 0 ? 1 : -1, cur_pos.y };
		}
		else
		{
			double move_angle = sin(polar.angle * 180 / PI) > 0 ? 0 : 180;
			move_angle -= info.self.view_angle;
			if (move_angle < 0)
				move_angle += 360;
			else if (move_angle > 360)
				move_angle -= 360;
			move(move_angle, polar.angle);
			// dst = { cur_pos.x, cur_pos.y + sin(polar.angle) > 0 ? 1 : -1 };
		}
	}
	else
	{
		double _x = dst.x - 100 * int(dst.x / 100), _y = dst.y - 100 * int(dst.y / 100);
		for (auto &a : AREA_DATA[dst_landform])
		{
			if (a.type == SHALLOW_WATER || a.type == CIRCLE_GRASS || a.type == RECTANGLE_GRASS)
				continue;
			if (a.shape == CIRCLE && hypot(_x - a.x0, _y - a.y0) < a.r + _r + 0.1)
			{
				movable = false;
				PolarPosition obs_polar = XY_to_polar(XYPosition{ double(a.x0), double(a.y0) });
				double _a = obs_polar.angle - polar.angle, _fa = asin((a.r + _r) / obs_polar.distance) * 180 / PI + 5;
				double move_angle = obs_polar.angle;
				if (_a > 0 && _a < 180 || _a > -180)
					move_angle += _fa;
				else
					move_angle -= _fa;
				move(move_angle, move_angle);
				break;
			}
			if (a.shape == RECTANGLE && _x + _r + 0.1 > a.x0 && _x - _r - 0.1 < a.x1 && _y - _r - 0.1 < a.y0 && _y + _r + 0.1 > a.y1)
			{
				if (cur_pos.x > a.x0 && cur_pos.x < a.x1)
				{
					movable = false;
					double move_angle = sin(polar.angle * 180 / PI) > 0 ? 0 : 180;
					move_angle -= info.self.view_angle;
					if (move_angle < 0)
						move_angle += 360;
					else if (move_angle > 360)
						move_angle -= 360;
					move(move_angle, polar.angle);
					break;
				}
				else
				{
					movable = false;
					double move_angle = cos(polar.angle * 180 / PI) > 0 ? 90 : 270;
					move_angle -= info.self.view_angle;
					if (move_angle < 0)
						move_angle += 360;
					else if (move_angle > 360)
						move_angle -= 360;
					move(move_angle, polar.angle);
					break;
				}
			}
		}
	}
	if (movable)
		move(polar.angle, polar.angle);
	last_pos = cur_pos;
}

const char* STATUS_N[] =
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

const char* VOCATION_N[] =
{
	"MEDIC",
	"SIGNALMAN",
	"HACK",
	"SNIPER",
	"VOCATION_SZ"
};

const char* ITEM_TYPE_N[]
{
	"WEAPON",
	"ARMOR",
	"GOODS",
	"ITEM_TYPE_SZ"
};

const char* ITEM_MODE_N[] =
{
	"PORTABLE",
	"SPENDABLE",
	"TRIGGERED",
	"ITEM_MODE_SZ"
};

const char* ITEM_N[] =
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

const char* SOUND_N[] =
{
	"RADIO_VOICE",
	"FOOTSTEP_SOUND",
	"GUN_SOUND",
	"BOOM_SOUND",
	"SOUND_SZ"
};

const char* BLOCK_SHAPE_N[] =
{
	"CIRCLE",
	"RECTANGLE",
	"BLOCK_SHAPE_SZ"
};

const char* BLOCK_TYPE_N[] =
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

const char* AREA_N[] =
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
	static bool see_item = false;
	static bool attacking = false;
	bool healing = true;

	update_info();

	std::ofstream fout;
	const std::string out_name = std::to_string(info.player_ID) + std::string(".txt");
	fout.open(out_name, std::ios::app);

	fout << "player:frame" << frame << "\nhp:" << info.self.hp << ' ' << VOCATION_N[info.self.vocation] << std::endl;
	fout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	fout << "bag info: [";
	for (auto &a : info.self.bag)
		fout << ITEM_N[a.type] << ',';
	fout << ']' << std::endl;

	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 450.0 + rand() % 100, 250.0 + rand() % 100 };
		parachute(HACK, landing_point);
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID * frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		fout << "jumping" << std::endl;
		return;
	}
	if (info.self.hp < 30)
	{
		bool has_bondage = false, has_first_aid = false;
		for (auto &_item : info.self.bag)
		{
			if (_item.type == BONDAGE)
				has_bondage = true;
			if (_item.type == FIRST_AID_CASE)
				has_first_aid = true;
		}
		if (has_first_aid)
			shoot(FIRST_AID_CASE, 0.0);
		else if (has_bondage)
			shoot(BONDAGE, 0.0);
		else
			healing = false;
	}
	if (!attacking && info.poison.current_radius > 0 && hypot(info.self.xy_pos.x - info.poison.current_center.x, info.self.xy_pos.y - info.poison.current_center.y) > info.poison.current_radius - 100)
	{
		fout << info.poison.current_center.x << "**" << info.poison.current_center.y << "**" << info.poison.current_radius << std::endl;
		walk(XY_to_polar(info.poison.current_center));
	}
	else if (!attacking && info.poison.current_radius > 0 && info.poison.move_flag == 0 && info.poison.rest_frames <= 30)
	{
		fout << info.poison.next_center.x << "&&" << info.poison.next_center.y << "&&" << info.poison.next_radius << "&&" << info.poison.rest_frames << std::endl;
		walk(XY_to_polar(info.poison.current_center));
	}
	else if (info.others.empty())
	{
		attacking = false;
		fout << "no others" << std::endl;
		if (info.items.empty())
		{
			//see nothing
			if (info.self.status != MOVING)
			{
				double move_angle = rand() % 360;
				double view_angle = move_angle;
				move(move_angle, view_angle);
				fout << "move" << move_angle << std::endl;
			}
		}
		else
		{
			static Item closest_item;
			static XYPosition closest_item_pos;
			if (!see_item)
				closest_item.polar_pos.distance = 100000;
			for (int i = 0; i < info.items.size(); ++i)
			{
				if ((info.self.vocation == HACK || info.items[i].type != CODE_CASE) && info.items[i].polar_pos.distance < XY_to_polar(closest_item_pos).distance)
				{
					closest_item = info.items[i];
					closest_item_pos = polar_to_XY(closest_item.polar_pos);
					see_item = true;
				}
			}
			fout << "status" << STATUS_N[info.self.status] << std::endl;
			fout << "**closest item angle" << closest_item.polar_pos.angle << "distance" << closest_item.polar_pos.distance << "**" << std::endl;
			static int tried = -1;
			if (closest_item.polar_pos.distance < 1 && tried != closest_item.item_ID)
			{
				pickup(closest_item.item_ID);
				fout << "try pickup" << closest_item.item_ID << ' ' << ITEM_N[closest_item.type] << std::endl;
				tried = closest_item.item_ID;
			}
			else if (tried == closest_item.item_ID)
			{
				see_item = false;
				walk(PolarPosition{ 100.0, double(rand() % 360) });
			}
			else if (info.self.status != MOVING)
			{
				walk(closest_item.polar_pos);
			}
		}
	}
	else
	{
		bool has_enemy = false;
		OtherInfo closest_enemy;
		closest_enemy.polar_pos.distance = 100000;
		closest_enemy.polar_pos.angle = 0;
		//check teammate
		for (int i = 0; i < info.others.size(); ++i)
		{
			bool is_friend = false;
			for (int teammate = 0; teammate < teammates.size(); ++teammate)
			{
				if (info.others[i].player_ID == teammates[teammate])
				{
					is_friend = true;
					break;
				}
			}
			if (!is_friend && info.others[i].status != 7 && info.others[i].status != 8 && info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
			{
				closest_enemy = info.others[i];
				has_enemy = true;
			}
		}
		if (has_enemy)
		{
			ITEM weapon = FIST;
			fout << "################" << closest_enemy.polar_pos.distance << "###" << closest_enemy.polar_pos.angle << std::endl;
			int i;
			for (i = 0; i < info.self.bag.size(); ++i)
			{
				if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && info.self.bag[i].type > weapon && info.self.bag[i].type < 10)
				{
					if (info.self.bag[i].type == TIGER_BILLOW_HAMMER && weapon != FIST)
						continue;
					weapon = info.self.bag[i].type;
				}
			}
			fout << ITEM_N[weapon] << std::endl;
			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				walk(closest_enemy.polar_pos);
			}
			else if (!healing)
			{
				shoot(weapon, closest_enemy.polar_pos.angle);
				attacking = true;
			}
		}
		else
		{
			attacking = false;
			if (info.items.empty())
			{
				//see nothing
				if (info.self.status != MOVING)
				{
					double move_angle = rand() % 360;
					double view_angle = 0;
					move(move_angle, view_angle);
					fout << "move" << move_angle << std::endl;
				}
			}
			else
			{
				static Item closest_item;
				static XYPosition closest_item_pos;
				if (!see_item)
					closest_item.polar_pos.distance = 100000;
				for (int i = 0; i < info.items.size(); ++i)
				{
					if ((info.self.vocation == HACK || info.items[i].type != CODE_CASE) && info.items[i].polar_pos.distance < XY_to_polar(closest_item_pos).distance)
					{
						closest_item = info.items[i];
						closest_item_pos = polar_to_XY(closest_item.polar_pos);
						see_item = true;
					}
				}
				fout << "status" << STATUS_N[info.self.status] << std::endl;
				fout << "**closest item angle" << closest_item.polar_pos.angle << "distance" << closest_item.polar_pos.distance << "**" << std::endl;
				static int _tried = -1;
				if (closest_item.polar_pos.distance < 1 && _tried != closest_item.item_ID)
				{
					pickup(closest_item.item_ID);
					fout << "try pickup" << closest_item.item_ID << ' ' << ITEM_N[closest_item.type] << std::endl;
					_tried = closest_item.item_ID;
				}
				else if (_tried == closest_item.item_ID)
				{
					see_item = false;
					walk(PolarPosition{ 100.0, double(rand() % 360) });
				}
				else if (info.self.status != MOVING)
				{
					walk(closest_item.polar_pos);
				}
			}
		}
	}
	return;
}
