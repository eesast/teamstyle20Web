#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;



double move_poison_angle(PlayerInfo info);

bool Notincircle(PlayerInfo info,XYPosition centerxy,XYPosition myxy);

ITEM shoot_item(PlayerInfo info) {//返回开枪用的武器
	ITEM weapon = FIST;
	for (int i = 0; i < info.self.bag.size(); ++i)
	{
		if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
		{
			weapon = info.self.bag[i].type;
			break;
		}
	}
	return weapon;
}

double shoot_angle(PlayerInfo info, ITEM weapon, OtherInfo closee) {
//返回打敌人的角度（没有返回-1）
	if (info.others.empty())
	{
		return -1;
	}
	if (closee.polar_pos.distance > ITEM_DATA[weapon].range)
	{
		return -1;
	}
	else
	{
		return closee.polar_pos.angle;
	}
	return -1;
}

double move_angle(PlayerInfo info, double shoot_ang, OtherInfo closee, Item closei) {
//返回捡东西的角度（后期返回-1）
	if(frame >= 480)
		return -1;
	if (closei.polar_pos.distance > 1 && info.self.status != MOVING)
		return closei.polar_pos.angle;
	else{
		return -1;
	}
}


Item closest_item(PlayerInfo info) {//返回最近物品
	Item closest;
	closest.polar_pos.distance = 100000;
	for (int i = 0; i < info.items.size(); ++i)
	{
		if (info.items[i].polar_pos.distance < closest.polar_pos.distance)
		{
			closest = info.items[i];
		}
	}
	return closest;
}

int pick_id(Item closest) {//返回最近物品的id
	if (closest.polar_pos.distance < 1)
	{
		return closest.item_ID;
	}
	else
	{
		return -1;
	}
}

OtherInfo closest_enemy(PlayerInfo info) {//返回最近敌人
	OtherInfo closee;
	closee.player_ID = -1;
	closee.polar_pos.distance = 100000;
	if (!info.others.empty())
	{
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
			if (!is_friend && info.others[i].polar_pos.distance < closee.polar_pos.distance)
			{
				closee = info.others[i];
			}
		}
	}
	return closee;
}

void output(PlayerInfo info)//hkz输出
{
 std::cout << "玩家" << info.player_ID << "坐标：(" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
 std::cout << "玩家" << info.player_ID << "行进方向：" << info.self.move_angle << std::endl;
 std::cout << "玩家" << info.player_ID << "视线方向：" << info.self.view_angle << std::endl;
 std::cout << "玩家" << info.player_ID << "血量:" << info.self.hp << std::endl;
 if (!info.player_ID)
 {
  std::cout << "毒圈位置:(" << info.poison.current_center.x << "," << info.poison.current_center.y << ")\n";
  std::cout << "毒圈半径:" << info.poison.current_radius << std::endl;
 }
}

double move_poison_angle(PlayerInfo info) {//返回跑毒的角度
	XYPosition centerxy;
	if(info.poison.move_flag == 1){
		centerxy = info.poison.next_center;
	}else{
		centerxy = info.poison.current_center;
	}
	XYPosition myxy = info.self.xy_pos;
	double center_angle = 0;
	if(1 || (frame>1000 || Notincircle(info,centerxy,myxy))){
		if((centerxy.x-myxy.x) == 0){
			return 0;
		}else{
			center_angle = atan2((centerxy.y-myxy.y),(centerxy.x-myxy.x))/(2*3.141)*360;
			if(center_angle<0){
				center_angle+=360;
			}
			return center_angle;
		}
	}else{//不动
		return -1;
	}
}

bool Notincircle(PlayerInfo info,XYPosition centerxy,XYPosition myxy){//判断是否在（下一次的）圈内
	double r = info.poison.next_radius;
	if((myxy.x-centerxy.x)*(myxy.x-centerxy.x)+(myxy.y-centerxy.y)*(myxy.y-centerxy.y)>r*r)
		return true;
	else
		return false;
}

//主函数！！！！！！
void play_game(){
	update_info();
	output(info);
	if (frame == 0){
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 650, 250 };//跳在固定点
		parachute(HACK, landing_point);
		return;
	}else{
		srand(time(nullptr) + info.player_ID*frame);
	}

	//判断能否开枪
	ITEM weapon = shoot_item(info);
	OtherInfo closest_e = closest_enemy(info);
	double stangle = shoot_angle(info, weapon, closest_e);
	if (stangle + 1 > 0){
		shoot(weapon, stangle);
	}

	Item closest_i = closest_item(info);

	//判断能否捡东西
	int id = pick_id(closest_i);
	if (id > 0){
		pickup(id);
	}

	//走路，前期捡东西，后期跑毒
	double move_ang = move_angle(info, stangle, closest_e, closest_i);
	if(move_ang == -1){
		//直线跑毒
		move_ang = move_poison_angle(info)-info.self.view_angle;
		if(move_ang<0)
			move_ang += 360;
		if(info.self.status != MOVING){
			//如果被卡
			move(move_ang, 0);
		}
	}
	return;
}
