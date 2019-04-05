#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <string>
#include <math.h>

#define IS_MEDIC


#define MOVING_PRIORITY 0
#define HELPING_PRIORITY 1
#define ATTACKING_PRIORITY 2
#define PICKING_PRIORITY 3

#define GATE 0.0006 //待更正

#define PI 3.14159265358979323846
#define INVPI 0.3183098862 
#define SMALL 0.00001

using namespace std;
const string AI_NAME = "AI_0_0";


#include "api.h"
#include "base.h"

const string FILE_NAME = AI_NAME + ".txt";
const string TRACE_FILE = AI_NAME + "_trace.txt";
const int TEAM_INDEX = AI_NAME[3] - 48;
const int AI_VOCATION = AI_NAME[5] - 48;
const XYPosition landing_point = { 380, 240 };//随机降落到某一个地点。
//static XYPosition position[2];


using namespace ts20;



extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;


///////Global Variables
int AidID;
int movingState = MOVING_PRIORITY;
int HPrecord;
int direct = 1;
//fstream fileOut;
double last_x = 0;
double last_y = 0;
//FILE* fp;
//FILE* fpoison;
PolarPosition targetPosition = { 350,350 };//目标地点的相对极坐标，一般设为毒圈相对位置，特殊情况改为其他位置

static PolarPosition PoisonRelatedPosition;//毒圈相对位置
static PolarPosition PoisonAbsolutePosition;//毒圈绝对位置
Item ideal;
OtherInfo targetEnemy;
OtherInfo patient;





///////Functions
//工具函数：返回距离
inline double getDistance(double x, double y)
{
	return sqrt(x*x + y*y);
}
double Calc_Distance(XYPosition p1, XYPosition p2)
{
	double length = 0;
	length = sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
	return length;
}

//工具函数：返回最小值
inline double min(double a, double b)
{
	return (a < b) ? a : b;
}
//工具函数：返回最大值
inline double max(double a, double b)
{
	return (a > b) ? a : b;
}

//输入x,y绝对坐标，返回相对级坐标,返回的相对角度范围为(-180,180)
PolarPosition getRelatedPolarPosition(XYPosition xy)
{
	PolarPosition relatedPosition;
	double deltaX = xy.x - info.self.xy_pos.x;
	double deltaY = xy.y - info.self.xy_pos.y;
	relatedPosition.distance = getDistance(deltaX, deltaY);
	double temp = atan(deltaY / deltaX) * 180 * INVPI;
	if (deltaX < 0)
		temp += 180;
	relatedPosition.angle = temp - info.self.view_angle;
	while (relatedPosition.angle < -180)
		relatedPosition.angle += 360;
	while (relatedPosition.angle > 180)
		relatedPosition.angle -= 360;
	return relatedPosition;
}


//获得毒圈相对位置，直接改变PosionRelatedPosition的值,返回的相对角度范围为(-180,180)
//这个函数更新PosionRelatedLocation的值，必须在每一帧调用一次后才能正确使用PosionRelatedLocation，
//请勿在一帧之内多次调用，以免影响效率
void getPoisonRelatedLocation()
{
	XYPosition deltaPosionPosition;
	if ((info.poison.current_center.x < SMALL) && (info.poison.current_center.y < SMALL))
	{
		deltaPosionPosition.x = 500 - info.self.xy_pos.x;
		deltaPosionPosition.y = 500 - info.self.xy_pos.y;
	}
	else
	{
		deltaPosionPosition.x = info.poison.current_center.x - info.self.xy_pos.x;
		deltaPosionPosition.y = info.poison.current_center.y - info.self.xy_pos.y;
	}
	PoisonRelatedPosition.distance = getDistance(deltaPosionPosition.x, deltaPosionPosition.y);
	PoisonAbsolutePosition.distance = PoisonRelatedPosition.distance;
	double temp = atan(deltaPosionPosition.y / deltaPosionPosition.x) * 180 * INVPI;
	if (deltaPosionPosition.x < 0)
		temp += 180;
	PoisonAbsolutePosition.angle = temp;
	while (PoisonAbsolutePosition.angle < -180)
		PoisonAbsolutePosition.angle += 360;
	while (PoisonAbsolutePosition.angle > 180)
		PoisonAbsolutePosition.angle -= 360;
	PoisonRelatedPosition.angle = temp - info.self.view_angle;
	while (PoisonRelatedPosition.angle < -180)
		PoisonRelatedPosition.angle += 360;
	while (PoisonRelatedPosition.angle > 180)
		PoisonRelatedPosition.angle -= 360;
}



