#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <math.h>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

/*player code*/
int bxb = 0;




/**/


void play_game()
{
	/* Your code in this function */


	/* sample AI */
	update_info();
	std::cout << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
	std::cout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	/*м╗пе*/
	
	/**/
	if (info.self.bag.size() > bxb)
	{
		std::cout << "pick succeed" << std::endl;
		bxb = info.self.bag.size();
	}
	
	
	if (info.self.bag.size() > 1)
	{
		std::cout << "pick succeed" << std::endl;
	
	}
	if (info.self.status != MOVING)
	{
		move(fabs(info.self.move_angle - atan((info.self.xy_pos.y - info.poison.current_center.y) / (info.self.xy_pos.x - info.poison.current_center.x))),
			fabs(info.self.move_angle - atan((info.self.xy_pos.y - info.poison.current_center.y) / (info.self.xy_pos.x - info.poison.current_center.x))));
	}
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 450 + rand() % 100, 250 + rand() % 100 };/*landing point*/
		parachute(SNIPER, landing_point);
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		std::cout << "jumping" << std::endl;
		return;
	}
	if (info.self.hp <= 20)
		radio(0, info.self.xy_pos.x * 10000 + info.self.xy_pos.y * 10 + 4);
	if (info.others.empty())
	{
		std::cout << "no others" << std::endl;
		if (info.items.empty())
		{
			//see nothing
			if (info.self.status != MOVING)
			{
				double move_angle = 0;
				double view_angle = move_angle;

					double xm = info.self.xy_pos.x;
					double ym = info.self.xy_pos.y;
					move(move_angle, view_angle);
					if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
					{
						std::cout << "move" << move_angle << std::endl;
					}
					else
						move(90, 90);
					
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
			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);
				std::cout << "try pickup" << closest_item.item_ID << std::endl;
			}
			else if (info.self.status != MOVING)
			{
				
					double xm = info.self.xy_pos.x;
					double ym = info.self.xy_pos.y;
					move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
					if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
					{
						std::cout << "move" << closest_item.polar_pos.angle << std::endl;
					
					}
					else
						move(90, 90);
				
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
			ITEM weapon = FIST;
			for (int i = 0; i < info.self.bag.size(); ++i)
			{
				if (ITEM_DATA[info.self.bag[i].type].type == SNIPER_BARRETT && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
				else if (ITEM_DATA[info.self.bag[i].type].type == SNIPER_RILFE && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
				else if (ITEM_DATA[info.self.bag[i].type].type == CROSSBOW && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
				else if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
			}
			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				
					double xm = info.self.xy_pos.x;
					double ym = info.self.xy_pos.y;
					move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
					if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
						std::cout << "move" << closest_enemy.polar_pos.angle << std::endl;
					else
						move(90, 90);
				
			}
			else
			{
				double cps = closest_enemy.polar_pos.angle,
					d = closest_enemy.polar_pos.distance,

					mova = closest_enemy.move_angle,
					r = 180 - mova + cps,
					d1;
				if (closest_enemy.player_ID == 0)
				{
					double ms = 1.8;
					d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
					double r2 = ms / d1 * sin(r);
					double shot = closest_enemy.polar_pos.angle + r2;
					if (shot > 360) shot -= 360;
					if (shot < 0) shot += 360;
					shoot(weapon, shot);
				}
				if (closest_enemy.player_ID == 1)
				{
					double ms = 1.6;
					d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
					double r2 = ms / d1 * sin(r);
					double shot = closest_enemy.polar_pos.angle + r2;
					if (shot > 360) shot -= 360;
					if (shot < 0) shot += 360;
					shoot(weapon, shot);
				}
				if (closest_enemy.player_ID == 2)
				{
					double ms = 1.2;
					d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
					double r2 = ms / d1 * sin(r);
					double shot = closest_enemy.polar_pos.angle + r2;
					if (shot > 360) shot -= 360;
					if (shot < 0) shot += 360;
					shoot(weapon, shot);
				}
				if (closest_enemy.player_ID == 3)
				{
					double ms = 1.0;
					d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
					double r2 = ms / d1 * sin(r);
					double shot = closest_enemy.polar_pos.angle + r2;
					if (shot > 360) shot -= 360;
					if (shot < 0) shot += 360;
					shoot(weapon, shot);
				}
			}
		}
	}

	/*player code*/


	/**/
	return;
}