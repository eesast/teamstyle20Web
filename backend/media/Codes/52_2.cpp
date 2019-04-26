#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

int sgn(double x)
{
	if (x > 0)
		return 1;
	else if (x < 0)
		return -1;
	else
		return 0;
}

bool barrier(double x, double y,block b)
{
	if (b.shape == CIRCLE)
	{
		if ((x - b.x0) * (x - b.x0) + (y - b.y0) * (y - b.y0) <= b.r * b.r)
			return true;
	}
	if (b.shape == RECTANGLE)
	{
		if (x>=x0 && y>=y0 && x<=x1 && y<=y1)
			return true;
	}
}

void diymove(double view_angle)
{
	double move_angle = view_angle;
	int x = floor(info.self.xy_pos.x);
	int y = floor(info.self.xy_pos.y);
	bool has_barrier = true;
	int n = 1;
	int j = 1;
	while(has_barrier)
	{
		has_barrier = false;
		for (int id = 0; id < info.landform_IDs.size(); id++)
		{
			block current_block = get_landform(info.landform_IDs[id]);
			if (current_block.type = DEEP_WATER || RECTANGLE_BUILDING || CIRCLE_BUILDING || WALL || TREE)
			{
				int xb = x;
				int yb = y;
				for (int k = 1; k <= 5; k++)
				{
					xb = floor(x + k cos(view_angle));
					yb = floor(y + k sin(view_angle));
					if (barrier(xb, yb, current_block) || barrier(xb + sgn(cos(move_angle)), yb, current_block) || barrier(xb, yb + sgn(sin(move_angle)), current_block))
						has_barrier = true;
				}
			}
		}
		if (has_barrier)
		{
			view_angle = move_angle + n * 10;
			j++;
			n = pow(-1,j-1)*floor(j/2);
		}
	}
	move(move_angle, view_angle)
}
void play_game()
{
	update_info();
	std::cout << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
	std::cout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	
	//跳伞
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 150 + rand() % 100, 150 + rand() % 100 };
		parachute(SNIPER, landing_point);
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}

	//是否落地
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		std::cout << "jumping" << std::endl;
		return;
	}

	//判断周围有没有人
	if (info.others.empty())
	{
		std::cout << "no others" << std::endl;
		//判断周围有没有物品
		if (info.items.empty())
		{
			std::cout << "no items" << std::endl;
			//进行移动
			if (info.self.status != MOVING)
			{
				double move_angle = 0;
				double view_angle = move_angle;
				if (PoisonInfo::move_flag == 1 || PoisonInfo::move_flag == 2)
				{
					double x = info.poison.current_center.x - info.self.xy_pos.x;
					double y = info.poison.current_center.y - info.self.xy_pos.y;
					move_angle = (x < 0) * (atan(y / x);
					view_angle = move_angle;
				}
				diymove(move_angle, view_angle);
				std::cout << "move" << move_angle << std::endl;
			}
		}
		else
			//捡东西
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
			if (closest_item.polar_pos.distance < 1)
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
	else
	{
		//有敌人
		bool has_enemy = false;
		OtherInfo closest_enemy;
		closest_enemy.polar_pos.distance = 100000;
		//判断友军or敌军
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
				radio(teammates[0], 1);
			}
		}
		//有敌军
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
			{
				move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			}//向敌人靠近
			else
			{
				shoot(weapon, closest_enemy.polar_pos.angle);
			}//射击
		}
		else
		{
			if (info.self.status != MOVING)
			{
				double move_angle = 0;
				double view_angle = move_angle;
				move(move_angle, view_angle);
				std::cout << "move" << move_angle << std::endl;
			}
		}
	}
	return;
}