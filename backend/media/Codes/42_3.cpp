#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <cmath>
#include<vector>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

//user vars
const double PI = 3.1415926535897932384626;
const double ROOM = 1;//????
XYPosition Des, Midway;
int sec_pointer = 0;
XYPosition sec_pos[6];
//Des: long-term goal
//Midway: short-term goal
const char* block_name[9] = { "DEEP_WATER","RECTANGLE_BUILDING","SHALLOW_WATER","CIRCLE_BUILDING","WALL","TREE","RECTANGLE_GRASS","CIRCLE_GRASS","BLOCK_TYPE_SZ" };
const char* item_name[21] = { "FIST","HAND_GUN","SUBMACHINE_GUN","SEMI_AUTOMATIC_RILE","ASSAULT_RIFLE","MACHINE_GUN","SNIPER_RILFE","SNIPER_BARRETT","TIGER_BILLOW_HAMMER","CROSSBOW","VEST_1","VEST_2","VEST_3","INSULATED_CLOTHING","MUFFLER","BONDAGE","FIRST_AID_CASE","CODE_CASE","SCOPE_2","SCOPE_4","SCOPE_8" };
//----------------------------------------------------------------------------------

//user_functions
void save_info(std::ofstream&);//???????
double cal_angle(XYPosition p1, XYPosition p2);//????????????
double cal_distance(XYPosition p1, XYPosition p2);//?????????????
void CheesyWalk();//???
void DodgeBarrier();//????
block Oceans[10]{
{RECTANGLE,DEEP_WATER, 0,   300,  0, 100,  0   },
{RECTANGLE,DEEP_WATER, 0,   900,  0, 100,  800 },
{RECTANGLE,DEEP_WATER, 0,   1000, 0, 200,  900 },
{RECTANGLE,DEEP_WATER, 400, 100,  0, 600,  0   },
{RECTANGLE,DEEP_WATER, 400, 1000, 0, 700,  900 },
{RECTANGLE,DEEP_WATER, 500, 900,  0, 600,  800 },
{RECTANGLE,DEEP_WATER, 800, 100,  0, 1000, 0   },
{RECTANGLE,DEEP_WATER, 900, 200,  0, 1000, 100 },
{RECTANGLE,DEEP_WATER, 900, 600,  0, 1000, 300 },
{RECTANGLE,DEEP_WATER, 900, 1000, 0, 1000, 800 },
};//Important constants, contains coordinates of ocean.

								 //----------------------------------------------------------------------------------

void play_game()
{
	//????????,???????
	/*if (frame == 0)
	{
		std::ofstream fout("AI_4_0.txt", std::ios_base::trunc);
		fout.close();
	}
	std::ofstream fout("AI_4_0.txt", std::ios_base::out | std::ios_base::app);
	save_info(fout);*/
	update_info();
	//----------------------------------------------------------------------------------
	//??
	if (frame == 0)
	{
		Des = Midway = info.poison.current_center;
		XYPosition landing_point = { 320, 430 };
		parachute(SIGNALMAN, landing_point);
		//srand(time(nullptr) + teammates[0]);
		//fout << "-------------------------------------------------" << std::endl;
		return;
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		/*fout << "status:jumping" << std::endl;
		fout << "-------------------------------------------------" << std::endl;*/
		return;
	}
	if ((Des.x != info.poison.current_center.x) || (Des.y != info.poison.current_center.y))Midway = Des = info.poison.current_center;
	//from demo
	if (info.others.empty())
	{
		if (info.items.empty())
		{
			//see nothing
			if ((info.self.xy_pos.x == sec_pos[sec_pointer].x) && (info.self.xy_pos.y == sec_pos[sec_pointer].y))
			{
				move(180, 180);
				Midway = Des;
			}
			else
			{
				sec_pos[sec_pointer] = info.self.xy_pos;
				sec_pointer++;
				sec_pointer %= 6;
				CheesyWalk();
			}
		}
		else
		{
			Item closest_item;
			closest_item.polar_pos.distance = 100000;
			for (int i = 0; i < info.items.size(); ++i)
			{
				if ((info.items[i].polar_pos.distance < closest_item.polar_pos.distance)&&(info.items[i].type!=CODE_CASE))
				{
					closest_item = info.items[i];
				}
			}
			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);
			}
			else if (info.self.status != MOVING)
			{
				move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
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
			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			}
			else
			{
				if(info.self.attack_cd==0)shoot(weapon, closest_enemy.polar_pos.angle);
			}
		}
	}
	//--------------------------------------------------------------------------------
	//fout << "-------------------------------------------------" << std::endl;
	return;
}


