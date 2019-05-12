#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <cmath>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

//Constants
const double PI = 3.141592654;
const double MINPOISON = 50;
const double MAXPOISON = 100;
const double MINOBSTACLE = 1.5;
const double LookAround = 2.5;

#define MEDIC_WEIGHT1 80
#define SNIPER_WEIGHT1 80
#define MEDIC_WEIGHT2 80
#define SNIPER_WEIGHT2 80
#define HPWEIGHT 1
#define DISTANCE_WEIGHT -0.8
#define OCCUR_WEIGHT 80

struct AngleArea
{
	double Start;
	double End;
};

enum DiDi
{
	XIAZOU = 0,
	JIANBAO = 1,
	MUBIAO = 2,
	ZHANDOU = 3
};

struct
{
	DiDi Status;
	XYPosition Target;//(-1,-1):no target 
	bool Paodu;
	XYPosition Former_pos;
	int No_move_frames;
	double former_view_angle;
} NSDDD;

class DiDiAngle
{
public:
	DiDiAngle() { angle = 0; };
	DiDiAngle(double a);
	DiDiAngle(double x, double y);
	DiDiAngle& operator+(DiDiAngle& a2);
	DiDiAngle& operator-(DiDiAngle& a2);
	DiDiAngle& operator=(double a);
	operator double() { return angle; };

private:
	double angle;
};

DiDiAngle::DiDiAngle(double a)
{
	angle = a;
	while (angle < 0) angle += 360;
	while (angle > 360)angle -= 360;
}

DiDiAngle::DiDiAngle(double x, double y)
{
	if (x > info.self.xy_pos.x)
	{
		angle = atan((y - info.self.xy_pos.y) / (x - info.self.xy_pos.x));
		angle *= 180 / PI;
	}
	else
	{
		angle = atan((y - info.self.xy_pos.y) / (x - info.self.xy_pos.x));
		angle *= 180 / PI;
		angle += 180;
	}
	
	while (angle < 0) angle += 360;
	while (angle > 360) angle -= 360;
}

DiDiAngle& DiDiAngle::operator+(DiDiAngle& a2)
{
	this->angle += a2.angle;
	if (this->angle > 360) this->angle -= 360;
	return *this;
}

DiDiAngle& DiDiAngle::operator-(DiDiAngle& a2)
{
	this->angle -= a2.angle;
	if (this->angle < 0) this->angle += 360;
	return *this;
}

DiDiAngle& DiDiAngle::operator=(double a)
{
	this->angle = a;
	while (this->angle < 0) this->angle += 360;
	while (this->angle > 360) this->angle -= 360;
	return *this;
}

//If a point is in a unwalkable rectangle
bool InRect(block b, double x, double y)
{
	if (b.shape == CIRCLE) return false;
	if (b.type == RECTANGLE_GRASS || b.type == SHALLOW_WATER) return false;

	if (x > b.x0
		&& x < b.x1
		&& y > b.y1
		&& y < b.y0)
	{
		return true;
	}
	else
	{
		return false;
	}
	return false;
}

/*Angles unable to move*/
std::vector<AngleArea> ObstacleAngle;

//Shared functions
/*Return a possible hiding place*/
//XYPosition hidden();

void SetTarget(double x=-1, double y=-1)
{
	NSDDD.Target.x = x;
	NSDDD.Target.y = y;
}

void SetTarget(XYPosition xy)
{
	NSDDD.Target = xy;
}

/*exchange_position*/
PolarPosition exchange_xy(XYPosition& pos, PolarPosition& polar)
{
	polar.distance = polar.angle = 0.0;
	polar.distance = sqrt(pow((pos.x - info.self.xy_pos.x), 2) + pow((pos.y - info.self.xy_pos.y), 2));
	polar.angle = atan2((pos.y - info.self.xy_pos.y), (pos.x - info.self.xy_pos.x)) * 180 / PI - info.self.view_angle;
	while(polar.angle < 0) polar.angle += 360;
	while (polar.angle > 360) polar.angle -= 360;
	return polar;
}

XYPosition exchange_polar(PolarPosition & polar, XYPosition & x_y)
{
	x_y.x = x_y.y = 0;
	x_y.x = info.self.xy_pos.x + polar.distance * cos((info.self.view_angle + polar.angle) * PI / 180);
	x_y.y = info.self.xy_pos.y + polar.distance * sin((info.self.view_angle + polar.angle) * PI / 180);
	return x_y;
}

/*shoot & pick_up*/
bool Isteammate(int Player_ID)
{
	for (int i = 0; i < teammates.size(); i++)
		if (Player_ID == teammates[i])
			return true;
	return false;
}

