#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#define radius 1.0

struct Point
{
	XYPosition position;
	double landform = 0.0;
	double poison = 0.0;
	double grativity = 0.0;
	double total = 10000.0;
}nearpoints[100];

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

double move_angle(PlayerInfo info, Item closei) {
//返回捡东西的角度（后期返回-1）
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


void output(PlayerInfo info)   //hkz输出
{
	std::cout << "player" << info.player_ID << "position:(" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")" << std::endl;
	//std::cout << "player" << info.player_ID << "moveangle:" << info.self.move_angle << std::endl;
	//std::cout << "player" << info.player_ID << "viewangle:" << info.self.view_angle << std::endl;
	std::cout << "player" << info.player_ID << "hp:" << info.self.hp << std::endl;
	if (!info.player_ID)
	{
		std::cout << "poison_radius" << info.poison.current_radius << std::endl;
		std::cout << "poison_position:(" << info.poison.current_center.x << "," << info.poison.current_center.y << ")" << std::endl;
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

int Nearpoints(void)  //返回#radius#内的点数
{
	int count = 0;
	double x = info.self.xy_pos.x, y = info.self.xy_pos.y;
	for (double i = x - radius; i <= x + radius; i+=0.2)   //radius宏定义为4.0便于调参
		for (double j = y - radius; j <= y + radius; j+=0.2)
			if (((x - i)*(x - i) + (y - j)*(y - j)) <= radius*radius)
			{
				nearpoints[count].position.x = i;             //全局变量nearpoints,数组长度radius*radius防止溢出
				nearpoints[count++].position.y = j;
			}
	return count ;
}

void move2lowest(int n) //走向一定范围内势能最低的点
{
	nearpoints[0].total = nearpoints[0].grativity + nearpoints[0].landform + nearpoints[0].poison;
	Point lowest = nearpoints[0];
	int flag = n;//检查周围势能是否一致
	for (int i = 0; i < n; i++)
	{
		nearpoints[i].total = nearpoints[i].grativity + nearpoints[i].landform + nearpoints[i].poison;
		if (nearpoints[i].total < lowest.total)
			lowest = nearpoints[i];
		if (abs(nearpoints[i].total - lowest.total) > 0.001) flag--;
	}
	double move_angle = 0.0;
	move_angle = atan2((lowest.position.y - info.self.xy_pos.y), (lowest.position.x - info.self.xy_pos.x)) / (2 * 3.141) * 360;
	if (move_angle < 0)
		move_angle += 360;
	if(flag!=n)
		move(move_angle, 0);
}

double poisonpotential(double x,double y) //计算并返回点的毒圈势能
{
	double tmp;
	double dis;
	if (info.poison.move_flag == 3 || info.poison.move_flag == 0)
	{
		return 0;
	}
	if (info.poison.move_flag == 2 )
	{
		tmp = sqrt((info.poison.current_center.x - x)*(info.poison.current_center.x - x) + (info.poison.current_center.y - y)* (info.poison.current_center.y - y));
		dis = tmp - info.poison.current_radius;
		return dis;
	}
	if (info.poison.move_flag == 1)
	{
		tmp = sqrt((info.poison.next_center.x - x)*(info.poison.next_center.x - x) + (info.poison.next_center.y - y)* (info.poison.next_center.y - y));
		dis = tmp - info.poison.next_radius;
		return dis;
	}
	return 0;
}

double landformpotential(XYPosition input, block target)  //地形势能
{
	double dis;
	double tmp;
	if (target.type == 0)//circle
	{
		tmp = sqrt((target.x0 - input.x)*(target.x0 - input.x) + (target.y0 - input.y)* (target.y0 - input.y));
		dis = tmp - target.r;
		return dis;
	}
	if (target.type == 1)//rectangle
	{
		if (input.x < target.x0)//左边
		{
			if (input.y < target.y1)//左下区域
			{
				tmp = sqrt((target.x0 - input.x)*(target.x0 - input.x) + (target.y1 - input.y)* (target.y1 - input.y));
				return tmp;
			}
			if (input.y > target.y0)//左上区域
			{
				tmp = sqrt((target.x0 - input.x)*(target.x0 - input.x) + (target.y0 - input.y)* (target.y0 - input.y));
				return tmp;
			}
			tmp = sqrt((target.x0 - input.x)*(target.x0 - input.x));//左中区域
			return tmp;
		}
		if (input.x > target.x1)//右边
		{
			if (input.y < target.y1)//右下区域
			{
				tmp = sqrt((target.x1 - input.x)*(target.x1 - input.x) + (target.y1 - input.y)* (target.y1 - input.y));
				return tmp;
			}
			if (input.y > target.y0)//右上区域
			{
				tmp = sqrt((target.x1 - input.x)*(target.x1 - input.x) + (target.y0 - input.y)* (target.y0 - input.y));
				return tmp;
			}
			tmp = sqrt((target.x1 - input.x)*(target.x1 - input.x));//右中区域
			return tmp;
		}
		if (input.x >= target.x0&&input.x <= target.x1&&input.y >= target.y1&&input.y <= target.y0)//内部
		{
			return -1;
		}
		return (abs(input.y - target.y0) < abs(input.y - target.y1)) ? abs(input.y - target.y0) : abs(input.y - target.y1);
	}
}

void change_landform(int n)  //修改地形势能
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < info.landform_IDs.size(); j++)
		{
			if (get_landform(info.landform_IDs[j]).type == 0 || get_landform(info.landform_IDs[j]).type == 1 || get_landform(info.landform_IDs[j]).type == 3 || get_landform(info.landform_IDs[j]).type == 5 || get_landform(info.landform_IDs[j]).type == 4)
			{
				if (landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])) > 5)
				{
					nearpoints[i].landform += 500 / (landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])));
				}
				else//上限2000
				{
					nearpoints[i].landform += (1000 / abs(landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])))) < 2000 ? (2000 / abs(landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])))) : 2000;
				}
			}
			if (get_landform(info.landform_IDs[j]).type == 2 && landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])) <= 1)
			{
				nearpoints[i].landform += 50;
			}
			if (get_landform(info.landform_IDs[j]).type == 2 && landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])) > 1)
			{
				nearpoints[i].landform += (50 / landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])));
			}
			if ((get_landform(info.landform_IDs[j]).type == 6 || get_landform(info.landform_IDs[j]).type == 7) && landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])) < 0)
			{
				nearpoints[i].landform += -10;
			}
			if ((get_landform(info.landform_IDs[j]).type == 6 || get_landform(info.landform_IDs[j]).type == 7) && landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j])) >= 0)
			{
				nearpoints[i].landform += landformpotential(nearpoints[i].position, get_landform(info.landform_IDs[j]));
			}
		}
	}
}


/*//物品函数
double pickupfunc(){//更新附近物品的
	double res = 0;
	//如果有不同物品，加不同值
	for (int i = 0; i < info.items.size(); ++i)
	{
		if (info.items[i].polar_pos.distance < closest.polar_pos.distance)
		{
			closest = info.items[i];
		}
	}
	return res;
}*/

//主函数！！！！！！
void play_game(){
	update_info();
	output(info);
	if (frame == 0){
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 650, 650 };//跳在固定点
		parachute(HACK, landing_point);
		return;
	}else{
		srand(time(nullptr) + info.player_ID*frame);
	}


	int total = Nearpoints(); //附近点的总数
	change_landform(total);
	for (int i = 0; i < total; i++)
	{
		nearpoints[i].poison = poisonpotential(nearpoints[i].position.x, nearpoints[i].position.y);
	}
	if(frame >= 500)
		move2lowest(total);

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
	double move_ang = move_angle(info, closest_i);
	move(move_ang,0);

	/*if(move_ang == -1){
		//直线跑毒
		move_ang = move_poison_angle(info)-info.self.view_angle;
		if(move_ang<0)
			move_ang += 360;
		if(info.self.status != MOVING){
			//如果被卡
			move(move_ang, 0);
		}
	}*/
	return;
}
