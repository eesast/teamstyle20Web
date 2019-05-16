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

std::vector<int> ID_needhelp;
std::vector<XYPosition> needhelp_pos;

//Constants
const double PI = 3.141592654;
double MINPOISON = 100;
double MAXPOISON = 150;
const double MINOBSTACLE = 5;
const double MINWALK = 1.5;
const double LookAround = 2.5;
const double radius = 0.7;

#define MEDIC_WEIGHT1 80
#define SNIPER_WEIGHT1 80
#define MEDIC_WEIGHT2 80
#define SNIPER_WEIGHT2 80
#define HPWEIGHT 1
#define DISTANCE_WEIGHT -0.8
#define OCCUR_WEIGHT 80

struct Obstacle//不能走的区域
{
	int index;
	block this_block;
};

enum DiDi//状态
{
	XIAZOU = 0,
	JIANBAO = 1,
	MUBIAO = 2,
	ZHANDOU = 3
};

struct
{
	DiDi Status;
	XYPosition Target;
	XYPosition NextTarget;
	bool Paodu;//是否在跑毒
	XYPosition Former_pos;
	int No_move_frames;
	double former_move_angle;
	double former_view_angle;
	
	int stuck_countdown;

} NSDDD;

bool Is_Same_Block(block b1, block b2)
{
	if (b1.r == b2.r
		&& b1.shape == b2.shape
		&& b1.type == b2.type
		&& b1.x0 == b2.x0
		&& b1.x1 == b2.x1
		&& b1.y0 == b2.y0
		&& b1.y1 == b2.y1)
		return true;
	else
		return false;
}

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

DiDiAngle & DiDiAngle::operator+(DiDiAngle & a2)
{
	this->angle += a2.angle;
	if (this->angle > 360) this->angle -= 360;
	return *this;
}

DiDiAngle& DiDiAngle::operator-(DiDiAngle & a2)
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

std::list<XYPosition> Route;

bool in_block(block& b, double x, double y)
{
	if (b.shape == CIRCLE)
	{
		if (b.type == CIRCLE_GRASS)
			return false;

		if ((x - b.x0) * (x - b.x0) + (y - b.y0) * (y - b.y0) < (b.r + 1.5 * radius) * (b.r + 1.5 * radius))
		{
			return true;
		}
	}
	else
	{
		if (b.type == RECTANGLE_GRASS || b.type == SHALLOW_WATER)
			return false;

		if (b.type == WALL)
		{
			int areax = (b.x0 / 100) * 100;
			int areay = (b.y0 / 100) * 100;
			switch ((b.x0 % 100) + (b.x1 % 100))
			{
			case 11:
			case 40:
				b.x0 = areax + 4;
				b.y0 = areay + 36;
				b.x1 = areax + 36;
				b.y1 = areay + 4;
				break;
			case 71:
			case 129:
				b.x0 = areax + 64;
				b.y0 = areay + 36;
				b.x1 = areax + 96;
				b.y1 = areay + 4;
				break;
			case 100:
			case 101:
				b.x0 = areax + 4;
				b.y0 = areay + 96;
				b.x1 = areax + 36;
				b.y1 = areay + 64;
				break;
			case 188:
			case 160:
				b.x0 = areax + 64;
				b.y0 = areay + 96;
				b.x1 = areax + 96;
				b.y1 = areay + 64;
				break;
			}
		}

		if (x > double(b.x0 - 1.5 * radius)
			&& x < double(b.x1 + 1.5 * radius)
			&& y > double(b.y1 - 1.5 * radius)
			&& y < double(b.y0 + 1.5 * radius))
		{
			return true;
		}
	}
	return false;
}

bool Blocked(block& obstacle, double x, double y, double angle)
{
	int areax = (int)floor(x / 100);
	int areay = (int)floor(y / 100);
	int landno = areay * 10 + areax;
	AREA areatype = MAP[landno];
	//outfile << "Search land" << landno << ",area" << (int)areatype << "\n";
	for (int b = 0; b < AREA_DATA[areatype].size(); ++b)
	{
		obstacle = AREA_DATA[areatype][b];
		if (obstacle.shape == CIRCLE)
		{
			obstacle.x0 += areax * 100;
			obstacle.y0 += areay * 100;
		}
		else
		{
			obstacle.x0 += areax * 100;
			obstacle.x1 += areax * 100;
			obstacle.y0 += areay * 100;
			obstacle.y1 += areay * 100;
		}
		if (in_block(obstacle, x, y))
			return true;
	}
	return false;
}

