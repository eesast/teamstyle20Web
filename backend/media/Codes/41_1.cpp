#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#define pi 3.1415926535
#pragma warning (disable:0145)
using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;
void Move_dir(XYPosition destination, double new_view_angle)
{
	double view_angle = info.self.view_angle;
	destination.x = destination.x - info.self.xy_pos.x;
	destination.y = destination.y - info.self.xy_pos.y;
	double destination_angle = (atan2(destination.y, destination.x) / pi + 2) * 180;
	while (destination_angle - view_angle >= 360) destination_angle -= 360;
	while (destination_angle - view_angle < 0) destination_angle += 360;
	move(destination_angle - view_angle, new_view_angle - view_angle);
}
void Move_rect(XYPosition destination)
{
	XYPosition middle;
	middle.x = destination.x;
	middle.y = info.self.xy_pos.y;
	if (info.self.xy_pos.x - middle.x <= 2 && info.self.xy_pos.x - middle.x >= -2)
	{
		Move_dir(destination, info.self.view_angle);
	}
	else
	{
		Move_dir(middle, info.self.view_angle);
	}
}
void go_into_circle()
{
	double deltax = abs(info.self.xy_pos.x - info.poison.current_center.x);
	double deltay = abs(info.self.xy_pos.y - info.poison.current_center.y);
	double r = info.poison.current_radius;
	if (deltax * deltax + deltay * deltay >= 0.5*r*r)
	{ 
		Move_dir(info.poison.current_center, info.self.view_angle);
	}
} 
void play_game()
{
	/* Your code in this function */
	/* sample AI */
	XYPosition former_position = info.self.xy_pos;
	update_info();
	std::cout << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
	std::cout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	if (info.self.bag.size() > 1)
		std::cout << "pick succeed" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 450,300 + rand() % 10 };
		parachute(MEDIC, landing_point);
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}

	Item weapon;
	double damage2 = 0;
	int j = 0;
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		if (ITEM_DATA[info.self.bag[i].item_ID].type == WEAPON && info.self.bag[i].durability > 0)
		{
			if (ITEM_DATA[weapon.item_ID].damage > damage2)
			{
				damage2 = ITEM_DATA[weapon.item_ID].damage;
				weapon = info.self.bag[i];
			}
		}
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		std::cout << "jumping" << std::endl;
		return;
	}
	if (abs(former_position.x - info.self.xy_pos.x) <= 0.1&&abs(former_position.y - info.self.xy_pos.y) <= 0.1&&info.self.status != PICKUP)
	{
		;
		move(90, 90);
	}
	else
	{
		if (info.others.empty())
		{
			std::cout << "no others" << std::endl;
			if (info.items.empty())
			{
				//see nothing
				if (info.self.status != MOVING)
				{

					go_into_circle();
					std::cout << "move" << info.self.move_angle << std::endl;
				}
			}
			else
			{
				Item closest_item;
				closest_item.polar_pos.distance = 100000;
				for (int i = 0; i < info.items.size(); ++i)
				{
					if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
					{
						closest_item = info.items[i];
					}
				}
				std::cout << "status" << info.self.status << std::endl;
				std::cout << "**closest item angle" << closest_item.polar_pos.angle << "distance" << closest_item.polar_pos.distance << "**" << std::endl;
				if (closest_item.polar_pos.distance < 1 || info.self.bag.size() < 8)
				{
					pickup(closest_item.item_ID);
					std::cout << "try pickup" << closest_item.item_ID << std::endl;
				}
				else if (info.self.status != MOVING)
				{
					move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
					std::cout << "move" << closest_item.polar_pos.angle << std::endl;
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
				{
					if (info.others[i].player_ID == teammates[teammate])
					{
						is_friend = true;
						break;
					}
				}
				if (!is_friend && info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
				{
					closest_enemy = info.others[i];
					has_enemy = true;
				}
			}
			if (has_enemy)
			{
				if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon.type].range)
				{

					move(closest_enemy.polar_pos.angle, 0);

				}
				else if (info.self.bag[j].durability > 0)
				{
					shoot(weapon.type, closest_enemy.polar_pos.angle);
				}
				else
				{
					weapon.type = FIST;
					if (info.self.status != PICKUP || info.self.status != MOVING || info.self.status != MOVING_SHOOTING)
					{
						go_into_circle();
					}
				}
			}
		}
	}
}