double cal_angle(XYPosition p1, XYPosition p2)//????????????
{
	double x = p2.x - p1.x, y = p2.y - p1.y;
	double angle = atan2(-x, y);//???X????
	angle = angle / PI * 180.0 + 90.0;
	angle -= info.self.view_angle;
	while (angle > 360.0) angle -= 360.0;
	while (angle < 0) angle += 360.0;
	return angle;
}
//?????????????

double cal_distance(XYPosition p1, XYPosition p2)//?????????????
{
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

void save_info(std::ofstream& fout)
{
	fout << "current:frame" << frame << "\nplayer_hp:" << info.self.hp << std::endl;
	fout << "positon(" << info.self.xy_pos.x << ',' << info.self.xy_pos.y << ")" << std::endl;
	fout << "midway:(" << Midway.x << "," << Midway.y << ")" << std::endl;
	int size = info.self.bag.size();
	fout << "bag:" << size << std::endl;
	for (int i = 0; i < size; i++)
	{
		fout << item_name[info.self.bag[i].type] << std::endl;
	}
	//size = info.items.size();
	//fout << "on_ground_item:" << size << std::endl;
	//for (int i = 0; i < size; i++)
	//{
	//	fout << info.items[i].type << std::endl;
	//}
	size = info.landform_IDs.size();
	fout << "vision:" << size << std::endl;
	for (int i = 0; i < size; i++)
	{
		fout << block_name[get_landform(info.landform_IDs[i]).type];
		fout << ":" //??(??)
			<< cal_distance(info.self.xy_pos, { double(get_landform(info.landform_IDs[i]).x0), double(get_landform(info.landform_IDs[i]).y0) })
			<< ":"//??(??)
			<< cal_angle(info.self.xy_pos, { double(get_landform(info.landform_IDs[i]).x0), double(get_landform(info.landform_IDs[i]).y0) }) - info.self.view_angle
			<< std::endl;
	}
	fout << "now heading:" << info.self.move_angle << std::endl;
	fout << "now looking:" << info.self.view_angle << std::endl;
	fout << "move_cd:" << info.self.move_cd << std::endl;
	fout << "attack_cd" << info.self.attack_cd << std::endl;
	fout << "current circle center:" << info.poison.current_center.x << "," << info.poison.current_center.y << std::endl;
}

void DodgeBarrier() {
	//To use this function, make sure you're looking where you're going.
	//Midway: important point. Always head for Midway.
	XYPosition MePos = info.self.xy_pos, TEMP[25];
	int t = 0, Ans = -1;
	double Dist = cal_distance(MePos, Midway);
	if (Dist <= 1) { Midway = Des; return; }//Arrived at Midpoint, head for Destination now.
	block barrier;

	//-----Oceans------

	for (int i = 0; i < 10; i++) {
		barrier = Oceans[i];
		XYPosition P[5];//Points
		double A[5];//Angle
		double LeftAngle = -1, RightAngle = -1;
		P[1].x = barrier.x0; P[1].y = barrier.y0; A[1] = cal_angle(MePos, P[1]);
		if (A[1] < 180) LeftAngle = std::max(LeftAngle, A[1]); else RightAngle = std::max(RightAngle, 360 - A[1]);
		P[2].x = barrier.x0; P[2].y = barrier.y1; A[2] = cal_angle(MePos, P[2]);
		if (A[2] < 180) LeftAngle = std::max(LeftAngle, A[2]); else RightAngle = std::max(RightAngle, 360 - A[2]);
		P[3].x = barrier.x1; P[3].y = barrier.y1; A[3] = cal_angle(MePos, P[3]);
		if (A[3] < 180) LeftAngle = std::max(LeftAngle, A[3]); else RightAngle = std::max(RightAngle, 360 - A[3]);
		P[4].x = barrier.x1; P[4].y = barrier.y0; A[4] = cal_angle(MePos, P[4]);
		if (A[4] < 180) LeftAngle = std::max(LeftAngle, A[4]); else RightAngle = std::max(RightAngle, 360 - A[4]);
		//fout << LeftAngle << " " << RightAngle << std::endl;
		if (LeftAngle > 0 && RightAngle > 0) {
			double MinAngle;
			MinAngle = std::min(LeftAngle, RightAngle);
			for (int i = 1; i <= 4; i++) if (MinAngle == A[i] || 360 - A[i] == MinAngle) {
				int j = (i + 2) % 4;
				if (j == 0) j = 4;
				if (P[j].x > P[i].x) TEMP[++t].x = P[i].x - ROOM;
				else TEMP[++t].x = P[i].x + ROOM;
				if (P[j].y > P[i].y) TEMP[t].y = P[i].y - ROOM;
				else TEMP[t].y = P[i].y + ROOM;
				break;
			}
			if (cal_distance(MePos, TEMP[t]) > Dist) t--;
			//else fout << "recgnize ocean!" << std::endl;
		}
	}

	//-----Optical Barriers-----
	for (int i = 0; i < info.landform_IDs.size(); i++) {
		barrier = get_landform(info.landform_IDs[i]);
		//------Buildings & Walls-----------
		if (barrier.type == RECTANGLE_BUILDING || barrier.type == WALL) {
			XYPosition P[5];//Points
			double A[5];//Angle
			double LeftAngle = -1, RightAngle = -1;
			P[1].x = barrier.x0; P[1].y = barrier.y0; A[1] = cal_angle(MePos, P[1]);
			if (A[1] < 180) LeftAngle = std::max(LeftAngle, A[1]); else RightAngle = std::max(RightAngle, 360 - A[1]);
			P[2].x = barrier.x0; P[2].y = barrier.y1; A[2] = cal_angle(MePos, P[2]);
			if (A[2] < 180) LeftAngle = std::max(LeftAngle, A[2]); else RightAngle = std::max(RightAngle, 360 - A[2]);
			P[3].x = barrier.x1; P[3].y = barrier.y1; A[3] = cal_angle(MePos, P[3]);
			if (A[3] < 180) LeftAngle = std::max(LeftAngle, A[3]); else RightAngle = std::max(RightAngle, 360 - A[3]);
			P[4].x = barrier.x1; P[4].y = barrier.y0; A[4] = cal_angle(MePos, P[4]);
			if (A[4] < 180) LeftAngle = std::max(LeftAngle, A[4]); else RightAngle = std::max(RightAngle, 360 - A[4]);
			//fout << LeftAngle << " " << RightAngle << std::endl;
			if (LeftAngle > 0 && RightAngle > 0) {
				//Only then is it possible for the barrier to block my path.
				double MinAngle;
				MinAngle = std::min(LeftAngle, RightAngle);
				for (int i = 1; i <= 4; i++) if (MinAngle == A[i] || 360 - A[i] == MinAngle) {
					int j = (i + 2) % 4; //finding the opposite vertex.
					if (j == 0) j = 4;
					if (P[j].x > P[i].x) TEMP[++t].x = P[i].x - ROOM;
					else TEMP[++t].x = P[i].x + ROOM;
					if (P[j].y > P[i].y) TEMP[t].y = P[i].y - ROOM;
					else TEMP[t].y = P[i].y + ROOM;
					break;
				}
				if (cal_distance(MePos, TEMP[t]) > Dist) t--;//Too faraway to block my path.
			}
		}

		//------Circular Barriers(Trees included)------------
		else if (barrier.type == CIRCLE_BUILDING || barrier.type == TREE) {
			XYPosition Center;
			Center.x = barrier.x0; Center.y = barrier.y0;
			double CDist = cal_distance(MePos, Center);
			if (CDist > Dist) continue; //Too faraway
			if(barrier.type == CIRCLE_BUILDING ) barrier.r += ROOM+2.5;
			else barrier.r += ROOM;//Making it rounder. Thus the person is considered a point.
			double Angle = cal_angle(MePos, Center);
			Angle /= 180; Angle *= PI;
			double LDist = CDist * fabs(sin(Angle));//Distance from circle center to my path.
			if (LDist < barrier.r) {
				double ODist = CDist * cos(Angle);//Distance to vertical point.
				double X = MePos.x + (Midway.x - MePos.x)*ODist / Dist;
				double Y = MePos.y + (Midway.y - MePos.y)*ODist / Dist;//Coordinates of vertical point.
				TEMP[++t].x = Center.x + (X - Center.x)*barrier.r / LDist;
				TEMP[t].y = Center.y + (Y - Center.y)*barrier.r / LDist;
			}
		}
	}
	if (t == 0) return;
	double MinDist = 100000;
	for (int i = t; i > 0; i--) {
		if (cal_distance(MePos, TEMP[i]) < MinDist) { Ans = i; MinDist = cal_distance(MePos, TEMP[i]); }
	}
	Midway = TEMP[Ans];
}

void CheesyWalk() {

	DodgeBarrier();
	//PickStuff,KillEnemy,HelpFriend,
	XYPosition MePos = info.self.xy_pos;
	double Angle = cal_angle(MePos, Midway);
	if (info.self.move_cd <= 1) move(Angle, Angle);//??????????????

}