int shotgun(double distance, std::vector<Item>& mybag)
{
	for (int i = 0; i < mybag.size(); i++)
		if (mybag[i].type < 10 && ITEM_DATA[mybag[i].type].range >= distance && mybag[i].durability > 0)
			return i;
	return -1;
}

void Shooting(std::vector<OtherInfo> & other_list, SelfInfo & myself, struct PolarPosition& target)
{
	//modified, moving_shooting added
	if (myself.status == SHOOTING || myself.status == MOVING_SHOOTING) return;
	if (!other_list.size())	return;
	int i, num = 0, temp_weight = 0, max_weight = 200 * DISTANCE_WEIGHT;
	for (i = 0; i < other_list.size(); i++)
	{
		temp_weight = other_list[i].polar_pos.distance * DISTANCE_WEIGHT;
		if (other_list[i].vocation == MEDIC)
			temp_weight += MEDIC_WEIGHT1;
		if (other_list[i].vocation == SNIPER)
			temp_weight += SNIPER_WEIGHT1;
		if (Isteammate(other_list[i].player_ID))
			temp_weight = max_weight - 1;
		if (temp_weight >= max_weight && shotgun(other_list[i].polar_pos.distance, myself.bag) >= 0 && other_list[i].status <= 6)
		{
			max_weight = temp_weight;
			num = i;
		}
	}
	target.distance = other_list[num].polar_pos.distance;
	target.angle = other_list[num].polar_pos.angle;
}

void Pick_up(std::vector<Item> & item_list, SelfInfo & myself, struct PolarPosition& target)
{
	if (myself.status == PICKUP) return;
	if (!item_list.size())	return;
	int i, num = 0, temp_weight = 0, max_weight = 200 * DISTANCE_WEIGHT;
	for (i = 0; i < item_list.size(); i++)
	{
		temp_weight = item_list[i].polar_pos.distance * DISTANCE_WEIGHT;
		temp_weight += (1 - ITEM_DATA[item_list[i].type].occur) * OCCUR_WEIGHT;
		if (item_list[i].type == BONDAGE || item_list[i].type == FIRST_AID_CASE)
			temp_weight += (myself.hp_limit - myself.hp) * HPWEIGHT;
		if (item_list[i].type >= 10 && item_list[i].type <= 12)
			temp_weight += myself.hp * HPWEIGHT * ITEM_DATA[item_list[i].type].reduce;
		if ((item_list[i].type == BONDAGE || item_list[i].type == FIRST_AID_CASE) && myself.vocation == MEDIC)
			temp_weight += MEDIC_WEIGHT2;
		if (item_list[i].type > 1 && item_list[i].type <= 6 && myself.vocation == SNIPER)
			temp_weight += SNIPER_WEIGHT2;
		if (temp_weight >= max_weight)
		{
			max_weight = temp_weight;
			num = i;
		}
	}
	target.distance = item_list[num].polar_pos.distance;
	target.angle = item_list[num].polar_pos.angle;
}

bool Overlap(double angle)
{
	bool overlap = false;
	for (int i = 0; i < ObstacleAngle.size(); ++i)
	{
		if (ObstacleAngle[i].Start < angle
			&& ObstacleAngle[i].End > angle)
		{
			overlap = true;
			break;
		}
	}
	return overlap;
}

