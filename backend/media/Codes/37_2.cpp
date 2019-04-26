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

void Move(double move_angle, double view_angle)
{
	int flag=0;
	double move_angle1; double move_angle2;
	if (flag == 0)
	{
		if (move_angle < 45)
		{
			move_angle1 = move_angle + 360 - 45;
		}
		if (move_angle >= 45)
		{
			move_angle1 = move_angle - 45;
		}
		move(move_angle1, view_angle);
		flag = 1;
	}
	if (flag == 1)
	{
		if (move_angle > 315)
		{
			move_angle1 = move_angle - 360 + 45;
		}
		if (move_angle <= 315)
		{
			move_angle1 = move_angle - 360 + 45;
		}
		move(move_angle1, view_angle);
		flag = 0;
	}
}

void play_game()
{
	/* Your code in this function */
	/* sample AI */
	update_info();
	std::cout << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
	std::cout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	if (info.self.bag.size() > 1)
		std::cout << "pick succeed" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = {450+ rand() % 10,250+ rand() % 10 };
		parachute(HACK, landing_point);
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

	
	if (info.self.hp <= 20 || info.self.status == DEAD)
	{
		int Send_wireless;
		Send_wireless = info.self.xy_pos.x * 10000 + info.self.xy_pos.y * 10 + 4;
		radio(0,Send_wireless);
	}
	for (int i=0; i < info.sounds.size(); ++i)
	{
		Sound Wireless = info.sounds[i];
		if (Wireless.sender == 0 || Wireless.sender == 1 || Wireless.sender == 3)
		{
			int wireless = Wireless.parameter;
			int wireless_x, wireless_y, wireless_Com;
			wireless_Com = wireless % 10;
			wireless_y = (wireless % 10000 - wireless % 10) / 10;
			wireless_x = wireless / 10000;
			if (wireless_Com == 1 && info.self.status != MOVING)
			{
				int Yo = info.self.xy_pos.y;
				int Xo = info.self.xy_pos.x;
				move(fabs(atan((wireless_y - info.self.xy_pos.y) / wireless_x - info.self.xy_pos.x)),
					fabs(atan((wireless_y - info.self.xy_pos.y) / wireless_x - info.self.xy_pos.x)));
				if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
				else move(90, 90);
			}
			if (wireless_Com == 2 && info.self.status != MOVING)
			{
				int Yo = info.self.xy_pos.y;
				int Xo = info.self.xy_pos.x;
				move(90, 90);
				if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
				else move(90, 90);
				for (int i = 0; i < 5; ++i)
				{
					int Yo = info.self.xy_pos.y;
					int Xo = info.self.xy_pos.x;
					move(0, 0);
					if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
					else move(90, 90);
				}
			}
			break;
		}
	}

	if (info.others.empty())
	{
		if (info.self.status != MOVING)
		{
			int Yo = info.self.xy_pos.y;
			int Xo = info.self.xy_pos.x;
			int angle = info.self.move_angle - atan((info.self.xy_pos.y - info.poison.current_center.y) / (info.self.xy_pos.x - info.poison.current_center.x));
			if (angle < 0) angle = angle + 360;
			move(angle,angle);
			if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
			else move(90, 90);
		}
		std::cout << "no others" << std::endl;
		if (info.items.empty())
		{
			//see nothing
			if (info.self.status != MOVING)
			{
				double move_angle = 0;
				double view_angle = move_angle;
				int Yo = info.self.xy_pos.y;
				int Xo = info.self.xy_pos.x;
				move(move_angle, view_angle);
				if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
				else move(90, 90);
				std::cout << "move" << move_angle << std::endl;
			}
		}
		else
		{
			Item best_item;
			Item *Found_item=new Item[info.items.size()];
			best_item.polar_pos.distance = 100;
			int i;
			bool Get_Item;
			for (i = 0; i < info.items.size(); ++i)
				for (int k = 0; k < i; ++k)
					if (info.items[i].polar_pos.distance < best_item.polar_pos.distance)
						Found_item[k] = info.items[i];
			for (int k = 0; k < i; ++k)
			{
				if (Found_item[k].type == 0)
				{
					best_item = Found_item[k];
					Get_Item = true;
					break;
				}
				else continue;
			}
			if (!Get_Item)
			{
				for (int k = 0; k < i; ++k)
				{
					if (Found_item[k].type == 1)
					{
						best_item = Found_item[k];
						Get_Item = true;
						break;
					}
					else continue;
				}
			}
			if (!Get_Item) best_item = Found_item[0];
			delete[] Found_item;
			int Yo = info.self.xy_pos.y;
			int Xo = info.self.xy_pos.x;
			move(best_item.polar_pos.angle, best_item.polar_pos.angle);
			if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
			else move(90, 90);
			std::cout << "status" << info.self.status << std::endl;
			std::cout << "**best item angle" << best_item.polar_pos.angle << "distance" << best_item.polar_pos.distance << "**" << std::endl;
			if (best_item.polar_pos.distance < 1)
			{
				pickup(best_item.item_ID);
				std::cout << "try pickup" << best_item.item_ID << std::endl;
			}
			else if (info.self.status != MOVING)
			{
				move(best_item.polar_pos.angle, best_item.polar_pos.angle);
				std::cout << "move" << best_item.polar_pos.angle << std::endl;
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
			int Yo = info.self.xy_pos.y;
			int Xo = info.self.xy_pos.x;
			Move(info.self.move_angle, info.self.view_angle);
			if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
			else move(90, 90);
			ITEM weapon = FIST;
			for (int i = info.self.bag.size()-1; i >=0 ; --i)
			{
				if (ITEM_DATA[info.self.bag[i].type].type == WEAPON&& weapon != FIST&&weapon!= TIGER_BILLOW_HAMMER && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
			}
			if (weapon == FIST)
			{
				int Yo = info.self.xy_pos.y;
				int Xo = info.self.xy_pos.x;
				move(90, 90);
				if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
				else move(90, 90);
				for (int i = 0; i < 5; ++i)
				{
					int Yo = info.self.xy_pos.y;
					int Xo = info.self.xy_pos.x;
					move(0, 0);
					if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
					else move(90, 90);
				}
			}
			else if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				int Yo = info.self.xy_pos.y;
				int Xo = info.self.xy_pos.x;
				move(90, 90);
				if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
				else move(90, 90);
				for (int i = 0; i < 5; ++i)
				{
					int Yo = info.self.xy_pos.y;
					int Xo = info.self.xy_pos.x;
					move(0, 0);
					if (Xo != info.self.xy_pos.x&&Yo != info.self.xy_pos.y);
					else move(90, 90);
				}
			}
			else
			{
				double cps = closest_enemy.polar_pos.angle;
				double d = closest_enemy.polar_pos.distance;
				double mova = closest_enemy.move_angle;
				double r = 180 - mova + cps;
				double d1;
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
	return;
}
