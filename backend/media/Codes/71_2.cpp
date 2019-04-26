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

int k=0;

void play_game()
{
	/* Your code in this function */
	/* sample AI */

	void moveto(XYPosition aim);
	void medic();
	//////////////////////////////////////////////////此角色为医疗兵
	update_info();
	std::cout << "玩家的帧: " << frame << "\nHP: " << info.self.hp << std::endl;
	std::cout << "定位坐标: " <<"X: "<< info.self.xy_pos.x << "  Y: " << info.self.xy_pos.y << std::endl;
	if (info.self.bag.size() > 1)
		std::cout << "拾取成功" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = {50/* 150 + rand() % 100*/, 50/*150 + rand() % 100*/ }; //开局选个好地方？
		parachute(MEDIC, landing_point);  //角色：医疗兵
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		std::cout << "在飞机上/跳伞中……" << std::endl;
		return;
	}
	if (info.others.empty())
	{
		std::cout << "未发现其他人" << std::endl;
		if (k < 4)
		{
			radio(teammates[k], info.player_ID);
			k++;
		}
		if (info.items.empty())
		{
			//see nothing
			if (info.self.status != MOVING)
			{
				double move_angle = 0;
				double view_angle = move_angle;
				move(move_angle, view_angle);
				std::cout << "移动" << move_angle << std::endl;
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
				std::cout << "尝试拾取" << closest_item.item_ID << std::endl;
			}
			else if (info.self.status != MOVING)
			{
				move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
				std::cout << "移动" << closest_item.polar_pos.angle << std::endl;
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

					//////////// ////////////// ////////////// ////////////// ////////////// ////////////// ////////////// //////////////此处开始打药救人算法?
					medic();

					break;
				}
			}
			if (!is_friend && info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
			{
				closest_enemy = info.others[i];
				has_enemy = true;
			}
		}
		if (has_enemy)  //发现敌人，进行攻击？
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
			//换用选择武器算法
			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			}
			else
			{
				shoot(weapon, closest_enemy.polar_pos.angle); //考虑射击偏移？
			}
		}
	}
	return;
}

void moveto(XYPosition aim)
{
	double dx = aim.x - info.self.xy_pos.x;
	double dy = aim.y - info.self.xy_pos.y;
	double dis = sqrt(dx * dx + dy * dy);

	double aim_angle = atan2(dy, dx) * 180.0 / 3.14159265358979;
	if (aim_angle < 0)
		  aim_angle = 360 + aim_angle;

	double set_angle = aim_angle - info.self.view_angle;
	if (set_angle < 0)
		  set_angle = 360 + set_angle;

	//std::cout << aim_angle << " " << dis << " " << dx << "." << dy << " " << aim.x <<"."<< aim.y << std::endl;
	if (dis < 1)
	{
		move(set_angle, set_angle, NOMOVE);
	}
	else
	{
		move(set_angle, set_angle);
	}
}


void medic()
{
	struct received_info
	{
		int hp_sort;
		XYPosition loc_sort;
		VOCATION voc_sort;
	};
	//接收无线电信息，将hp值存在另外三个变量中
	int i = 0, j=0;
	int32_t temp[3];
	int sender_ID[3];

	for (; j < info.sounds.size(); ++j)
	{
		if (info.sounds[j].type == RADIO_VOICE) //条件：筛选出无线电信号。无线电信号只可能来自队友。
		{
			temp[i] = info.sounds[j].parameter;
			sender_ID[i] = info.sounds[j].sender;
			i++;
		}
	}

	received_info medic_teammates[3];

	for (i = 0; i < 3; i++)
	{
		 int32_t mid=temp[i];
		 medic_teammates[i].loc_sort.y = mid % 1000;
		 mid = (mid - medic_teammates[i].loc_sort.y)/1000;
		 medic_teammates[i].loc_sort.x = mid % 1000;
		 mid = (mid - medic_teammates[i].loc_sort.x)/1000;
		 medic_teammates[i].voc_sort = (VOCATION)(mid % 10);
		 mid = (mid - (mid % 10) )/ 10;
		 medic_teammates[i].hp_sort = (int)mid;
	}
	//需要换算成hp、地点以及职业？考虑将hp分段，每段对应不同的数字？
	//考虑增加translate功能以完成此任务？

	//将另外三个角色按照hp从低到高排序
	int mid_hp_var;
	XYPosition mid_loc_var;
	VOCATION mid_voc_var;
	received_info mid;
	int sender_mid;

	for (j=0; j<2; j++)                                 //由hp从低到高排序，按照结构体数组medic_teammates[3]排序
		for (i = 0; i < 2 - j; i++)
		{
			if (medic_teammates[i].hp_sort > medic_teammates[i+1].hp_sort)
			{
				mid = medic_teammates[i];
				sender_mid=sender_ID[i];
				medic_teammates[i] = medic_teammates[i + 1];
				sender_ID[i]=sender_ID[i+1];
				medic_teammates[i + 1] = mid;
				sender_ID[i + 1] = sender_mid;
			}
		}

   
	XYPosition min_alive_loc;
	VOCATION min_alive_hp_voc;
	int min_alive_hp_ID;
	double med_aid_angle=0;  //使用医疗版本shoot函数时用到的角度。医疗兵最大救治距离为1.0，考虑使用90°的倍数？？？或是说已经给定对象ID，不需要另外指定角度？
	ITEM med_aid;
	if (info.self.hp <= 15 && info.self.hp >=0)
	{
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 16 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, info.player_ID);
	}

	if (info.self.hp>= 15 && info.self.hp <= 70)
	{
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 15 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, info.player_ID);
	}

	if (medic_teammates[0].hp_sort <= 15 && medic_teammates[0].hp_sort >= 0 /*&&距离不是太远？&&不能为真死&&在圈内*/ ) //血量低于15，大于等于0，不能为真死，且距离小于某个值，用救助箱。
	{
		for (i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 16 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, sender_ID[0]);
	}

	if (medic_teammates[0].hp_sort >= 15 && medic_teammates[0].hp_sort <= 70 /*&&距离不是太远？&&在圈内*/) //血量高于15，低于70，且距离小于某个值，用绷带。
	{
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 15 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, sender_ID[0]);
	}
}