/*Input absolute move angle and absolute view angle
Return absolute move angle
Turning while walking disabled(commented)*/
double AttemptMove(double move_angle, double view_angle, int parameter = -1)
{
	//relative angle
	DiDiAngle move_angle_r(move_angle - info.self.view_angle);
	DiDiAngle view_angle_r(view_angle - info.self.view_angle);

	if (!Overlap(double(move_angle)))
	{
		move_angle_r = double(move_angle) - info.self.view_angle;
		view_angle_r = double(view_angle) - info.self.view_angle;
		move(double(move_angle_r), double(view_angle_r), parameter);
		return move_angle;
	}
	else
	{
		for (int i = 0; i < ObstacleAngle.size(); ++i)
		{
			if (ObstacleAngle[i].Start -1e-6 < double(move_angle)
				&& ObstacleAngle[i].End +1e-6 > double(move_angle))
			{
				if (ObstacleAngle[i].Start > -1e-6
					&& ObstacleAngle[i].Start < 1e-6)
				{
					//find the complementory part
					move_angle_r = 360;
					for (int j = 0; j < ObstacleAngle.size(); ++j)
					{
						if (ObstacleAngle[j].End - 360 > -1e-6
							&& ObstacleAngle[j].End - 360 < 1e-6)
							move_angle_r = ObstacleAngle[j].Start;
					}
					//No complementory part
					if (move_angle_r - 360 > -1e-6
						&& move_angle_r - 360 < 1e-6)
					{
						if (2 * move_angle > ObstacleAngle[i].End)
						{
							move_angle_r = ObstacleAngle[i].End - info.self.view_angle;
						}
						else
						{
							move_angle_r = 0.0 - info.self.view_angle;
						}
					}
					//Exist complementory part
					else
					{
						if (2 * move_angle + 360 - move_angle_r > ObstacleAngle[i].End)
						{
							move_angle_r = ObstacleAngle[i].End - info.self.view_angle;
						}
						else
						{
							move_angle_r = move_angle_r - info.self.view_angle;
						}
					}
					//move_angle_r = (double(move_angle_r) + ObstacleAngle[i].End) / 2 - info.self.view_angle;
					//move_angle_r = ObstacleAngle[i].End - info.self.view_angle;
					view_angle_r = move_angle_r;
					//view_angle_r = double(move_angle_r) + info.self.view_width / LookAround;
					move(double(move_angle_r), double(view_angle_r), -1);
					return ObstacleAngle[i].End;
				}
				else if (ObstacleAngle[i].End - 360 > -1e-6
						&& ObstacleAngle[i].End - 360 < 1e-6)
				{
					//find the complementory part
					move_angle_r = 0;
					for (int j = 0; j < ObstacleAngle.size(); ++j)
					{
						if (ObstacleAngle[j].Start > -1e-6
							&& ObstacleAngle[j].Start < 1e-6)
							move_angle_r = ObstacleAngle[j].End;
					}
					//No complementory part
					if (move_angle_r > -1e-6
						&& move_angle_r < 1e-6)
					{
						if (360.0 - 2 * (360.0-move_angle) < ObstacleAngle[i].Start)
						{
							move_angle_r = ObstacleAngle[i].Start - info.self.view_angle;
						}
						else
						{
							move_angle_r = 360.0 - info.self.view_angle;
						}
					}
					//Exist complementory part
					else
					{
						if (360 - 2 * move_angle - move_angle_r < ObstacleAngle[i].Start)
						{
							move_angle_r = ObstacleAngle[i].Start - info.self.view_angle;
						}
						else
						{
							move_angle_r = move_angle_r - info.self.view_angle;
						}
					}
					//move_angle_r = (double(move_angle_r) + ObstacleAngle[i].Start) / 2 - info.self.view_angle;
					//move_angle_r = ObstacleAngle[i].Start - info.self.view_angle;
					view_angle_r = move_angle_r;
					//view_angle_r = double(move_angle_r) - info.self.view_width / LookAround;
					move(double(move_angle_r), double(view_angle_r), -1);
					return ObstacleAngle[i].Start;
				}
				else
				{
					if (2 * double(move_angle) < ObstacleAngle[i].Start + ObstacleAngle[i].End)
					{
						/*move_angle_r = 0;
						for (int j = 0; j < ObstacleAngle.size(); ++j)
						{
							if (ObstacleAngle[j].End - ObstacleAngle[i].Start < -1e-6
								&& ObstacleAngle[j].End > double(move_angle_r))
								move_angle_r = ObstacleAngle[j].End;
						}

						move_angle_r = (double(move_angle_r) + ObstacleAngle[i].Start) / 2 - info.self.view_angle;*/
						move_angle_r = ObstacleAngle[i].Start - info.self.view_angle;
						view_angle_r = move_angle_r;
						//view_angle_r = double(move_angle_r) - info.self.view_width / LookAround;
						move(double(move_angle_r), double(view_angle_r), -1);
						return ObstacleAngle[i].Start;
					}
					else
					{
						/*move_angle_r = 360;
						for (int j = 0; j < ObstacleAngle.size(); ++j)
						{
							if (ObstacleAngle[j].Start - ObstacleAngle[i].End > 1e-6
								&& ObstacleAngle[j].Start < double(move_angle_r))
								move_angle_r = ObstacleAngle[j].Start;
						}
						move_angle_r = (double(move_angle_r) + ObstacleAngle[i].Start) / 2 - info.self.view_angle;*/
						move_angle_r = ObstacleAngle[i].End - info.self.view_angle;
						view_angle_r = move_angle_r;
						//view_angle_r = double(move_angle_r) + info.self.view_width / LookAround;
						move(double(move_angle_r), double(view_angle_r), -1);
						return ObstacleAngle[i].End;
					}
				}
				break;
			}
		}
	}
}

