#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include<cstdio>
#include<cmath>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;
const double Pi = 3.14159265;
double AccurateAngle(double Angle)
{
	if (Angle < 0)
	{
		for (; Angle < 0; Angle += 360) {  }
	}
	if (Angle >= 360)
	{
		for (; Angle >= 360; Angle -= 360) {  }
	}
	return Angle;
}
PolarPosition XYPostionToMyPolarPosition(double XPosition, double YPosition)
{
	PolarPosition ToMyPolarPosition = {sqrt((XPosition - info.self.xy_pos.x)*(XPosition - info.self.xy_pos.x)+(YPosition - info.self.xy_pos.y)*(YPosition - info.self.xy_pos.y)),(atan2(info.self.xy_pos.y - YPosition,info.self.xy_pos.x - XPosition)/Pi+1)*180};
	return ToMyPolarPosition;
}


void play_game()
{
	update_info();
	if (info.self.bag.size() > 1)
		std::cout << "pick succeed" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = {250+ rand()% 50 ,300 + rand()%50};
		parachute(MEDIC, landing_point);
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
	if (frame == 1)
	{
		move(AccurateAngle((XYPostionToMyPolarPosition(500, 500).angle - info.self.view_angle)), XYPostionToMyPolarPosition(500, 500).angle);
	}
	//没看见人
	if (info.others.empty())
	{
		std::cout << "no others" << std::endl;
		//没看见东西
		//吃药
		if (info.self.attack_cd == 0 && info.self.hp < 60)
		{
			bool GetBondage = false, GetFirst_Aid_Case = false;
			for (int i = 0; i < info.self.bag.size(); ++i)
			{
				if (info.self.bag.at(i).type == BONDAGE)
				{
					GetBondage = true;
				}
				if (info.self.bag.at(i).type == FIRST_AID_CASE)
				{
					GetFirst_Aid_Case = true;
				}
				if (GetBondage == true && GetFirst_Aid_Case == true)
				{
					break;
				}
			}
			if (info.self.hp < 20)
			{
				if (GetFirst_Aid_Case == true)
				{
					shoot(FIRST_AID_CASE, 0, info.player_ID);
				}
				if (GetBondage == true)
				{
					shoot(BONDAGE, 0, info.player_ID);
				}
			}
			else if (info.self.hp < 60)
			{
				if (GetBondage == true)
				{
					shoot(BONDAGE, 0, info.player_ID);
				}
			}
		}
		if (info.items.empty())
		{
			if (info.self.status != MOVING)
			{
				//判断有无危险声音
				bool SafeSound = true;
				for (int i = 0; i < info.sounds.size(); ++i)
				{
					if (info.sounds.at(i).type == 2 || info.sounds.at(i).type == 3)
					{
						SafeSound = false;
						break;
					}
				}
				//无危险声音
					//跑毒
					if (frame >= 200)
					{
						move(AccurateAngle((XYPostionToMyPolarPosition(info.poison.next_center.x, info.poison.next_center.y).angle - info.self.view_angle)), XYPostionToMyPolarPosition(info.poison.next_center.x, info.poison.next_center.y).angle,0);
					}
					else
					{
						move(AccurateAngle((XYPostionToMyPolarPosition(500, 500).angle - info.self.view_angle)), XYPostionToMyPolarPosition(500, 500).angle,0);
					}
					try_update_info();
					//绕开前方障碍
					PolarPosition ClosestBarrier,Closest_;//前方障碍物、中间量
					ClosestBarrier.distance = 100000;
					for (int i = 0; i < info.landform_IDs.size(); ++i)
					{
						if (get_landform(info.landform_IDs[i]).type != SHALLOW_WATER)
						{
							//不可通行
							if (get_landform(info.landform_IDs[i]).type != RECTANGLE_GRASS && get_landform(info.landform_IDs[i]).type != CIRCLE_GRASS)
							{
								if (get_landform(info.landform_IDs[i]).shape == RECTANGLE)
								{
									Closest_.distance = fmin(fmin(XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x0, (double)get_landform(info.landform_IDs[i]).y0).distance, XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x1, (double)get_landform(info.landform_IDs[i]).y0).distance), fmin(XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x0, (double)get_landform(info.landform_IDs[i]).y1).distance, XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x1, (double)get_landform(info.landform_IDs[i]).y1).distance));
									if (Closest_.distance < ClosestBarrier.distance)
									{
										ClosestBarrier.distance = Closest_.distance;
										ClosestBarrier.angle =  XYPostionToMyPolarPosition(((double)get_landform(info.landform_IDs[i]).x0 + (double)get_landform(info.landform_IDs[i]).x1) / 2.0, ((double)get_landform(info.landform_IDs[i]).y0 + (double)get_landform(info.landform_IDs[i]).y1) / 2.0).angle;
									}
								}
								else
								{
									Closest_.distance = XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x0, (double)get_landform(info.landform_IDs[i]).y0).distance;
									if (Closest_.distance < ClosestBarrier.distance + get_landform(info.landform_IDs[i]).r)
									{
										ClosestBarrier.distance = Closest_.distance;
										ClosestBarrier.angle = XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x0, (double)get_landform(info.landform_IDs[i]).y0).angle;
									}
								}
							}
						}
					}
					//移动
					if (ClosestBarrier.distance < 2 && abs(ClosestBarrier.angle - info.self.view_angle) < 10)
					{
						move(AccurateAngle((90 + ClosestBarrier.angle-info.self.view_angle)), AccurateAngle((90 + ClosestBarrier.angle)));
					}
					else 
					{
						move(0,info.self.view_angle);
					}
				
			}
		}
		//看见东西
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
			std::cout << "**closest item angle" << closest_item.polar_pos.angle << "distance" << closest_item.polar_pos.distance << "**" << std::endl;
			if (closest_item.polar_pos.distance < 1)
			{
			pickup(closest_item.item_ID);
			std::cout << "try pickup" << closest_item.item_ID << std::endl;
			}
			else if (info.self.status != MOVING)
			{
			move(closest_item.polar_pos.angle, AccurateAngle(closest_item.polar_pos.angle + info.self.view_angle));
			std::cout << "move" << closest_item.polar_pos.angle << std::endl;
			}
		}
	}
	//看见人
	else
	{
		int has_enemy = 0;
		OtherInfo closest_enemy;
		closest_enemy.polar_pos.distance = 100000;
		//检查队友
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
				has_enemy++;
			}
		}
		//有危险
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
			//攻击
			if (weapon != FIST && has_enemy == 1 && info.self.hp >= 30)
			{
				if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
				{
					move(closest_enemy.polar_pos.angle, AccurateAngle(closest_enemy.polar_pos.angle + info.self.view_angle));
				}
				else if (info.self.attack_cd == 0)
				{
					shoot(weapon, AccurateAngle(atan2(closest_enemy.polar_pos.distance * sin(closest_enemy.polar_pos.angle / 180 * Pi) + closest_enemy.move_speed * sin(closest_enemy.move_angle / 180 * Pi), closest_enemy.polar_pos.distance * cos(closest_enemy.polar_pos.angle / 180 * Pi) + closest_enemy.move_speed * cos(closest_enemy.move_angle / 180 * Pi)) / Pi + 1) * 360);
				}
			}
			//逃跑
			else if (info.self.status != MOVING)
			{
				move(AccurateAngle((closest_enemy.polar_pos.angle+540-info.self.move_angle)), AccurateAngle((closest_enemy.polar_pos.angle + 180)),0);
				try_update_info();
				PolarPosition ClosestPath,Closest_;
				ClosestPath.distance = 100000;
				bool GoToGrass = false;
				for (int i = 0; i < info.landform_IDs.size(); ++i)
				{
					if (get_landform(info.landform_IDs[i]).type == RECTANGLE_GRASS && get_landform(info.landform_IDs[i]).type == CIRCLE_GRASS)
					{
						if (get_landform(info.landform_IDs[i]).shape == RECTANGLE)
						{
							Closest_.distance = XYPostionToMyPolarPosition(((double)get_landform(info.landform_IDs[i]).x0 + (double)get_landform(info.landform_IDs[i]).x1) / 2.0, ((double)get_landform(info.landform_IDs[i]).y0 + (double)get_landform(info.landform_IDs[i]).y1) / 2.0).distance;
							if (Closest_.distance < ClosestPath.distance)
							{
								ClosestPath.distance = Closest_.distance;
								ClosestPath.angle = XYPostionToMyPolarPosition(((double)get_landform(info.landform_IDs[i]).x0 + (double)get_landform(info.landform_IDs[i]).x1) / 2.0, ((double)get_landform(info.landform_IDs[i]).y0 + (double)get_landform(info.landform_IDs[i]).y1) / 2.0).angle;
							}
						}
						else
						{
							Closest_.distance = XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x0, (double)get_landform(info.landform_IDs[i]).y0).distance;
							if (Closest_.distance < ClosestPath.distance)
							{
								ClosestPath.distance = Closest_.distance;
								ClosestPath.angle = XYPostionToMyPolarPosition((double)get_landform(info.landform_IDs[i]).x0, (double)get_landform(info.landform_IDs[i]).y0).angle;
							}
						}
					}
				}
				if (ClosestPath.distance < 10)
				{
					GoToGrass = true;
				}
				//躲草丛
				if (GoToGrass == true)
				{
					move(AccurateAngle((ClosestPath.angle - info.self.view_angle)),ClosestPath.angle);
				}
				else
				{
					move(160 + rand() % 40, AccurateAngle((160 + rand() % 40 + info.self.view_angle)));
				}
			}
		}
	}
	//输出状态
	FILE *fp = stdout;
	fprintf(fp,"第%d帧\n",frame);
	fprintf(fp,"玩家ID：%d\n",info.player_ID);
	if (XYPostionToMyPolarPosition(info.poison.current_center.x,info.poison.current_center.y).distance < info.poison.current_radius)
	{
		fprintf(fp, "毒圈里！(%.2f,%.2f)半径：%.2f\n",info.poison.current_center.x,info.poison.current_center.y,info.poison.current_radius);
	}
	else
	{
		fprintf(fp,"毒圈外！(%.2f,%.2f)半径：%.2f\n", info.poison.current_center.x, info.poison.current_center.y,info.poison.current_radius);
	}
	fprintf(fp, "状态：%d\n", info.self.status);
	fprintf(fp,"移动方向：%.2f\n",info.self.move_angle);
	fprintf(fp,"视线方向：%.2f\n",info.self.view_angle);
	fprintf(fp,"血量:%.2f/%.2f\n",info.self.hp,info.self.hp_limit);
	fprintf(fp,"坐标：(%f,%f)\n", info.self.xy_pos.x,info.self.xy_pos.y);
	fprintf(fp,"身上物品:\n");
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		fprintf(fp,"{%d,%d}\n",info.items[i].type,info.items[i].durability);
	}
	return;
}