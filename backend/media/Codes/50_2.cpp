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
const double MINOBSTACLE = 1;
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

} NSDDD;

class DiDiAngle
{
public:
	DiDiAngle() { angle = 0; };
	DiDiAngle(double a);
	DiDiAngle(double x, double y);
	DiDiAngle operator+(DiDiAngle& a2);
	DiDiAngle operator-(DiDiAngle& a2);
	DiDiAngle operator=(double a);
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

DiDiAngle DiDiAngle::operator+(DiDiAngle& a2)
{
	return DiDiAngle(this->angle+a2.angle);
}

DiDiAngle DiDiAngle::operator-(DiDiAngle& a2)
{
	return DiDiAngle(this->angle - a2.angle);
}

DiDiAngle DiDiAngle::operator=(double a)
{
	return DiDiAngle(a);
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
	if (myself.status == SHOOTING) return;
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

/*Absolute move angle and absolute view angle
Return absolute move angle and view angle*/
double AttemptMove(double move_angle, double view_angle, int parameter = -1)
{
	//relative angle
	DiDiAngle move_angle_r(move_angle - info.self.view_angle);
	DiDiAngle view_angle_r(view_angle - info.self.view_angle);

	if (!Overlap(double(move_angle)))
	{
		move_angle_r = double(move_angle) - info.self.view_angle;
		view_angle_r = double(view_angle) - info.self.view_angle;
		move(double(move_angle_r), double(move_angle_r), parameter);
		return move_angle;
	}
	else
	{
		for (int i = 0; i < ObstacleAngle.size(); ++i)
		{
			if (ObstacleAngle[i].Start < double(move_angle)
				&& ObstacleAngle[i].End > double(move_angle))
			{
				if (ObstacleAngle[i].Start > -1e-6
					&& ObstacleAngle[i].Start < 1e-6)
				{
					move_angle_r = ObstacleAngle[i].End + 45 - info.self.view_angle;
					view_angle_r = double(move_angle_r) + info.self.view_width / LookAround;
					move(double(move_angle_r), double(move_angle_r), -1);
					return ObstacleAngle[i].End + 45;
				}
				else if (ObstacleAngle[i].End - 360 > -1e-6
						&& ObstacleAngle[i].End - 360 < 1e-6)
				{
					move_angle_r = ObstacleAngle[i].Start - 45 - info.self.view_angle;
					view_angle_r = double(move_angle_r) - info.self.view_width / LookAround;
					move(double(move_angle_r), double(move_angle_r), -1);
					return ObstacleAngle[i].Start - 45;
				}
				else
				{
					if (2 * double(move_angle) < ObstacleAngle[i].Start + ObstacleAngle[i].End)
					{
						move_angle_r = ObstacleAngle[i].Start - 45 - info.self.view_angle;
						view_angle_r = double(move_angle_r) - info.self.view_width / LookAround;
						move(double(move_angle_r), double(move_angle_r), -1);
						return ObstacleAngle[i].Start - 45;
					}
					else
					{
						move_angle_r = ObstacleAngle[i].End + 45 - info.self.view_angle;
						view_angle_r = double(move_angle_r) + info.self.view_width / LookAround;
						move(double(move_angle_r), double(move_angle_r), -1);
						return ObstacleAngle[i].End + 45;
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
	std::ofstream outfile("Player_0_0.txt", std::ios::out | std::ios::app);
	std::ofstream outposfile("PlayerPos_0_0.txt", std::ios::out | std::ios::app);

	//have moved or not
	bool moved = false;

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
		outfile << "Package filled.\n\n" << std::endl;
	if (frame == 0)
	{
		srand(3);
		XYPosition landing_point = { 350 + rand() % 50, 350 + rand() % 50 };
		parachute(HACK, landing_point);
		NSDDD.Status = XIAZOU;
		NSDDD.Paodu = false;
		NSDDD.Target = { 500,500 };
		return;
	}
	else
	{
		srand(time(nullptr));
	}

	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		outfile << "Jumping.\n\n" << std::endl;
		return;
	}

	//shouxinxi

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
			if (x < 0 || x>9 || y < 0 || y>9) break;
			landno = y * 10 + x;
			areatype = MAP[landno];
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
						DiDiAngle span(asin(currentblock.r / (mindist + currentblock.r)) * 180 / PI);
						minangle = double(currentpoint - span);
						maxangle = double(currentpoint + span);

						if (info.self.xy_pos.x > double(currentblock.x0) - double(currentblock.r)
							&& info.self.xy_pos.x < double(currentblock.x0) + double(currentblock.r)
							&& info.self.xy_pos.y < currentblock.y0)
						{
							ObstacleAngle.push_back({ 0.0, minangle });
							ObstacleAngle.push_back({ maxangle, 360.0 });
							outfile << "Rectangle obstacle detected at (" << maxangle << ",360),(0," << minangle << ")\n";
						}
						else
						{
							ObstacleAngle.push_back({ minangle, maxangle });
							outfile << "Rectangle obstacle detected at (" << minangle << "," << maxangle << ")\n";
						}
					}
				}
				else if (currentblock.shape == RECTANGLE)
				{
					currentblock.x0 += x * 100;
					currentblock.x1 += x * 100;
					currentblock.y0 += y * 100;
					currentblock.y1 += y * 100;
					mindist = 10000;
					{
						if (currentblock.x0 - info.self.xy_pos.x > 0
							&& currentblock.x0 - info.self.xy_pos.x < mindist)
						{
							mindist = currentblock.x0 - info.self.xy_pos.x;
						}
						if (info.self.xy_pos.x - currentblock.x1 > 0
							&& info.self.xy_pos.x - currentblock.x1 < mindist)
						{
							mindist = info.self.xy_pos.x - currentblock.x1;
						}
						if (info.self.xy_pos.y - currentblock.y0 > 0
							&& info.self.xy_pos.y - currentblock.y0 < mindist)
						{
							mindist = info.self.xy_pos.y - currentblock.y0;
						}
						if (currentblock.y1 - info.self.xy_pos.y > 0
							&& currentblock.y1 - info.self.xy_pos.y < mindist)
						{
							mindist = currentblock.y1 - info.self.xy_pos.y;
						}
					}

					if (mindist <= MINOBSTACLE)
					{
						minangle = 360;
						maxangle = 0;
						{
							DiDiAngle currentpoint(currentblock.x0, currentblock.y0);
							if (double(currentpoint) < minangle) minangle = double(currentpoint);
							if (double(currentpoint) > maxangle) maxangle = double(currentpoint);

							currentpoint = DiDiAngle(currentblock.x1, currentblock.y0);
							if (double(currentpoint) < minangle) minangle = double(currentpoint);
							if (double(currentpoint) > maxangle) maxangle = double(currentpoint);

							currentpoint = DiDiAngle(currentblock.x0, currentblock.y1);
							if (double(currentpoint) < minangle) minangle = double(currentpoint);
							if (double(currentpoint) > maxangle) maxangle = double(currentpoint);

							currentpoint = DiDiAngle(currentblock.x1, currentblock.y1);
							if (double(currentpoint) < minangle) minangle = double(currentpoint);
							if (double(currentpoint) > maxangle) maxangle = double(currentpoint);
						}

						if (info.self.xy_pos.x > currentblock.x0
							&& info.self.xy_pos.x < currentblock.x1
							&& info.self.xy_pos.y < currentblock.y1)
						{
							ObstacleAngle.push_back({ 0.0, minangle });
							ObstacleAngle.push_back({ maxangle, 360.0 });
							outfile << "Rectangle obstacle detected at (" << maxangle << ",360),(0," << minangle << ")\n";
						}
						else
						{
							ObstacleAngle.push_back({ minangle, maxangle });
							outfile << "Rectangle obstacle detected at (" << minangle << "," << maxangle << ")\n";
						}
					}
				}
				else
					outfile << "!!Unknown block shape!\n";
			}
		}
	}
	outfile << "\n";

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
			moved = true;
		}
		else
		{
			NSDDD.Status = XIAZOU;
		}

