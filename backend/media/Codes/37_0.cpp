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
int s = 0;
int war = 1; int med = 0;
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
		
		
			double xm = info.self.xy_pos.x;
			double ym = info.self.xy_pos.y;
			move(fabs(move_angle1), fabs(view_angle));
			if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
			{
				std::cout << "move" << move_angle << std::endl;
				
			}
			else
				move(90, 90);
		
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
		
			double xm = info.self.xy_pos.x;
			double ym = info.self.xy_pos.y;
			move(fabs(move_angle1), fabs(view_angle));
			if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
			{
				std::cout << "move" << move_angle << std::endl;
				
			}
			else
				move(90, 90);
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
	if (info.self.bag.size() > 1)
		std::cout << "pick succeed" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 6450 + rand() % 100, 255+ rand() % 100 };
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
	war = 1;
	try_update_info();
	for ( int i=0; i < info.sounds.size(); ++i)
	{
		Sound Wireless = info.sounds[i];
		if (Wireless.type == 0)
		{
			
			
				int x = Wireless.parameter;
				if(x%10==1)
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
					
					std::cout << "move" << move_angle << std::endl;
				}
				if (x % 10 == 2)
				{
					war = 0;
					int x2 = x / 10000; int y2 = (x / 10) % 1000;
					double move_angle = atan(fabs(y2 - info.self.xy_pos.y) / fabs(x2 - info.self.xy_pos.y));
					if (move_angle < 90)
					{
						move_angle = move_angle + 360 - 90;
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
					
					std::cout << "move" << move_angle << std::endl;
				}
				if (x % 10 == 3)
				{
					war = 1;
					int x2 = x / 10000; int y2 = (x / 10) % 1000;
					double move_angle = atan((y2 - info.self.xy_pos.y) / (x2 - info.self.xy_pos.y));
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
					
					std::cout << "move" << move_angle << std::endl;
				}
				if (x % 10 == 4)
				{
					int x2 = x / 10000; int y2 = (x / 10) % 1000;
					double move_angle = atan(fabs(y2 - info.self.xy_pos.y) / fabs(x2 - info.self.xy_pos.y));
					double view_angle = move_angle;
					med = 1;
					if (med == 1)
					{
						ITEM medi = FIRST_AID_CASE;
						for (unsigned int i = 0; i < info.self.bag.size(); ++i)
						{
							int j = 16;
							for (j = 16; j >= 15; j--)
							{
								if (ITEM_DATA[info.self.bag[i].type].type == GOODS && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
								{
									medi = info.self.bag[i].type;
									break;
								}
							}
						}
						if (sqrt(x2*x2 + y2 * y2) > PICKUP_DISTANCE)
						{
							
								double xm = info.self.xy_pos.x;
								double ym = info.self.xy_pos.y;
								move(fabs(move_angle), fabs(move_angle));
								if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
								{
									std::cout << "move" << move_angle << std::endl;
									
								}
								else
									move(90, 90);
							
						}
						else
						{
							shoot(medi, move_angle, Wireless.sender);
							med = 0;
						}
					}
					
					
				}
			
			
		}
	

	}

	OtherInfo closest_friend;
	closest_friend.polar_pos.distance = 100000;
	for (unsigned int i = 0; i < info.others.size(); ++i)
	{
		bool is_friend = false;
		for (unsigned int teammate = 0; teammate < teammates.size(); ++teammate)
		{
			if (info.others[i].player_ID == teammates[teammate] && info.others[i].status == 7)
			{
				med = 1;
				closest_friend = info.others[i];


				break;
			}
		}

	}
	if (med == 1)
	{
		if (closest_friend.status == 8)
		{
			med = 0;
		}
		if (med == 1)
		{
			ITEM medi = FIRST_AID_CASE;
			for (unsigned int i = 0; i < info.self.bag.size(); ++i)
			{
				int j = 16;
				for (j = 16; j >= 15; j--)
				{
					if (ITEM_DATA[info.self.bag[i].type].type == GOODS && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
					{
						medi = info.self.bag[i].type;
						break;
					}
				}
			}
			if (closest_friend.polar_pos.distance > PICKUP_DISTANCE)
			{
				
					double xm = info.self.xy_pos.x;
					double ym = info.self.xy_pos.y;
					move(closest_friend.polar_pos.angle, closest_friend.polar_pos.angle);
					if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
					{
						std::cout << "move" << closest_friend.polar_pos.angle << std::endl;
						
					}
					else
						move(90, 90);
				
			}
			else
			{
				shoot(medi, closest_friend.polar_pos.angle, closest_friend.player_ID);
				med = 0;
			}
		}
	}
	if (med == 0)
	{
		if (info.others.empty())
		{
			std::cout << "no others" << std::endl;
			if (info.items.empty())
			{
				//see nothing
				if (info.self.status != MOVING)
				{
					
					
						double xm = info.self.xy_pos.x;
						double ym = info.self.xy_pos.y;
						move(info.self.move_angle - atan((info.self.xy_pos.y - info.poison.current_center.y) / (info.self.xy_pos.x - info.poison.current_center.x)),
							info.self.move_angle - atan((info.self.xy_pos.y - info.poison.current_center.y) / (info.self.xy_pos.x - info.poison.current_center.x)));
						if (info.self.xy_pos.x != xm || info.self.xy_pos.y != ym)
						{
							std::cout << "move" << info.self.move_angle - atan((info.self.xy_pos.y - info.poison.current_center.y) / (info.self.xy_pos.x - info.poison.current_center.x)) << std::endl;
							
						}
						else
							move(90, 90);
					

				}
			}
			else
			{
				Item best_item;
				Item *Found_item = new Item[info.items.size()];
				best_item.polar_pos.distance = 100;
				int i;
				bool Get_Item;
				for (i = 0; i < info.items.size(); ++i)
					for (int k = 0; k < i; ++k)
						if (info.items[i].polar_pos.distance < best_item.polar_pos.distance)
							Found_item[k] = info.items[i];
				for (int k = 0; k < i; ++k)
				{
					if (Found_item[k].type == 2)
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
						if (Found_item[k].type == 0)
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
				move(best_item.polar_pos.angle, best_item.polar_pos.angle);
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
				if (war = 1)
				{
					ITEM weapon = FIST;
					if (closest_enemy.polar_pos.distance >= 150)
					{
						for (int i = 0; i < info.self.bag.size(); ++i)
						{
							int j = 6;
							for (j = 7; j >= 6; j--)
							{
								if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
								{
									weapon = info.self.bag[i].type;
									break;
								}
							}
						}
						if (weapon == FIST)
						{
							for (int i = 0; i < info.self.bag.size(); ++i)
							{
								int j = 2;
								for (j = 2; j >= 1; j--)
								{
									if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
									{
										weapon = info.self.bag[i].type;
										break;
									}
								}
								j = 9;
								if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
								{
									weapon = info.self.bag[i].type;
									break;
								}
								for (j = 5; j >= 3; j--)
								{
									if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
									{
										weapon = info.self.bag[i].type;
										break;
									}
								}
							}
						}
						if (weapon == FIST)
						{
							for (int i = 0; i < info.self.bag.size(); ++i)
							{
								int j = 8;
								if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
								{
									weapon = info.self.bag[i].type;
								}
							}
						}
					}
					else
					{
						if (closest_enemy.polar_pos.distance > 10)
						{
							for (int i = 0; i < info.self.bag.size(); ++i)
							{
								int j = 2;
								for (j = 2; j >= 1; j--)
								{
									if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
									{
										weapon = info.self.bag[i].type;
										break;
									}
								}
								j = 9;
								if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
								{
									weapon = info.self.bag[i].type;
									break;
								}
								for (j = 5; j >= 3; j--)
								{
									if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && ITEM_DATA[info.self.bag[i].type].number == j)
									{
										weapon = info.self.bag[i].type;
										break;
									}
								}
							}

						}
					}
					if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
					{
						Move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);

					}
					else
					{
						shoot(weapon, closest_enemy.polar_pos.angle);
					}
				}
				else
				{
					if (closest_enemy.polar_pos.angle >= 180)
					{
						Move(closest_enemy.polar_pos.angle - 180, closest_enemy.polar_pos.angle);
					}
					else
					{
						Move(180 + closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
					}
				}
			}
		}
	}
	return;
}

