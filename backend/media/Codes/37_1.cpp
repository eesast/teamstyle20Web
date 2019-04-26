#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;
extern int s = 0;
extern int flag = 0;
void Move(double move_angle, double view_angle)
{
	if (s == 0)
	{
		double move_angle1; double move_angle2;
		if (move_angle < 45)
		{
			move_angle1 = move_angle + 360 - 45;
		}
		if (move_angle >= 45)
		{
			move_angle1 = move_angle - 45;
		}
		move(fabs(move_angle1), fabs(view_angle));
		s = 1;
	}
	if (s == 1)
	{
		double move_angle1; double move_angle2;
		if (move_angle > 315)
		{
			move_angle1 = move_angle - 360 + 45;
		}
		if (move_angle <= 315)
		{
			move_angle1 = move_angle - 360 + 45;
		}
		move(fabs(move_angle1), fabs(view_angle));
		s = 0;
	}
}

void play_game()
{
	/* Your code in this function */
	/* sample AI */
	update_info();
	std::cout << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
	std::cout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
	bool a = try_update_info();

	if (info.self.bag.size() > 1)
		std::cout << "pick succeed" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[1]);
		XYPosition landing_point = { 450+rand()%100, 255+rand()%100 };
		parachute(SIGNALMAN, landing_point);
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
	if (info.self.status != MOVING
		&& (info.poison.current_center.y - info.self.xy_pos.y)*(info.poison.current_center.y - info.self.xy_pos.y)+ (info.poison.current_center.x - info.self.xy_pos.x)*(info.poison.current_center.x - info.self.xy_pos.x)>=400)
	{

		double move_angle = atan(fabs(info.poison.current_center.y - info.self.xy_pos.y) / fabs(info.poison.current_center.x - info.self.xy_pos.y));
		double view_angle = move_angle;
		
			double xm = info.self.xy_pos.x;
			double ym = info.self.xy_pos.y;
			move(fabs(move_angle), fabs(view_angle));
			if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
			{
				std::cout << "move" << move_angle << std::endl;
			
			}
			else
				move(90, 90);
		
	}
	if (info.self.hp <= 20)
		radio(0, info.self.xy_pos.x * 10000 + info.self.xy_pos.y * 10 + 4);
	if (a == false)
	{
		double move_angle = 0;
		double view_angle = move_angle;
		move(fabs(move_angle),fabs(view_angle));
		std::cout << "move" << move_angle << std::endl;
	}
	else
	{
		int x;
		for (int i = 0; i < info.sounds.size(); ++i)
		{
			Sound Wireless = info.sounds[i];
			if (Wireless.sender == 0 || Wireless.sender == 2 || Wireless.sender == 3)
			{
				x = Wireless.parameter; break;
			}
		}
		if (x % 10 == 1)
		{

			int x2 = x / 10000; int y2 = (x / 10) % 1000;
			double move_angle = atan(fabs(y2 - info.self.xy_pos.y) / fabs(x2 - info.self.xy_pos.y));
			double view_angle = move_angle;
			
				double xm = info.self.xy_pos.x;
				double ym = info.self.xy_pos.y;
				move(fabs(move_angle), fabs(view_angle));
				if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
				{
					std::cout << "move" << move_angle << std::endl;
			
				}
				else
					move(90, 90);
			
		}
		if (x % 10 == 2)
		{

			int x2 = x / 10000; int y2 = (x / 10) % 1000;
			double move_angle = atan(fabs(y2 - info.self.xy_pos.y) / fabs(x2 - info.self.xy_pos.y));
			if (move_angle < 270) move_angle += 90;
			else
			{
				move_angle -= 90;
			}
			double view_angle = move_angle;
		
				double xm = info.self.xy_pos.x;
				double ym = info.self.xy_pos.y;
				move(fabs(move_angle), fabs(view_angle));
				if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
				{
					std::cout << "move" << move_angle << std::endl;
					
				}
				else
					move(90, 90);
			
		}
		if (x % 10 == 3)
				{

					int x2 = x / 10000; int y2 = (x / 10) % 1000;
					double move_angle = atan(fabs(y2 - info.self.xy_pos.y) / fabs(x2 - info.self.xy_pos.y));
					double view_angle = move_angle;
					
						double xm = info.self.xy_pos.x;
						double ym = info.self.xy_pos.y;
						move(fabs(move_angle), fabs(view_angle));
						if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
						{
							std::cout << "move" << move_angle << std::endl;
					
						}
						else
							move(90, 90);
					
				}
			}
	if (info.others.empty())
			{
				int xsg = info.self.xy_pos.x * 10000 + info.self.xy_pos.y * 10 + 1;
				radio(2, xsg);
				std::cout << "no others" << std::endl;
				if (info.items.empty())
				{
					//see nothing
					if (info.self.status != MOVING)
					{
						move(fabs(info.self.move_angle - atan(fabs(info.self.xy_pos.y - info.poison.current_center.y) / fabs(info.self.xy_pos.x - info.poison.current_center.x))),
							fabs(info.self.move_angle - atan(fabs(info.self.xy_pos.y - info.poison.current_center.y) / fabs(info.self.xy_pos.x - info.poison.current_center.x))));
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

					if (closest_item.item_ID == 2 || closest_item.item_ID == 1)
					{
						int xx = info.self.xy_pos.x + cos(closest_item.polar_pos.angle)*closest_item.polar_pos.distance;
						int yy = info.self.xy_pos.x + cos(closest_item.polar_pos.angle)*closest_item.polar_pos.distance;
						xsg = xx * 10000 + yy * 10 + 1;
						radio(0, xsg);
					}
					else if (closest_item.polar_pos.distance < 1)
					{
						pickup(closest_item.item_ID);
						std::cout << "try pickup" << closest_item.item_ID << std::endl;
					}
					else if (info.self.status != MOVING)
					{
						
						
							double xm = info.self.xy_pos.x;
							double ym = info.self.xy_pos.y;
							move(fabs(closest_item.polar_pos.angle), fabs(closest_item.polar_pos.angle));
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
				int x;
				Sound Wireless;
				for (int i = 0; i < info.sounds.size(); ++i)
				{
					Wireless = info.sounds[i];
					if (Wireless.sender == -1) break;
				}
				if (Wireless.type == 1 || Wireless.type == 2)
				{
					int xx = info.self.xy_pos.x + cos(closest_enemy.polar_pos.angle)*closest_enemy.polar_pos.distance;
					int yy = info.self.xy_pos.x + cos(closest_enemy.polar_pos.angle)*closest_enemy.polar_pos.distance;
					int xsg = xx * 10000 + yy * 10 + 2;
					radio(2, xsg); radio(3, xsg);
					double move_angle;
					if (Wireless.angle < 270)
						move_angle = Wireless.angle + 90;
					else move_angle = Wireless.angle - 90;
					move(fabs(move_angle), fabs(move_angle));
				}
				if (has_enemy)
				{
					int xx = info.self.xy_pos.x + cos(closest_enemy.polar_pos.angle)*closest_enemy.polar_pos.distance;
					int yy = info.self.xy_pos.x + cos(closest_enemy.polar_pos.angle)*closest_enemy.polar_pos.distance;
					int xsg = xx * 10000 + yy * 10 + 3;
					radio(0, xsg);
					ITEM weapon = FIST;
					for (int i = 9; i < info.self.bag.size() && i>0; --i)
					{
						if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
						{
							weapon = info.self.bag[i].type;
							break;
						}
					}

					if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
					{
						Move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
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
							shoot(weapon, fabs(shot));
						}
						if (closest_enemy.player_ID == 1)
						{
							double ms = 1.6;
							d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
							double r2 = ms / d1 * sin(r);
							double shot = closest_enemy.polar_pos.angle + r2;
							if (shot > 360) shot -= 360;
							if (shot < 0) shot += 360;
							shoot(weapon, fabs(shot));
						}
						if (closest_enemy.player_ID == 2)
						{
							double ms = 1.2;
							d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
							double r2 = ms / d1 * sin(r);
							double shot = closest_enemy.polar_pos.angle + r2;
							if (shot > 360) shot -= 360;
							if (shot < 0) shot += 360;
							shoot(weapon, fabs(shot));
						}
						if (closest_enemy.player_ID == 3)
						{
							double ms = 1.0;
							d1 = sqrt(ms*ms + d * d - 2 * ms * d*cos(r));
							double r2 = ms / d1 * sin(r);
							double shot = closest_enemy.polar_pos.angle + r2;
							if (shot > 360) shot -= 360;
							if (shot < 0) shot += 360;
							shoot(weapon, fabs(shot));
						}
					}
				}
			}
			return;
		}
	