		//Priority 2.5: Special items
		if (!moved)
		{

		}

		//Priority 3: Poison
		if (!moved && frame >= 200)
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
				moved = true;
			}
		}

		//Priority 4: Items
		if (!moved && !info.items.empty())
		{
			moved = true;

			Item closest_item;
			closest_item.polar_pos.distance = 100000;
			for (int i = 0; i < info.items.size(); ++i)
			{
				if (info.items[i].item_ID != CODE_CASE
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
				NSDDD.Status = MUBIAO;
			}
		}

		//Priority 5: Nothing happens
		if (!moved)
		{
			DiDiAngle move_angle;
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
		
			if (Overlap(double(move_angle)) || target_dist < 10)
			{
				do
				{
					move_angle = double(rand() % 360);
				} while (Overlap(double(move_angle)));

				SetTarget(info.self.xy_pos.x + 100 * cos(move_angle / 180 * PI),
					info.self.xy_pos.y + 100 * sin(move_angle / 180 * PI));
				move_angle = double(move_angle) - info.self.view_angle;
				DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
				move(double(move_angle), double(view_angle), -1);
				outfile << "move" << double(move_angle) << "\n\n";
			}
			else
			{
				move_angle = double(move_angle) - info.self.view_angle;
				if (info.self.move_cd == 2)
				{
					DiDiAngle view_angle(double(move_angle) - info.self.view_width / LookAround);
					move(double(move_angle), double(view_angle), NOMOVE);
					outfile << "Change view, move" << double(move_angle) << "\n\n";
				}
				else
				{
					DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
					move(double(move_angle), double(view_angle), -1);
					outfile << "Interrupt move, move" << double(move_angle) << "\n\n";
				}
			}
		}
		break;
	}
	case JIANBAO:
	{
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
		}
		else
		{
			NSDDD.Status = XIAZOU;
		}
		break;
	}
	case MUBIAO:
	{
		outfile << "Mubiao ing\n";
		DiDiAngle target(NSDDD.Target.x, NSDDD.Target.y);
		target = double(target) - info.self.view_angle;

		double target_dist = (NSDDD.Target.x - info.self.xy_pos.x) * (NSDDD.Target.x - info.self.xy_pos.x);
		target_dist += (NSDDD.Target.y - info.self.xy_pos.y) * (NSDDD.Target.y - info.self.xy_pos.y);
		target_dist = sqrt(target_dist);

		//Rediscover the target
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

		//If very close
		if (NSDDD.Status == MUBIAO && target_dist < 1)
		{
			NSDDD.Status = XIAZOU;
		}

		if (frame >= 200)
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
			DiDiAngle view_angle(double(move_angle) - info.self.view_width / LookAround);
			outfile << "move" << AttemptMove(double(move_angle), double(view_angle), NOMOVE) << "\n\n";
		}
		else
		{
			DiDiAngle view_angle(double(move_angle) + info.self.view_width / LookAround);
			outfile << "move" << AttemptMove(double(move_angle), double(view_angle), -1) << "\n\n";
		}
		break;
	}
	};

	outfile.close();
	outposfile.close();
	return;
}