void play_game()
{
	/* Your code in this function */
	//Output
	std::ofstream outfile("Player_0_2.txt", std::ios::out | std::ios::app);
	std::ofstream outposfile("PlayerPos_0_2.txt", std::ios::out | std::ios::app);

	/*
	Moving priorities:
	1:Obstacles
	2:Enemies
	(2.5:Special items)
	3:Poison
	4:Items
	5:Nothing happens
	*/

	/* sample AI */
	update_info();
	outfile << "=============Frame" << frame << "=============\n";
	outfile << "HP:" << info.self.hp << "\n";
	outfile << "Pos:(" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
	outfile << "Target:(" << NSDDD.Target.x << "," << NSDDD.Target.y << ")\n";
	outfile << "Move:" << info.self.move_angle << "\twith CD" << info.self.move_cd << "\n";
	outfile << "View:"<< info.self.view_angle << "\n\n";
	outposfile 
		<< frame << '\t' 
		<< info.self.xy_pos.x << '\t' 
		<< info.self.xy_pos.y << '\t' 
		<< info.self.move_angle << '\t' 
		<< info.self.view_angle << std::endl;
	if (info.self.bag.size() > 1)
		outfile << "Package filled.(" << info.self.bag.size() << ")\n\n" << std::endl;
	if (frame == 0)
	{
		srand(34);
		XYPosition landing_point = { 350 + rand() % 50, 350 + rand() % 50 };
		parachute(SIGNALMAN, landing_point);
		NSDDD.Status = XIAZOU;
		NSDDD.Paodu = false;
		NSDDD.Target = { 500,500 };
		NSDDD.Former_pos = info.self.xy_pos;
		NSDDD.No_move_frames = 0;
		NSDDD.former_view_angle = info.self.view_angle;
		
		outfile.close();
		outposfile.close();
		return;
	}
	else
	{
		srand(time(nullptr));
	}

	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		outfile << "Jumping.\n\n" << std::endl;
		NSDDD.Former_pos = info.self.xy_pos;
		NSDDD.No_move_frames = 0;

		outfile.close();
		outposfile.close();
		return;
	}

	//Receive radio

	//Priority 1: Obstacles
	int landno = 0;
	int areax, areay;
	AREA areatype;
	block currentblock;
	double mindist;
	double minangle;
	double maxangle;
	ObstacleAngle.clear();

	try_update_info();
	areax = (int)floor(info.self.xy_pos.x / 100);
	areay = (int)floor(info.self.xy_pos.y / 100);
	
	for (int x = areax - 1; x <= areax + 1; ++x)
	{
		for (int y = areay - 1; y <= areay + 1; ++y)
		{
			if (x < 0 || x > 9 || y < 0 || y > 9) break;
			landno = y * 10 + x;
			areatype = MAP[landno];
			//outfile << "Search land" << landno << ",area" << (int)areatype << "\n";
			int rect_obst = 0;
			for (int b = 0; b < AREA_DATA[areatype].size(); ++b)
			{
				currentblock = AREA_DATA[areatype][b];
				if (currentblock.shape == CIRCLE)
				{
					currentblock.x0 += x * 100;
					currentblock.y0 += y * 100;
					mindist = (currentblock.x0 - info.self.xy_pos.x) * (currentblock.x0 - info.self.xy_pos.x);
					mindist += (currentblock.y0 - info.self.xy_pos.y) * (currentblock.y0 - info.self.xy_pos.y);
					mindist = sqrt(mindist) - currentblock.r;

					if (mindist <= MINOBSTACLE)
					{
						DiDiAngle currentpoint(currentblock.x0, currentblock.y0);
						DiDiAngle span;
						if (currentblock.r + 1.1 * VOCATION_DATA[info.self.vocation].radius > mindist + currentblock.r)
						{
							span = 90.0;
						}
						else
						{
							span = asin((currentblock.r + 1.1 * VOCATION_DATA[info.self.vocation].radius) / (mindist + currentblock.r)) * 180.0 / PI;
						}
						if (double(span) > -1e-6
							&& double(span) < 1e-6)
						{
							outfile << "ZEROSPAN!" << currentblock.r + 1.1* VOCATION_DATA[info.self.vocation].radius << "," << mindist + currentblock.r 
								<<","<< asin((currentblock.r + 1.1 * VOCATION_DATA[info.self.vocation].radius) / (mindist + currentblock.r)) * 180.0 / PI << "\n";
						}
						minangle = double(currentpoint - span);
						maxangle = double(currentpoint + span);

						outfile << "Circle obstacle detected at (" << currentblock.x0 << "," << currentblock.y0 << ")with radius " << currentblock.r << "\n";
						outfile << "\tabsolute angle" << double(currentpoint) << "\tspan" << double(span) << "\n";

						if (info.self.xy_pos.x < currentblock.x0
							&& info.self.xy_pos.y > double(currentblock.y0) - double(currentblock.r) - 1.1 * VOCATION_DATA[info.self.vocation].radius
							&& info.self.xy_pos.y < double(currentblock.y0) - double(currentblock.r) + 1.1 * VOCATION_DATA[info.self.vocation].radius)
						{
							ObstacleAngle.push_back({ 0.0, minangle });
							ObstacleAngle.push_back({ maxangle, 360.0 });
							outfile << "Circle obstacle detected at (" << maxangle << ",360),(0," << minangle << ")\n";
						}
						else
						{
							ObstacleAngle.push_back({ minangle, maxangle });
							outfile << "Circle obstacle detected at (" << minangle << "," << maxangle << ")\n";
						}
					}
				}
				else if (currentblock.shape == RECTANGLE)
				{
					currentblock.x0 += x * 100;
					currentblock.x1 += x * 100;
					currentblock.y0 += y * 100;
					currentblock.y1 += y * 100;

					if (InRect(currentblock, info.self.xy_pos.x, info.self.xy_pos.y + MINOBSTACLE))
					{
						//up
						rect_obst = 1000 + rect_obst % 1000;
					}
					if (InRect(currentblock, info.self.xy_pos.x, info.self.xy_pos.y - MINOBSTACLE))
					{
						//down
						rect_obst = (rect_obst / 1000) * 1000 + 100 + rect_obst % 100;
					}
					if (InRect(currentblock, info.self.xy_pos.x - MINOBSTACLE, info.self.xy_pos.y))
					{
						//left
						rect_obst = (rect_obst / 100) * 100 + 10 + rect_obst % 10;
					}
					if (InRect(currentblock, info.self.xy_pos.x + MINOBSTACLE, info.self.xy_pos.y))
					{
						//right
						rect_obst = (rect_obst / 10) * 10 + 1;
					}
				}
				else
					outfile << "!!Unknown block shape!\n";
			}

			minangle = 0;
			maxangle = 360;
			//if (rect_obst) outfile << "Rect_obst=" << rect_obst << "\n";
			switch (rect_obst)
			{
			case 0:
				continue;
			case 1000:
				minangle = 0.0;
				maxangle = 180.0;
				break;
			case 100:
				minangle = 180.0;
				maxangle = 360.0;
				break;
			case 10:
				minangle = 90.0;
				maxangle = 270.0;
				break;
			case 1:
				minangle = 270.0;
				maxangle = 360.0;
				break;
			case 1100:
				minangle = 0.0;
				maxangle = 180.0;
				break;
			case 11:
				minangle = 180.0;
				maxangle = 360.0;
				break;
			case 1010:
				minangle = 0.0;
				maxangle = 270.0;
				break;
			case 1001:
				ObstacleAngle.push_back({ 0.0, 180.0 });
				ObstacleAngle.push_back({ 270.0, 360.0 });
				outfile << "Rectangle obstacle detected at (0,180),(270,360)\n";
				continue;
			case 110:
				minangle = 90.0;
				maxangle = 360.0;
				break;
			case 101:
				ObstacleAngle.push_back({ 0.0, 90.0 });
				ObstacleAngle.push_back({ 180.0, 360.0 });
				outfile << "Rectangle obstacle detected at (0,90),(180,360)\n";
				continue;
			case 1110:
				minangle = 0.0;
				maxangle = 360.0;
				break;
			case 1101:
				ObstacleAngle.push_back({ 0.0, 180.0 });
				ObstacleAngle.push_back({ 180.0, 360.0 });
				outfile << "Rectangle obstacle detected at (0,180),(180,360)\n";
				continue;
			case 1011:
				ObstacleAngle.push_back({ 0.0, 270.0 });
				ObstacleAngle.push_back({ 270.0, 360.0 });
				outfile << "Rectangle obstacle detected at (0,270),(270,360)\n";
				continue;
			case 111:
				ObstacleAngle.push_back({ 0.0, 90.0 });
				ObstacleAngle.push_back({ 90.0, 360.0 });
				outfile << "Rectangle obstacle detected at (0,90),(90,360)\n";
				continue;
			default:
				outfile << "Unknown type!" << rect_obst << "\n";
				break;
			}
			ObstacleAngle.push_back({ minangle, maxangle });
			outfile << "Rectangle obstacle detected at (" << minangle << "," << maxangle << ")\n";
		}
	}
	outfile << "\n";

	bool repeat = false;
	do
	{
		repeat = false;
		switch (NSDDD.Status)
		{
		case XIAZOU:
		{
			outfile << "Xiazou ing\n";

			//Priority 2: Enemies
			struct PolarPosition Tar = { -1,0 };
			Shooting(info.others, info.self, Tar);
			if (Tar.distance > 0)
			{
				NSDDD.Status = ZHANDOU;
				exchange_polar(Tar, NSDDD.Target);
				repeat = true;
				continue;
			}
			else
			{
				NSDDD.Status = XIAZOU;
			}

			//Priority 2.5: Special items

			//Priority 3: Poison
			if (frame >= 200)//Poison appear after frame 200
			{
				double poison_dx = info.poison.current_center.x - info.self.xy_pos.x;
				double poison_dy = info.poison.current_center.y - info.self.xy_pos.y;
				double poison_ratio = (poison_dx * poison_dx + poison_dy * poison_dy);
				poison_ratio /= info.poison.current_radius * info.poison.current_radius;
				poison_ratio = sqrt(poison_ratio);
				double poison_dr = (1 - poison_ratio) * info.poison.current_radius;
				DiDiAngle poison_angle(info.poison.current_center.x, info.poison.current_center.y);
				if (NSDDD.Paodu && poison_dr >= MAXPOISON)
				{
					//Stop paodu
					outfile << "Paodu finish.(r=" << poison_ratio << ", dr=" << poison_dr << ")\n";
					outfile << "Me at (" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
					outfile << "Poison at(" << info.poison.current_center.x
						<< "," << info.poison.current_center.y << ") with radius"
						<< info.poison.current_radius << "\n\n";
					NSDDD.Status = XIAZOU;
					NSDDD.Paodu = false;
					/*poison_angle = poison_angle + (DiDiAngle(180.0));
					AttemptMove(double(poison_angle), double(poison_angle) + info.self.view_width / LookAround);
					moved = true;*/
				}
				else
				{
					if (poison_dr <= MINPOISON)
					{
						//Too close to the edge
						outfile << "Too close to the edge!.(r=" << poison_ratio << ", dr=" << poison_dr << ")\n";
						outfile << "Me at (" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
						outfile << "Poison at(" << info.poison.current_center.x
							<< "," << info.poison.current_center.y << ") with radius"
							<< info.poison.current_radius << "\n\n";
						SetTarget(info.poison.current_center);
						NSDDD.Paodu = true;
					}
					else
					{
						continue;
					}
				}
			}

			//Priority 4: Items
			if (!info.items.empty())
			{
				Item closest_item;
				closest_item.polar_pos.distance = 100000;
				for (int i = 0; i < info.items.size(); ++i)
				{
					if ((info.items[i].item_ID != CODE_CASE || info.self.vocation == HACK)
						&& info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
					{
						closest_item = info.items[i];
					}
				}
				outfile << "Closest item at(" << closest_item.polar_pos.distance << "," << closest_item.polar_pos.angle << ")\n";
				if (closest_item.polar_pos.distance < 1)
				{
					pickup(closest_item.item_ID);
					outfile << "try pickup" << closest_item.item_ID << "\n\n";
				}
				else
				{
					SetTarget(info.self.xy_pos.x + closest_item.polar_pos.distance * cos((info.self.view_angle + closest_item.polar_pos.angle) / 180 * PI),
						info.self.xy_pos.y + closest_item.polar_pos.distance * sin((info.self.view_angle + closest_item.polar_pos.angle) / 180 * PI));
					//NSDDD.Status = JIANBAO;
				}
				continue;
			}

			//Priority 5: Nothing happens, walk towards the center
			if (frame >= 200)
			{
				DiDiAngle center_angle(info.poison.current_center.x, info.poison.current_center.y);
				center_angle = double(center_angle) + 180 + 30;
				srand(frame);
				double random_shift = 100.0 + double(rand() % 25);
				SetTarget(500.0 + random_shift * cos(double(center_angle)), 500.0 + random_shift * sin(double(center_angle)));
			}
			else
			{
				DiDiAngle center_angle(500.0, 500.0);
				center_angle = double(center_angle) + 180 + 30;
				srand(frame);
				double random_shift = 100.0 + double(rand() % 25);
				SetTarget(500.0 + random_shift * cos(double(center_angle)), 500.0 + random_shift * sin(double(center_angle)));
			}
			DiDiAngle move_angle(NSDDD.Target.x, NSDDD.Target.y);
			/*
			double target_dist = (NSDDD.Target.x - info.self.xy_pos.x) * (NSDDD.Target.x - info.self.xy_pos.x);
			target_dist += (NSDDD.Target.y - info.self.xy_pos.y) * (NSDDD.Target.y - info.self.xy_pos.y);
			if (info.self.status != MOVING && info.self.status != MOVING_SHOOTING)
			{
				move_angle = double(rand() % 360);
				target_dist = 100;
			}
			else
			{
				move_angle = DiDiAngle(NSDDD.Target.x, NSDDD.Target.y);
			}

			if (Overlap(double(move_angle)) || target_dist < 50)
			{
				do
				{
					move_angle = double(rand() % 360);
				} while (Overlap(double(move_angle)));

				SetTarget(info.self.xy_pos.x + 100 * cos(move_angle / 180 * PI),
					info.self.xy_pos.y + 100 * sin(move_angle / 180 * PI));
				move_angle = double(move_angle) - info.self.view_angle;
				DiDiAngle view_angle(double(move_angle) + 0.0);
				//DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
				move(double(move_angle), double(view_angle), -1);
				outfile << "move" << double(move_angle) << "\n\n";
			}
			else
			{
				move_angle = double(move_angle) - info.self.view_angle;
				if (info.self.move_cd == 2)
				{
					DiDiAngle view_angle(double(move_angle) + 0.0);
					//DiDiAngle view_angle(double(move_angle) - info.self.view_width / LookAround);
					move(double(move_angle), double(view_angle), NOMOVE);
					outfile << "Change view, move" << double(move_angle) << "\n\n";
				}
				else
				{
					DiDiAngle view_angle(double(move_angle) + 0.0);
					//DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
					move(double(move_angle), double(view_angle), -1);
					outfile << "Interrupt move, move" << double(move_angle) << "\n\n";
				}
			}
			*/
			break;
		}
		case JIANBAO:
		{
			outfile << "Jianbao ing\n";
			break;
		}
		case ZHANDOU:
		{
			outfile << "Zhandou ing\n";
			ITEM weapon = FIST;
			struct PolarPosition Target = { -1,0 };
			Shooting(info.others, info.self, Target);
			if (Target.distance > 0)
			{
				int wuqi = shotgun(Target.distance, info.self.bag);
				if (wuqi >= 0)
				{
					weapon = info.self.bag[wuqi].type;
					shoot(weapon, Target.angle);
				}
				//What if not?
			}
			else
			{
				NSDDD.Status = XIAZOU;
			}
			break;
		}
		};
	} while (repeat);

	//If not moving
	double moved_last_frame_sq = (NSDDD.Former_pos.x - info.self.xy_pos.x) * (NSDDD.Former_pos.x - info.self.xy_pos.x);
	moved_last_frame_sq += (NSDDD.Former_pos.y - info.self.xy_pos.y) * (NSDDD.Former_pos.y - info.self.xy_pos.y);
	if (moved_last_frame_sq < 0.1 * 0.1)
	{
		NSDDD.No_move_frames++;
	}
	else
	{
		NSDDD.No_move_frames = 0;
	}

	if (NSDDD.No_move_frames >= 5)
	{
		outfile << "Stuck!\n";
		srand(frame);
		double random_angle = double(rand() % 120);

		PolarPosition inv_polar = { 10, 120 + random_angle };
		XYPosition inverse;
		exchange_polar(inv_polar, inverse);
		SetTarget(inverse);

		NSDDD.No_move_frames = 0;

		outfile.close();
		outposfile.close();
		return;
	}

	NSDDD.Former_pos = info.self.xy_pos;

	//Invalid target, which means standing still
	if (NSDDD.Target.x < 0
		|| NSDDD.Target.y < 0)
	{
		outfile << "Stand still.\n\n";
		outfile.close();
		outposfile.close();
		return;
	}

	//Calculating info about the target
	DiDiAngle target(NSDDD.Target.x, NSDDD.Target.y);
	target = double(target) - info.self.view_angle;
	double target_dist = (NSDDD.Target.x - info.self.xy_pos.x) * (NSDDD.Target.x - info.self.xy_pos.x);
	target_dist += (NSDDD.Target.y - info.self.xy_pos.y) * (NSDDD.Target.y - info.self.xy_pos.y);
	target_dist = sqrt(target_dist);

	//Rediscover the target(disabled)
	/*if (double(target) < info.self.view_width / 2
		|| double(target) > 360 - info.self.view_width / 2)
	{
		bool found = false;
		if (target_dist > info.self.view_distance)
		{
			found = true;
		}

		if (!info.items.empty())
		{
			for (int i = 0; i < info.items.size(); ++i)
			{
				if (info.items[i].polar_pos.angle - double(target) < 10
					&& info.items[i].polar_pos.angle - double(target) > -10
					&& info.items[i].polar_pos.distance - target_dist <3
					&& info.items[i].polar_pos.distance - target_dist > -3)
				{
					found = true;
					SetTarget(info.self.xy_pos.x + info.items[i].polar_pos.distance * cos(info.self.view_angle + info.items[i].polar_pos.angle / 180 * PI),
						info.self.xy_pos.y + info.items[i].polar_pos.distance * sin(info.self.view_angle + info.items[i].polar_pos.angle / 180 * PI));
				}
			}
		}

		if (!info.others.empty())
		{
			for (int i = 0; i < info.others.size(); ++i)
			{
				if (info.others[i].polar_pos.angle - double(target) < 10
					&& info.others[i].polar_pos.angle - double(target) > -10
					&& info.others[i].polar_pos.distance - target_dist <3
					&& info.others[i].polar_pos.distance - target_dist > -3)
				{
					found = true;
					SetTarget(info.self.xy_pos.x + info.others[i].polar_pos.distance * cos(info.self.view_angle + info.others[i].polar_pos.angle / 180 * PI),
						info.self.xy_pos.y + info.others[i].polar_pos.distance * sin(info.self.view_angle + info.others[i].polar_pos.angle / 180 * PI));
				}
			}
		}

		if (!found)
		{
			NSDDD.Status = XIAZOU;
		}
	}*/

	//If very close, change target
	//If turned reverse, change target
	if (target_dist < 1
		||(abs(NSDDD.former_view_angle - info.self.view_angle)-180 > -1e-6
			&& abs(NSDDD.former_view_angle - info.self.view_angle) - 180 < 1e-6))
	{
		NSDDD.Paodu = false;
		if (frame >= 200)
		{
			DiDiAngle center_angle(info.poison.current_center.x, info.poison.current_center.y);
			center_angle = double(center_angle) + 180 + 30;
			srand(frame);
			double random_shift = 100.0 + double(rand() % 25);
			SetTarget(500.0 + random_shift * cos(double(center_angle)), 500.0 + random_shift * sin(double(center_angle)));
		}
		else
		{
			DiDiAngle center_angle(500.0, 500.0);
			center_angle = double(center_angle) + 180 + 30;
			srand(frame);
			double random_shift = 100.0 + double(rand() % 25);
			SetTarget(500.0 + random_shift * cos(double(center_angle)), 500.0 + random_shift * sin(double(center_angle)));
		}
	}
	NSDDD.former_view_angle = info.self.view_angle;

	//If there is poison
	if (frame >= 200)
	{
		double poison_dx = info.poison.current_center.x - info.self.xy_pos.x;
		double poison_dy = info.poison.current_center.y - info.self.xy_pos.y;
		double poison_ratio = (poison_dx * poison_dx + poison_dy * poison_dy);
		poison_ratio /= info.poison.current_radius * info.poison.current_radius;
		poison_ratio = sqrt(poison_ratio);
		double poison_dr = (1 - poison_ratio) * info.poison.current_radius;
		if (NSDDD.Paodu && poison_dr >= MAXPOISON)
		{
			//Stop paodu
			outfile << "Paodu finish.(r=" << poison_ratio << ", dr=" << poison_dr << ")\n";
			outfile << "Me at (" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
			outfile << "Poison at(" << info.poison.current_center.x
				<< "," << info.poison.current_center.y << ") with radius"
				<< info.poison.current_radius << "\n\n";
			NSDDD.Status = XIAZOU;
			NSDDD.Paodu = false;
		}
		else if (poison_dr <= MINPOISON)
		{
			//Too close to the edge
			outfile << "Too close to the edge!.(r=" << poison_ratio << ", dr=" << poison_dr << ")\n";
			outfile << "Me at (" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
			outfile << "Poison at(" << info.poison.current_center.x
				<< "," << info.poison.current_center.y << ") with radius"
				<< info.poison.current_radius << "\n\n";
			SetTarget(info.poison.current_center);
			NSDDD.Paodu = true;
		}
	}

	//move towards the target
	DiDiAngle move_angle(NSDDD.Target.x, NSDDD.Target.y);
	if (info.self.move_cd == 2)
	{
		;
	}
	else
	{
		DiDiAngle view_angle(double(move_angle) - 0.0);
		//DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
		outfile << "move" << AttemptMove(double(move_angle), double(view_angle), -1) << "\n\n";
	}

	outfile.close();
	outposfile.close();
	return;
}