void AI_move(double moveAngle, double viewAngle)
{
	//f << "\tlast positon\t" << position[0].x << "\t " << position[0].y << std::endl;
	std::cout << "\tmove: " << moveAngle << "\tview: " << viewAngle << std::endl;
	while (moveAngle < 0)
	{
		moveAngle += 360;
	}
	while (viewAngle < 0)
	{
		viewAngle += 360;
	}
	while (moveAngle > 360)
	{
		moveAngle -= 360;
	}
	while (viewAngle > 360)
	{
		viewAngle -= 360;
	}

	move(moveAngle, viewAngle);
	//f << "\tcurrent positon\t" << position[1].x << "\t " << position[1].y << std::endl;
}



//这个checkStuck只向左转
//这个checkStuck的参数是为区别上面的同名函数，只需随便输入一个整数即可
//当它返回的值不为0时，证明还被卡住，此函数会进行一系列移动操作，请立即结束play_game()，以免后面的移动覆盖其操作。
//当返回的值为0时，证明没有被卡住，继续执行其他操作。
int checkStuck(int n)
{
	static int stuckLevel = 0;
	const int MAXSTUCKLEVEL = 7;
	if (abs(info.self.xy_pos.x - last_x) + abs(info.self.xy_pos.y - last_y) < SMALL)
	{
		stuckLevel += 3;
		if (stuckLevel > MAXSTUCKLEVEL)
			stuckLevel = MAXSTUCKLEVEL;
		switch (stuckLevel)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:AI_move(90, 90); break;

		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:AI_move(45, 45); break;
		default:AI_move(30, 30); break;
		}

	}
	else
	{
		stuckLevel --;
		if (stuckLevel <= 0)
			stuckLevel = 0;
		if (stuckLevel > 0)
		{
			AI_move(0, 0);
		}
	}
	return stuckLevel;
}



void play_game()
{
	/* Your code in this function */
	/* sample AI */
	update_info();
	//=========================第0帧或其他特殊帧的时候的处理放在这=====================

	if (frame == 0)//第零帧的时候
	{
		srand(time(nullptr) + teammates[0]);
		parachute((VOCATION)AI_VOCATION, landing_point);
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		std::cout << "\tjumping\t" << std::endl;
		return;
	}

	//=========================其他一般帧的时候的处理放在这=====================



	//这个函数更新PosionRelatedLocation的值，必须在每一帧调用一次后才能正确使用PosionRelatedLocation，
	//请勿在一帧之内多次调用，以免影响效率
	getPoisonRelatedLocation();

	//更新targetPosition.
	targetPosition = PoisonRelatedPosition;

	//=====================打印当前所需信息==========================
	std::cout << "\n-------------------  start  --------------------\n";
	std::cout << "\tplayer:frame\t" << frame << "\n\thp:\t" << info.self.hp << std::endl;//frame是帧，玩家
	std::cout << "\tpositon\t" << info.self.xy_pos.x << "\t " << info.self.xy_pos.y << std::endl;


	//只撞墙

	if (checkStuck(1))
	{
		return;
	}



	if (info.others.empty())
	{
		std::cout << "\tno others\t" << std::endl;

		if (info.items.empty())
		{
			AI_move(targetPosition.angle, targetPosition.angle);
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
			std::cout << "\tstatus\t" << info.self.status << std::endl;

			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);
				std::cout << "\ttry pickup\t" << closest_item.item_ID << std::endl;

			}
			else
			{
				move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
				std::cout << "\tmove\t" << closest_item.polar_pos.angle << std::endl;
				//fileOut << "\tmove\t" << closest_item.polar_pos.angle << std::endl;

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
				if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
			}

			//如果当前武器是拳头的话不打人，跑路
			if (weapon == FIST)
			{
				AI_move(targetPosition.angle, targetPosition.angle);
			}
			else if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				AI_move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			}
			else
			{
				shoot(weapon, closest_enemy.polar_pos.angle);
			}
		}
	}

	std::cout << "\n-------------------  end  --------------------\n";
	last_x = info.self.xy_pos.x;
	last_y = info.self.xy_pos.y;
	return;
}
//The Old Function
