#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

std::ofstream out("info.txt");
std::ofstream process("process.txt");
void print_info(int level);

void play_game()
{
	update_info();
	print_info(0);
	process << "frame:" << frame << std::endl;

	//跳伞
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 150 + rand() % 100, 150 + rand() % 100 };
		parachute(MEDIC, landing_point);
		return;
	}
	else
		srand(time(nullptr) + info.player_ID*frame);

	//是否未落地
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		process << "jumping" << std::endl;
		return;
	}

	//是否有其他人
	if (info.others.empty())
	{
		process << "no others" << std::endl;
		if (info.items.empty())
		{
			if (info.self.status != MOVING)
			{
				double x = info.poison.current_center.x - info.self.xy_pos.x;
				double y = info.poison.current_center.y - info.self.xy_pos.y;
				double move_angle = (x < 0)*(atan(y / x) + rand() % 135);
				double view_angle = move_angle;
				move(move_angle, view_angle);
				process << "move" << move_angle << std::endl;
			}
		}
		else
		{
			Item closest_item;
			closest_item.polar_pos.distance = 100000;

			for (int i = 0; i < info.items.size(); ++i)
				if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
					closest_item = info.items[i];

			process << "status" << info.self.status << std::endl;
			process << "**closest item angle" << closest_item.polar_pos.angle << "distance" << closest_item.polar_pos.distance << "**" << std::endl;
			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);
				process << "try pickup" << closest_item.item_ID << std::endl;
			}
			else if (info.self.status != MOVING)
			{
				move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
				process << "move" << closest_item.polar_pos.angle << std::endl;
			}
		}
	}
	else
	{
		bool has_enemy = false;
		OtherInfo closest_enemy;
		closest_enemy.polar_pos.distance = 100000;
		//check teammate
		for (int i = 0; i < info.others.size(); ++i)
		{
			bool is_friend = false;
			for (int teammate = 0; teammate < teammates.size(); ++teammate)
				if (info.others[i].player_ID == teammates[teammate])
				{
					is_friend = true;
					break;
				}

			if (!is_friend && info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
			{
				closest_enemy = info.others[i];
				has_enemy = true;
			}
		}
		if (has_enemy)
		{
			ITEM weapon = FIST;
			for (int i = 0; i < info.self.bag.size(); ++i)
			{
				if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
			}
			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
				move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			else
				shoot(weapon, closest_enemy.polar_pos.angle);
		}
	}
	return;
}

void print_info(int level)
{
	out << "frame:" << frame << std::endl;

	for (int temp = 0; temp < info.others.size(); temp++)
		out << "others:" << info.others[temp].player_ID << std::endl;
	for (int temp = 0; temp < info.sounds.size(); temp++)
		out << "sounds:" << info.sounds[temp].type << std::endl;
	for (int temp = 0; temp < info.self.bag.size(); temp++)
		out << "items:" << info.self.bag[temp].type << std::endl;

	out << "self:" << info.self.hp << std::endl;
	out << "self:" << info.self.xy_pos.x << " " << info.self.xy_pos.y << std::endl;
	out << std::endl;
}