double dist(XYPosition a, XYPosition b)
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

/*
1 2 3
8 b 4
7 6 5
*/
int Find_area(block b, double x, double y)
{
	if (b.shape == CIRCLE) return -2;
	if (x < b.x0)
	{
		if (y < b.y1)
		{
			return 7;
		}
		else if (y > b.y0)
		{
			return 1;
		}
		else
		{
			return 8;
		}
	}
	else if (x > b.x1)
	{
		if (y < b.y1)
		{
			return 5;
		}
		else if (y > b.y0)
		{
			return 3;
		}
		else
		{
			return 4;
		}
	}
	else
	{
		if (y < b.y1 + 1e6)
		{
			return 6;
		}
		else if (y > b.y0 - 1e6)
		{
			return 2;
		}
		else
		{
			//std::cout << "(" << b.x0 << "," << b.y0 << "),(" << b.x1 << "," << b.y1 << "),(" << x << "," << y << ")\n";
			return -1;
		}
	}
}

void Find_Route(std::list<XYPosition>::iterator begin)
{
	if (Route.size() > 100) return;
	std::list<XYPosition>::iterator end = begin;
	end++;
	double angle = atan2((*end).y - (*begin).y, (*end).x - (*begin).x);
	double length = dist(*begin, *end);
	int n = (int)floor(length / 0.5);
	block obstacle;
	int count = 0;
	XYPosition Mod_end = *end;
	while (Blocked(obstacle, Mod_end.x, Mod_end.y, angle))
	{
		Mod_end.x = Mod_end.x - ((*end).x - (*begin).x) / n;
		Mod_end.y = Mod_end.y - ((*end).y - (*begin).y) / n;
		count++;
	}
	if (count > 0)
	{
		count += 5;
		Mod_end.x = Mod_end.x - ((*end).x - (*begin).x) / n * 5;
		Mod_end.y = Mod_end.y - ((*end).y - (*begin).y) / n * 5;

		n -= count;
		end = Route.insert(end, Mod_end);
	}
	
	bool b;
	for (int i = 1; i < n; ++i)
	{
		b = Blocked(obstacle, (*begin).x + ((*end).x - (*begin).x) / n * i, (*begin).y + ((*end).y - (*begin).y) / n * i, angle);
		if (b)
		{
			XYPosition Alt1, Alt2, Alt11, Alt22;
			if (obstacle.shape == CIRCLE)
			{
				Alt1 = { obstacle.x0 + (obstacle.r + 2 * radius) * cos(angle + PI / 2), obstacle.y0 + (obstacle.r + 2 * radius) * sin(angle + PI / 2) };
				Alt2 = { obstacle.x0 + (obstacle.r + 2 * radius) * cos(angle - PI / 2), obstacle.y0 + (obstacle.r + 2 * radius) * sin(angle - PI / 2) };
				if (dist(*begin, Alt1) + dist(Alt1, *end) < dist(*begin, Alt2) + dist(Alt2, *end))
				{
					end = Route.insert(end, Alt1);
					Find_Route(begin);
					begin = end;
					end++;
					Find_Route(begin);
				}
				else
				{
					end = Route.insert(end, Alt2);
					Find_Route(begin);
					begin = end;
					end++;
					Find_Route(begin);
				}
			}
			else if (obstacle.shape == RECTANGLE)
			{
				int a1 = Find_area(obstacle, (*begin).x, (*begin).y);
				int a2 = Find_area(obstacle, (*end).x, (*end).y);
				XYPosition Corner1 = { obstacle.x0 - 2 * radius, obstacle.y0 + 2 * radius };
				XYPosition Corner3 = { obstacle.x1 + 2 * radius, obstacle.y0 + 2 * radius };
				XYPosition Corner5 = { obstacle.x1 + 2 * radius, obstacle.y1 - 2 * radius };
				XYPosition Corner7 = { obstacle.x0 - 2 * radius, obstacle.y1 - 2 * radius };
				int fflag = a2 - a1;
				while (fflag < 0) fflag += 8;
				while (fflag > 7) fflag -= 8;
				switch (fflag)
				{
				case 0:
					break;
				case 1:
				case 7:
					switch (a1 * 10 + a2)
					{
					case 12:
					case 18:
					case 21:
					case 81:
						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 32:
					case 34:
					case 23:
					case 43:
						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 54:
					case 56:
					case 45:
					case 65:
						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 76:
					case 78:
					case 67:
					case 87:
						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					default: //std::cout << "Miss in case 1&7(" << a1 << "," << a2 << ").\n";
						break;
					}
					break;
				case 2:
				case 6:
					switch (a1 * 10 + a2)
					{
					case 13:
						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 31:
						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 35:
						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 53:
						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 57:
						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 75:
						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 17:
						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 71:
						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 24:
					case 42:
						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 46:
					case 64:
						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 68:
					case 86:
						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 28:
					case 82:
						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					default: //std::cout << "Miss in case 2&6.\n";
						break;
					}
					break;
				case 3:
				case 5:
					switch (a1 * 10 + a2)
					{
					case 14:
					case 41:
					case 25:
					case 52:
						end = Route.insert(end, Corner3);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 36:
					case 63:
					case 47:
					case 74:
						end = Route.insert(end, Corner5);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 58:
					case 85:
					case 61:
					case 16:
						end = Route.insert(end, Corner7);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					case 72:
					case 27:
					case 83:
					case 38:
						end = Route.insert(end, Corner1);
						Find_Route(begin);
						begin = end;
						end++;

						Find_Route(begin);
						break;
					default: //std::cout << "Miss in case 3&5.(" << a1 << "," << a2 << ")\n";
						break;
					}
					break;
				case 4:
					switch (a1)
					{
					case 1:
					case 5:
						Alt1 = Corner3;
						Alt2 = Corner7;
						if (dist(*begin, Alt1) + dist(Alt1, *end) < dist(*begin, Alt2) + dist(Alt2, *end))
						{
							end = Route.insert(end, Alt1);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						else
						{
							end = Route.insert(end, Alt2);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						break;
					case 2:
						Alt1 = Corner1;	Alt11 = Corner7;
						Alt2 = Corner3;	Alt22 = Corner5;
						if (dist(*begin, Alt1) + dist(Alt1, Alt11) + dist(Alt11, *end) < dist(*begin, Alt2) + dist(Alt2, Alt22) + dist(Alt22, *end))
						{
							end = Route.insert(end, Alt1);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt11);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						else
						{
							end = Route.insert(end, Alt2);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt22);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						break;
					case 3:
					case 7:
						Alt1 = Corner1;
						Alt2 = Corner5;
						if (dist(*begin, Alt1) + dist(Alt1, *end) < dist(*begin, Alt2) + dist(Alt2, *end))
						{
							end = Route.insert(end, Alt1);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						else
						{
							end = Route.insert(end, Alt2);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						break;
					case 4:
						Alt1 = Corner3;	Alt11 = Corner1;
						Alt2 = Corner5;	Alt22 = Corner7;
						if (dist(*begin, Alt1) + dist(Alt1, Alt11) + dist(Alt11, *end) < dist(*begin, Alt2) + dist(Alt2, Alt22) + dist(Alt22, *end))
						{
							end = Route.insert(end, Alt1);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt11);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						else
						{
							end = Route.insert(end, Alt2);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt22);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						break;
					case 6:
						Alt1 = Corner7;	Alt11 = Corner1;
						Alt2 = Corner5;	Alt22 = Corner3;
						if (dist(*begin, Alt1) + dist(Alt1, Alt11) + dist(Alt11, *end) < dist(*begin, Alt2) + dist(Alt2, Alt22) + dist(Alt22, *end))
						{
							end = Route.insert(end, Alt1);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt11);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						else
						{
							end = Route.insert(end, Alt2);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt22);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						break;
					case 8:
						Alt1 = Corner1;	Alt11 = Corner3;
						Alt2 = Corner7;	Alt22 = Corner5;
						if (dist(*begin, Alt1) + dist(Alt1, Alt11) + dist(Alt11, *end) < dist(*begin, Alt2) + dist(Alt2, Alt22) + dist(Alt22, *end))
						{
							end = Route.insert(end, Alt1);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt11);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						else
						{
							end = Route.insert(end, Alt2);
							Find_Route(begin);
							begin = end;
							end++;

							end = Route.insert(end, Alt22);
							Find_Route(begin);
							begin = end;
							end++;

							Find_Route(begin);
						}
						break;
					default: //std::cout << "Miss in case 4.(" << a1 << ")\n";
						break;
					}
					break;
				default: //std::cout << "Miss in main case.\n";
					break;
				}
			}
			break;
		}
	}
}


//Shared functions
/*Return a possible hiding place*/
//XYPosition hidden();

void SetTarget(double x = -1, double y = -1)
{
	NSDDD.Target.x = x;
	NSDDD.Target.y = y;
	Route.clear();
	Route.push_back(info.self.xy_pos);
	Route.push_back(NSDDD.Target);
	Find_Route(Route.begin());
	Route.pop_front();
	NSDDD.NextTarget = *(Route.begin());
}

void SetTarget(XYPosition xy)
{
	NSDDD.Target = xy;
	Route.clear();
	Route.push_back(info.self.xy_pos);
	Route.push_back(NSDDD.Target);
	Find_Route(Route.begin());
	Route.pop_front();
	NSDDD.NextTarget = *(Route.begin());
}

/*exchange_position*/
PolarPosition exchange_xy(XYPosition& pos, PolarPosition& polar)
{
	polar.distance = polar.angle = 0.0;
	polar.distance = sqrt(pow((pos.x - info.self.xy_pos.x), 2) + pow((pos.y - info.self.xy_pos.y), 2));
	polar.angle = atan2((pos.y - info.self.xy_pos.y), (pos.x - info.self.xy_pos.x)) * 180 / PI - info.self.view_angle;
	while (polar.angle < 0) polar.angle += 360;
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

int shotgun(double distance, std::vector<Item> & mybag)
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

int Pick_up(std::vector<Item> &item_list, SelfInfo &myself)
{	//if(myself.status == PICKUP) return -1;
	//if(!item_list.size())	return -1;
	int i, num = 0, temp_weight = 0, max_weight = 200 * DISTANCE_WEIGHT;
	for (i = 0; i < item_list.size(); i++)
	{
		temp_weight = item_list[i].polar_pos.distance*DISTANCE_WEIGHT;
		temp_weight += (1 - ITEM_DATA[item_list[i].type].occur) * OCCUR_WEIGHT;
		if (item_list[i].type == BONDAGE || item_list[i].type == FIRST_AID_CASE)
			temp_weight += (myself.hp_limit - myself.hp) * HPWEIGHT;
		if (item_list[i].type >= 10 && item_list[i].type <= 12)
			temp_weight += myself.hp*HPWEIGHT*ITEM_DATA[item_list[i].type].reduce;
		if ((item_list[i].type == BONDAGE || item_list[i].type == FIRST_AID_CASE) && myself.vocation == MEDIC)
			temp_weight += MEDIC_WEIGHT2;
		if (item_list[i].type == SNIPER_BARRETT || item_list[i].type == SNIPER_RILFE && myself.vocation == SNIPER)
			temp_weight += SNIPER_WEIGHT2;
		if (temp_weight >= max_weight)
		{
			max_weight = temp_weight;
			num = i;
		}
	}
	return num;
}

bool Havedrag() {
	for (int j = 0; j < info.items.size(); j++) {
		if (info.items.at(j).type == BONDAGE || info.items.at(j).type == FIRST_AID_CASE) {
			return true;
		}
	}
	return false;
}

XYPosition Searching(int ID) {
	XYPosition pos;
	if (ID >= 0) {
		for (int j = 0; j < info.others.size(); j++) {
			if (info.others.at(j).player_ID == ID) {
				PolarPosition polarpos = info.others.at(j).polar_pos;
				pos = { polarpos.distance * cos(polarpos.angle) + info.self.xy_pos.x, polarpos.distance * sin(polarpos.angle) + info.self.xy_pos.y };
				return pos;
			}
		}
		pos = { -1,-1 };
		return pos;
	}
	else {
		for (int j = 0; j < info.others.size(); j++) {
			if (Isteammate(info.others.at(j).player_ID) && info.others.at(j).status == DEAD) {
				ID_needhelp.push_back(info.others.at(j).player_ID);
				PolarPosition polarpos = info.others.at(j).polar_pos;
				pos = { polarpos.distance * cos(polarpos.angle) + info.self.xy_pos.x, polarpos.distance * sin(polarpos.angle) + info.self.xy_pos.y };
				return pos;
			}
		}
		pos = { -1,-1 };
		return pos;
	}
}

void Treat(int ID) {
	int i = -1;
	for (int j = 0; j < info.items.size(); j++) {
		if (info.items.at(j).type == FIRST_AID_CASE) {
			i = j;
			break;
		}
		else if (info.items.at(j).type == BONDAGE) {
			i = j;
		}
	}
	shoot(info.items.at(i).type, 0, ID);
}



/*
send_info:
input:
the people you want to send info
the type of info
the x&y position(if needed)

type:
type 0: your own situation
type 1: fight(the enermy's x&y positions are needed)
type 2: escape(the nearest shelter's positions are needed)
type 3: get_bag(the bag's x&y positions are needed)
type 4: rendezvous(the assembling place's x&y positio are needed)
type 5: your own aggressivity
*/

void send_info(int receiver_ID, int type, int num_x = 0, int num_y = 0)
{
	int32_t message;
	switch (type)
	{
	case 0:
		message = info.self.xy_pos.x * (int)(int)pow(2, 16) + info.self.xy_pos.y * (int)pow(2, 6) + info.self.hp;
		break;
	case 1:
		message = 1 * (int)pow(2, 26) + num_x * (int)pow(2, 16) + num_y * (int)pow(2, 6);
		break;
	case 2:
		message = 2 * (int)pow(2, 26) + num_x * (int)pow(2, 16) + num_y * (int)pow(2, 6);
		break;
	case 3:
		message = 3 * (int)pow(2, 26) + num_x * (int)pow(2, 16) + num_y * (int)pow(2, 6);
		break;
	case 4:
		message = 4 * (int)pow(2, 26) + num_x * (int)pow(2, 16) + num_y * (int)pow(2, 6);
		break;
	case 5:
		message = 6 * (int)pow(2, 26);
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].type == WEAPON)
			{
				message += info.self.bag[i].item_ID * (int)pow(2, 12);
				message += info.self.bag[i].durability * (int)pow(2, 6);
			}
			else if (ITEM_DATA[info.self.bag[i].type].type == ARMOR)
			{
				message += (info.self.bag[i].item_ID - 9) * (int)pow(2, 3);
			}
			else if (ITEM_DATA[info.self.bag[i].type].type == GOODS)
			{
				message += (info.self.bag[i].item_ID - 13);
			}
		}
		break;
	}
	radio(receiver_ID, message);
}

/*
get_info:
Collating radio information and decoding
The final implementation also has a finite state machine that transfers states based on known information
*/

void get_info()
{
	try_update_info();
	while (!info.sounds.empty())
	{
		int msg;
		Sound sound_re;
		sound_re = info.sounds.front();
		info.sounds.pop_back();
		msg = sound_re.parameter;
		if (sound_re.sender != -1)
		{
			int info_get[5] = { 0 };
			int type = (msg / (int)pow(2, 26)) % ((int)pow(2, 3));
			info_get[0] = type;
			switch (type)
			{
			case 0:
				info_get[1] = (msg / (int)pow(2, 16)) % (int)pow(2, 10);
				info_get[2] = (msg / (int)pow(2, 6)) % (int)pow(2, 10);
				info_get[3] = msg % (int)pow(2, 6);
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				info_get[1] = (msg / (int)pow(2, 16)) % (int)pow(2, 10);
				info_get[2] = (msg / (int)pow(2, 6)) % (int)pow(2, 10);
				break;
			case 5:
				info_get[1] = (msg / (int)pow(2, 12)) % (int)pow(2, 4);
				info_get[2] = (msg / (int)pow(2, 6)) % (int)pow(2, 6);
				info_get[3] = (msg / (int)pow(2, 3)) % (int)pow(2, 3) + 9;
				info_get[4] = (msg % (int)pow(2, 3)) + 13;
				break;
			default:break;
			}

			int flag = 0;
			XYPosition xxyy;
			xxyy.x = info_get[1];
			xxyy.y = info_get[2];
			PolarPosition polar;
			polar = exchange_xy(xxyy, polar);
			switch (info_get[0])
			{
			case 0:
				/*
				if (info_get[3] < 10)
				{
					for (int i = 0; i < info.self.bag.size(); ++i)
					{
						if (ITEM_DATA[info.self.bag[i].type].type == GOODS)
							flag = i;
					}
					if (flag)
						move(info_get[1], info_get[2]);
				}
				*/
				break;
			case 1:
				SetTarget(xxyy);
				Shooting(info.others, info.self, polar);
				break;
			case 2:
				SetTarget(xxyy);
				break;
			case 3:
				SetTarget(xxyy);
				Pick_up(info.self.bag, info.self);
				break;
			case 4:
				if (Havedrag() && info.self.vocation == MEDIC) {
					SetTarget(xxyy);
					ID_needhelp.push_back(sound_re.sender);
					needhelp_pos.push_back(xxyy);
				}
				break;
			default: break;
			}
			break;
		}
	}
}

int find_item(std::vector<Item> &mybag, int item_num)	/*BONDAGE对应1，FIRST_AID_CASE对应2*/
{
	int i = 0;
	for (i = 0; i < mybag.size(); i++)
	{
		if (item_num == 1 && mybag[i].type == BONDAGE)
			return i;
		else if (item_num == 2 && mybag[i].type == FIRST_AID_CASE)
			return i;
	}
	return 0;
}

void recover(SelfInfo &myself)
{
	int hurt = myself.hp_limit - myself.hp;
	int i = find_item(myself.bag, 1);
	int j = find_item(myself.bag, 2);
	if (hurt)
	{
		if (hurt < 20 && i >= 0)
			shoot(myself.bag[i].type, 0, -1);
		else if (j >= 0)
			shoot(myself.bag[j].type, 0, -1);
		else if (i >= 0)
			shoot(myself.bag[i].type, 0, -1);
	}
}

void play_game()
{
	/* Your code in this function */
	//Output
	std::ofstream outfile("Player_0_2.txt", std::ios::out | std::ios::app);
	std::ofstream outposfile("PlayerPos_0_2.txt", std::ios::out | std::ios::app);

	/* sample AI */
	update_info();
	/*outfile << "=============Frame" << frame << "=============\n";
	outfile << "HP:" << info.self.hp << "\n";
	outfile << "Pos:(" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
	outfile << "Target:(" << NSDDD.Target.x << "," << NSDDD.Target.y << ")\n";
	outfile << "Move:" << info.self.move_angle << "\twith CD" << info.self.move_cd << "\n";
	outfile << "View:" << info.self.view_angle << "\n\n";
	outposfile
		<< frame << '\t'
		<< info.self.xy_pos.x << '\t'
		<< info.self.xy_pos.y << '\t'
		<< info.self.move_angle << '\t'
		<< info.self.view_angle << std::endl;*/
	/*if (info.self.bag.size() > 1)
		outfile << "Package filled.(" << info.self.bag.size() << ")\n\n" << std::endl;
*/
	if (frame == 0)
	{
		srand(55);
		XYPosition landing_point = { 750 + rand() % 50, 450 + rand() % 50 };
		parachute(HACK, landing_point);
		NSDDD.Status = XIAZOU;
		NSDDD.Paodu = false;
		NSDDD.Target = { 500,500 };
		NSDDD.Former_pos = info.self.xy_pos;
		NSDDD.No_move_frames = 0;
		NSDDD.former_view_angle = info.self.view_angle;
		NSDDD.former_move_angle = info.self.move_angle;
		NSDDD.stuck_countdown = 0;

		outfile.close();
		outposfile.close();
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		//outfile << "jumping" << std::endl;
		NSDDD.Former_pos = info.self.xy_pos;
		NSDDD.No_move_frames = 0;

		outfile.close();
		outposfile.close();
		return;
	}
	if (info.self.status == PICKUP)
	{
		//outfile << "picking" << std::endl;

		outfile.close();
		outposfile.close();
		return;
	}
	if (frame > 200)
	{
		MINPOISON = info.poison.current_radius / 1.3;
		MAXPOISON = MINPOISON + 50;
	}
	//Receive radio
	if (NSDDD.Status != ZHANDOU && info.self.vocation != MEDIC && info.self.hp / info.self.hp_limit < 0.4) {
		send_info(teammates.front(), 4, int(info.self.xy_pos.x), int(info.self.xy_pos.y));
		//outfile << info.player_ID << " ask for help" << std::endl;
	}
	get_info();
	if (Havedrag()) {
		if (ID_needhelp.size() == 0) {
			XYPosition pos = Searching(-1);
			if (pos.x > 0 && pos.y > 0) {
				SetTarget(pos);
				//outfile << "Find a teammate need help" << std::endl;
			}
		}
		if (ID_needhelp.size() != 0) {
			if ((info.self.xy_pos.x - needhelp_pos.front().x) * (info.self.xy_pos.x - needhelp_pos.front().x) + (info.self.xy_pos.y - needhelp_pos.front().y) * (info.self.xy_pos.y - needhelp_pos.front().y) < 1) {
				bool flag = false;
				for (int i = 0; i < info.others.size(); i++) {
					if (info.others.at(i).player_ID == ID_needhelp.front()) {
						Treat(ID_needhelp.front());
						ID_needhelp.pop_back();
						needhelp_pos.pop_back();
						flag = true;
						//outfile << "Treat sucessfully" << std::endl;
					}
				}
				if (!flag) {
					move(180, 180, 0);
				}
			}
			else {
				SetTarget(needhelp_pos.front());
			}
		}
	}
	/*
	Moving priorities:
	1:Obstacles
	2:Enemies
	(2.5:Special items)
	3:Poison
	4:Items
	5:Nothing happens
	*/

	//Priority 1: Obstacles

	bool repeat = false;
	do
	{
		repeat = false;

		switch (NSDDD.Status)
		{
		case XIAZOU:
		{
			//outfile << "Xiazou ing\n";

			//Priority 2: Enemies
			struct PolarPosition Tar = { -1,0 };
			Shooting(info.others, info.self, Tar);
			if (Tar.distance > 0)
			{
				NSDDD.Status = ZHANDOU;
				XYPosition enemy_pos;
				exchange_polar(Tar, enemy_pos);
				SetTarget(enemy_pos);
				repeat = true;
				continue;
			}
			else
			{
				NSDDD.Status = XIAZOU;
			}

			//Priority 2.5: Special items

			//Priority 3: Poison

			//Priority 4: Items
			if (!info.items.empty())
			{
				Item closest_item;
				int i = Pick_up(info.items, info.self);
				closest_item = info.items[i];
				//outfile << "Closest item at(" << closest_item.polar_pos.distance << "," << closest_item.polar_pos.angle << ")\n";
				XYPosition closetitem;
				exchange_polar(closest_item.polar_pos, closetitem);
				SetTarget(closetitem);
				if (closest_item.polar_pos.distance < 1)
				{
					pickup(closest_item.item_ID);
					//outfile << "try pickup" << closest_item.item_ID << "\n\n";
					SetTarget(info.poison.next_center);
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
			if (info.poison.move_flag == 0 || info.poison.move_flag == 2)
			{
				DiDiAngle center_angle(info.poison.next_center.x, info.poison.next_center.y);
				center_angle = double(center_angle) + 180 + 30;
				srand(frame);
				double random_shift = double(rand() % 50);
				SetTarget(info.poison.next_center.x + random_shift * cos(double(center_angle)), info.poison.next_center.y + random_shift * sin(double(center_angle)));
			}
			else
			{
				DiDiAngle center_angle(info.poison.current_center.x, info.poison.current_center.y);
				center_angle = double(center_angle) + 180 + 30;
				srand(frame);
				double random_shift = double(rand() % 50);
				SetTarget(info.poison.current_center.x + random_shift * cos(double(center_angle)), info.poison.current_center.y + random_shift * sin(double(center_angle)));
			}
			break;
		}
		case JIANBAO:
		{
			//outfile << "Jianbao ing\n";
			break;
		}
		case ZHANDOU:
		{
			//outfile << "Zhandou ing\n";
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
				recover(info.self);
				NSDDD.Status = XIAZOU;
			}
			break;
		}
		};
	} while (repeat);

	if (NSDDD.stuck_countdown > 0)
	{
		NSDDD.stuck_countdown--;
	}
	
	//If not moving
	double moved_last_frame_sq = (NSDDD.Former_pos.x - info.self.xy_pos.x) * (NSDDD.Former_pos.x - info.self.xy_pos.x);
	moved_last_frame_sq += (NSDDD.Former_pos.y - info.self.xy_pos.y) * (NSDDD.Former_pos.y - info.self.xy_pos.y);
	if (moved_last_frame_sq < 0.1 * 0.1)
	{
		NSDDD.No_move_frames++;
		if (NSDDD.No_move_frames >= 5 || NSDDD.stuck_countdown > 0)
		{
			//outfile << "Stuck!\n";
			srand(frame);
			double random_angle = double(rand() % 120) -60;
			if (random_angle > 0) random_angle += 60.0;
			if (random_angle < 0) random_angle -= 60;

			PolarPosition inv_polar = { 10, random_angle };
			XYPosition inverse;
			exchange_polar(inv_polar, inverse);
			SetTarget(inverse);

			NSDDD.No_move_frames = 0;
			NSDDD.stuck_countdown = 6;
		}
	}
	else
	{
		NSDDD.No_move_frames = 0;
	}

	NSDDD.Former_pos = info.self.xy_pos;

	//If there is poison
	if (frame >= 200 && NSDDD.stuck_countdown == 0)
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
			/*outfile << "Paodu finish.(r=" << poison_ratio << ", dr=" << poison_dr << ")\n";
			outfile << "Me at (" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
			outfile << "Poison at(" << info.poison.current_center.x
				<< "," << info.poison.current_center.y << ") with radius"
				<< info.poison.current_radius << "\n\n";*/
			NSDDD.Status = XIAZOU;
			NSDDD.Paodu = false;

			//move to next center
			DiDiAngle center_angle(info.poison.next_center.x, info.poison.next_center.y);
			center_angle = double(center_angle) + 180 + 30;
			srand(frame);
			double random_shift = double(rand() % 50);
			SetTarget(info.poison.next_center.x + random_shift * cos(double(center_angle)), info.poison.next_center.y + random_shift * sin(double(center_angle)));
		}
		else if (poison_dr <= MINPOISON)
		{
			//Too close to the edge
			/*outfile << "Too close to the edge!.(r=" << poison_ratio << ", dr=" << poison_dr << ")\n";
			outfile << "Me at (" << info.self.xy_pos.x << "," << info.self.xy_pos.y << ")\n";
			outfile << "Poison at(" << info.poison.current_center.x
				<< "," << info.poison.current_center.y << ") with radius"
				<< info.poison.current_radius << "\n\n";*/
			SetTarget(info.poison.current_center);
			NSDDD.Paodu = true;
		}
	}

	//Invalid target, which means standing still
	if (NSDDD.NextTarget.x < 0
		|| NSDDD.NextTarget.y < 0)
	{
		//outfile << "Stand still.\n\n";
		outfile.close();
		outposfile.close();
		return;
	}

	//Calculating info about the target
	double target_dist;
	target_dist = dist(NSDDD.NextTarget, info.self.xy_pos);

	while (target_dist < 1)
	{
		if (Route.size() > 0)
		{
			Route.pop_front();
			if (!Route.empty())
				NSDDD.NextTarget = *(Route.begin());
		}

		target_dist = dist(NSDDD.NextTarget, info.self.xy_pos);
	}

	NSDDD.former_view_angle = info.self.view_angle;
	NSDDD.former_move_angle = info.self.move_angle;

	//move towards the target
	DiDiAngle move_angle(NSDDD.NextTarget.x, NSDDD.NextTarget.y);
	//outfile << "Target angle" << double(move_angle) << std::endl;
	if (info.self.move_cd == 2)
	{
		move (0.0, 0.0, NOMOVE);
	}
	else
	{
		move_angle = (double)move_angle - info.self.view_angle;
		DiDiAngle view_angle(double(move_angle) - 0.0);
		//DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
		//outfile << "move" << double(move_angle) << "\n\n";
		move(double(move_angle), double(view_angle), -1);
	}

	outfile.close();
	outposfile.close();

	return;
}