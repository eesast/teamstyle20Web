#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include<fstream>
#include<cmath>
#include<queue>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

#define PI 3.14159265
#define precision_angle (double)90/(double)50  //angle precision
#define precision_displacement mindisplacement/5.0  // currently-allowed minimum moving distance
#define self_radius  (VOCATION_DATA[info.self.vocation].radius+0.001+0.2*VOCATION_DATA[info.self.vocation].move)
#define selfx info.self.xy_pos.x
#define selfy info.self.xy_pos.y
#define allowed_pick_distance (18-frame/100)
std::ofstream out;


enum trymove
{
	notry,
	trymovex_p,
	trymovex_n,
	trymovey_p,
	trymovey_n,
	trymove_angle,
};
static trymove last_try;

enum impedecondition
{
	noimpede,
	ximpede,
	yimpede,
	xyimpede,
	circleimpede,
	roadaimpede,
	roadbimpede,
};
impedecondition ip = noimpede;

enum ACTION
{
	MOVE_ATTACK_ACTION = 0,
	USE_DRUG,
	PICK_UP,
	//...
};
ACTION next_action;

struct Move_instruction
{
	double move_angle;
	int move_frame;
};
static std::queue<Move_instruction>move_instruction;


static XYPosition my_last_position;
double mindisplacement = 0.2;
static XYPosition move_target;
static double move_angle;
Item closest_item;
static int ntryangle = 0;
double centre_angle;// 自身到障碍的连心角
static double enemy_last_angle = 400;
static int enemy_fighting_case1 = 0, enemy_fighting_case2 = 0;
static int enemy_fighting_mode = 0;
static Item drug_to_use;
double mod360(double angle)
{
	if (angle > 0 && angle < 360)return angle;
	while (angle < 0)angle += 360;
	while (angle > 360)angle -= 360;
	return angle;
}

double mod100(double position)
{
	while (position > 100)position -= 100;
	return position;
}

inline double distance(XYPosition a, XYPosition b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

bool inpoison()// be harmed by poison
{
	if (distance(info.self.xy_pos, info.poison.current_center) > info.poison.current_radius*0.95)return true;
	return false;
}

inline bool isstick()
{
	return fabs(distance(info.self.xy_pos, my_last_position)) < 0.19*VOCATION_DATA[info.self.vocation].move;
}

XYPosition Convertion(PolarPosition polar_pos)
{
	XYPosition xy_pos;
	xy_pos.x = selfx + polar_pos.distance*cos((polar_pos.angle + info.self.view_angle) / 180 * PI);
	xy_pos.y = selfy + polar_pos.distance*sin((polar_pos.angle + info.self.view_angle) / 180 * PI);
	return xy_pos;
}

PolarPosition Convertion(XYPosition xy_pos)
{
	PolarPosition polar_pos;
	polar_pos.distance = distance(info.self.xy_pos, xy_pos);
	if (xy_pos.x - selfx > 0)polar_pos.angle = mod360(atan2(xy_pos.y - selfy, xy_pos.x - selfx) / PI * 180);
	else polar_pos.angle = mod360(atan2(xy_pos.y - selfy, xy_pos.x - selfx) / PI * 180 + 180);
	return polar_pos;
}
void update_target()//finished
{
	move_target = Convertion(closest_item.polar_pos);
	if (frame > 200)
	{
		if (closest_item.item_ID == -1 || closest_item.polar_pos.distance > allowed_pick_distance)
		{
			if (inpoison() || info.poison.current_radius < 0.703)  move_target = info.poison.current_center;
			else move_target = info.poison.next_center;
		}
	}
	if (closest_item.item_ID == -1 && frame <= 200)move_target = info.poison.next_center;
	move_angle = Convertion(move_target).angle;
}

void find_closest_item()
{
	out << "item list\n";
	for (register int i = 0; i < info.items.size(); i++)out << info.items[i].type << ' ' << info.items[i].polar_pos.distance << ' ' << info.items[i].polar_pos.angle << std::endl;
	closest_item.polar_pos.distance = 0x3f3f3f3f;
	closest_item.item_ID = -1;
	if (frame <= 200)
	{
		if (!info.items.empty())
		{
			closest_item = info.items[0];
			return;
		}
	}
	else
	{
		for (int i = 0; i < info.items.size(); ++i)
		{
			double angle = fabs(info.self.move_angle - info.items[i].polar_pos.angle);
			if (angle < 60)
			{
				closest_item = info.items[i];
				return;
			}
		}
	}
}

void init()
{
	find_closest_item();

}

inline bool purpose_in_view(double angle)
{
	if (fabs(angle - info.self.view_angle) > VOCATION_DATA[info.self.vocation].angle / 2)return false;
	return true;
}

inline bool in_city()
{
	if (selfx >= 400 && selfx <= 600 && selfy >= 400 && selfy <= 600)return true;
	if (selfx >= 100 && selfx <= 200 && selfy >= 100 && selfy <= 200)return true;
	if (selfx >= 200 && selfx <= 300 && selfy >= 700 && selfy <= 800)return true;
	if (selfx >= 700 && selfx <= 800 && selfy >= 200 && selfy <= 300)return true;
	if (selfx >= 800 && selfx <= 900 && selfy >= 800 && selfy <= 900)return true;
	return false;
}

inline bool in_hill()
{
	if (selfx >= 400 && selfx <= 600 && selfy >= 300 && selfy <= 400)return true;
	if (selfx >= 200 && selfx <= 300 && selfy >= 400 && selfy <= 600)return true;
	if (selfx >= 400 && selfx <= 600 && selfy >= 700 && selfy <= 800)return true;
	if (selfx >= 800 && selfx <= 900 && selfy >= 400 && selfy <= 500)return true;
	if (selfx >= 700 && selfx <= 800 && selfy >= 600 && selfy <= 700)return true;
	return false;
}

void follow_move_instruction()
{
	Move_instruction new_move = move_instruction.front();
	move(mod360(new_move.move_angle - info.self.view_angle), mod360(new_move.move_angle - info.self.view_angle));
	move_instruction.front().move_frame--;
	if (move_instruction.front().move_frame == 0)move_instruction.pop();
	return;
}

bool impede(block B)
{
	int changed = 1;
	if (ip != noimpede)changed = 0;
	if (B.type == SHALLOW_WATER || B.type == RECTANGLE_GRASS || B.type == CIRCLE_GRASS)return false;
	if (B.shape == CIRCLE)
	{
		XYPosition tmp = { (double)(B.x0),(double)(B.y0) };
		out << "dist " << distance(tmp, info.self.xy_pos) << std::endl;
		if (distance(tmp, info.self.xy_pos) > B.r + self_radius)return false;
		else
		{
			ip = circleimpede;
			centre_angle = tmp.x - selfx > 0 ? mod360(atan((tmp.y - selfy) / (tmp.x - selfx)) * 180 / PI) : mod360(atan((tmp.y - selfy) / (tmp.x - selfx)) * 180 / PI + 180);
			out << "centre_angle " << centre_angle << std::endl;
			return true;
		}
	}
	if (B.shape == RECTANGLE)
	{
		XYPosition left_lower_angle = { B.x0,B.y1 };
		XYPosition right_lower_angle = { B.x1,B.y1 };
		XYPosition left_upper_angle = { B.x0,B.y0 };
		XYPosition right_upper_angle = { B.x1,B.y0 };
		if (B.type == WALL && ((selfx > 305 && selfx < 401 && selfy>305 && selfy < 401) || (selfx > 605 && selfx < 701 && selfy>605 && selfy < 701)))
		{
			out << "ROADA" << std::endl;
			ip = roadaimpede;
			return true;
		}
		else if (B.type == WALL && ((selfx > 599 && selfx < 701 && selfy>299 && selfy < 401) || (selfx > 299 && selfx < 401 && selfy>605 && selfy < 701)))
		{
			out << "ROADB" << std::endl;
			ip = roadbimpede;
			return true;
		}
		else
		{
			if (selfx > left_lower_angle.x - self_radius + 0.2*VOCATION_DATA[info.self.vocation].move&& selfx < right_lower_angle.x + self_radius - 0.2*VOCATION_DATA[info.self.vocation].move)
			{
				out << "self " << selfx << ' ' << selfy << ' ' << self_radius << ' ' << left_lower_angle.x << std::endl;
				if (fabs(selfy - left_upper_angle.y) < self_radius || fabs(selfy - left_lower_angle.y) < self_radius)
				{
					if (ip == yimpede || ip == noimpede)ip = yimpede;
					else ip = circleimpede;
					changed = 1;
				}
			}
			if (selfy > left_lower_angle.y - self_radius + 0.2*VOCATION_DATA[info.self.vocation].move&& selfy < right_upper_angle.y + self_radius - 0.2*VOCATION_DATA[info.self.vocation].move)
			{
				if (fabs(selfx - left_upper_angle.x) < self_radius || fabs(selfx - right_lower_angle.x) < self_radius)
				{
					if (ip == noimpede)
					{
						ip = ximpede;
						changed = 1;
					}
					else
					{
						ip = circleimpede;
						changed = 1;
						XYPosition tmp;
						if (fabs(selfy - left_upper_angle.y) < self_radius&& fabs(left_upper_angle.x - selfx) < self_radius)tmp = left_upper_angle;
						if (fabs(selfy - right_upper_angle.y) < self_radius&& fabs(right_upper_angle.x - selfx) < self_radius)tmp = right_upper_angle;
						if (fabs(selfy - left_lower_angle.y) < self_radius&& fabs(left_lower_angle.x - selfx) < self_radius)tmp = left_lower_angle;
						if (fabs(selfy - right_lower_angle.y) < self_radius&& fabs(right_lower_angle.x - selfx) < self_radius)tmp = right_lower_angle;
						centre_angle = tmp.x - selfx > 0 ? mod360(atan((tmp.y - selfy) / (tmp.x - selfx))) : mod360(atan((tmp.y - selfy) / (tmp.x - selfx)) + 180);
						out << "centre_angle" << centre_angle << std::endl;
					}
				}
			}
			if (distance(info.self.xy_pos, left_lower_angle) < self_radius || distance(info.self.xy_pos, right_lower_angle) < self_radius || distance(info.self.xy_pos, left_upper_angle) < self_radius || distance(info.self.xy_pos, right_upper_angle) < self_radius)
			{
				if (ip != ximpede && ip != yimpede)
				{
					ip = circleimpede;
					changed = 1;
				}
			}
		}
		if (changed == 0)return false;
		if (ip != noimpede)return true;
		return false;
	}
}

void my_move()// run to centre of next poison circle
{
	XYPosition delta = { move_target.x - selfx,move_target.y - selfy };
	if (!isstick())
	{
		last_try = notry;
		ntryangle = 0;
	}
	out << "move_angle " << move_angle << std::endl;
	out << "view_angle " << info.self.view_angle << std::endl;
	if (!purpose_in_view(move_angle) && last_try == notry)
	{
		move(mod360(move_angle - info.self.view_angle), mod360(move_angle - info.self.view_angle));
		out << "turn to poison" << std::endl;
		return;
	}
	if (isstick() && last_try == trymove_angle)
	{
		move(mod360(ntryangle * 30 + rand() % 30), mod360(ntryangle * 30 + rand() % 30));
		last_try = trymove_angle;
		return;
	}
	if (info.landform_IDs.size() <= 100)
	{
		out << "Landforms [\n";
		for (register int i = 0; i < info.landform_IDs.size(); i++)
		{
			out << get_landform(info.landform_IDs[i]).shape << ' '
				<< get_landform(info.landform_IDs[i]).type << ' '
				<< get_landform(info.landform_IDs[i]).r << ' '
				<< get_landform(info.landform_IDs[i]).x0 << ' '
				<< get_landform(info.landform_IDs[i]).y0 << ' '
				<< get_landform(info.landform_IDs[i]).x1 << ' '
				<< get_landform(info.landform_IDs[i]).y1 << std::endl;
		}
		int nimpede = 0;
		for (register int i = 0; i < info.landform_IDs.size(); i++)
		{
			block now_block = get_landform(info.landform_IDs[i]);
			if (impede(now_block))
			{
				out << "impede: "
					<< get_landform(info.landform_IDs[i]).shape << ' '
					<< get_landform(info.landform_IDs[i]).type << ' '
					<< get_landform(info.landform_IDs[i]).r << ' '
					<< get_landform(info.landform_IDs[i]).x0 << ' '
					<< get_landform(info.landform_IDs[i]).y0 << ' '
					<< get_landform(info.landform_IDs[i]).x1 << ' '
					<< get_landform(info.landform_IDs[i]).y1 << ' '
					<< ip << std::endl;
				nimpede++;
			}
		}
		if (nimpede == 0)
		{
			out << "find no impede" << std::endl;
			if (isstick())
			{
				out << " stick!" << std::endl;
				out << "last_try " << last_try << std::endl;
				if (last_try == notry)
				{
					if (delta.y > 0)
					{
						move(mod360(90 - info.self.view_angle), mod360(90 - info.self.view_angle));
						out << "trymovey_p" << std::endl;
						last_try = trymovey_p;
					}
					else
					{
						move(mod360(-90 - info.self.view_angle), mod360(-90 - info.self.view_angle));
						last_try = trymovey_n;
						out << "trymovey_n" << std::endl;
					}
				}
				else if (last_try == trymovey_p || last_try == trymovey_n)
				{
					if (delta.x > 0)
					{
						move(mod360(0 - info.self.view_angle), mod360(0 - info.self.view_angle));
						last_try = trymovex_p;
						out << "trymovex_p" << std::endl;
					}
					else
					{
						move(mod360(180 - info.self.view_angle), mod360(180 - info.self.view_angle));
						last_try = trymovex_n;
						out << "trymovex_n" << std::endl;
					}
				}
				else
				{
					last_try = trymove_angle;
					ntryangle++;
					move(mod360(ntryangle * 30 + rand() % 30), mod360(ntryangle * 30 + rand() % 30));
					out << "trymove_angle" << mod360(ntryangle * 30 + rand() % 30) << std::endl;
				}
			}
			else
			{
				move(mod360(move_angle - info.self.view_angle), mod360(move_angle - info.self.view_angle));
				last_try = notry;
				out << "move to target" << mod360(move_angle - info.self.view_angle) << std::endl;
			}
		}
		else
		{
			if (!isstick())last_try = notry;
			out << "nimpede: " << nimpede << std::endl;
			out << "impedecondition " << ip << std::endl;
			if (ip == ximpede)
			{
				if (delta.y > 0)
				{
					move(mod360(90 - info.self.view_angle), mod360(90 - info.self.view_angle));
					out << "move up" << std::endl;
				}
				else
				{
					move(mod360(-90 - info.self.view_angle), mod360(-90 - info.self.view_angle));
					out << "move down" << std::endl;
				}
			}
			if (ip == yimpede)
			{
				if (delta.x > 0)
				{
					out << "move right" << std::endl;
					move(mod360(0 - info.self.view_angle), mod360(0 - info.self.view_angle));
				}
				else
				{
					out << "move left" << std::endl;
					move(mod360(180 - info.self.view_angle), mod360(180 - info.self.view_angle));
				}
			}
			if (ip == circleimpede)
			{
				if (mod360(move_angle - centre_angle) < 180)
				{
					move(mod360(centre_angle - info.self.view_angle + 91), mod360(centre_angle - info.self.view_angle + 91));
					out << "moveangle:" << mod360(centre_angle - info.self.view_angle + 91) << std::endl;
				}
				else
				{
					move(mod360(centre_angle - info.self.view_angle - 91), mod360(centre_angle - info.self.view_angle - 91));
					out << "moveangle:" << mod360(centre_angle - info.self.view_angle - 91) << std::endl;
				}
				last_try = trymove_angle;
			}
			if (ip == roadaimpede)
			{
				if (fabs(45 - move_angle) > 90)move(mod360(225 - info.self.view_angle), mod360(225 - info.self.view_angle));
				else move(mod360(45 - info.self.view_angle), mod360(45 - info.self.view_angle));
				last_try = trymove_angle;
			}
			if (ip == roadbimpede)
			{
				if (fabs(135 - move_angle) > 90)move(mod360(315 - info.self.view_angle), mod360(315 - info.self.view_angle));
				else move(mod360(135 - info.self.view_angle), mod360(135 - info.self.view_angle));
				last_try = trymove_angle;
			}
		}
	}
	else
	{
		out << "nlandforms: " << info.landform_IDs.size() << std::endl;
	}
	//if (!isstick())
	//{
	//	if (last_try == trymovex_n)
	//	{
	//		move_instruction.push({ 180,3 });
	//		follow_move_instruction();
	//	}
	//	if (last_try == trymovex_p)
	//	{
	//		move_instruction.push({ 0,3 });
	//		follow_move_instruction();
	//	}
	//	if (last_try == trymovey_p)
	//	{
	//		move_instruction.push({ 90,3 });
	//		follow_move_instruction();
	//	}
	//	if (last_try == trymovey_n)
	//	{
	//		move_instruction.push({ 270,3 });
	//		follow_move_instruction();
	//	}
	//	last_try = notry;
	//	ntryangle = 0;
	//	return;
	//}
	//if (isstick())
	//{
	//	while (!move_instruction.empty()) move_instruction.pop();
	//}
	//if (in_city())
	//{
	//	out << "move in city" << std::endl;
	//}
	//if (!move_instruction.empty())
	//{
	//	follow_move_instruction();
	//	out << "follow move instruction" << std::endl;
	//	return;
	//}
	//if (!purpose_in_view(move_angle) && last_try == notry)
	//{
	//	move(mod360(move_angle - info.self.view_angle), mod360(move_angle - info.self.view_angle));
	//	out << "turn to poison" << std::endl;
	//	return;
	//}
	//if (isstick() && last_try == trymove_angle)
	//{
	//	move(mod360(ntryangle * 30 + rand() % 30), mod360(ntryangle * 30 + rand() % 30));
	//	last_try = trymove_angle;
	//	return;
	//}
	//if (in_city() && mod100(selfx) > 5 && mod100(selfx) < 7 && mod100(selfy) > 5 && mod100(selfy) < 7)
	//{
	//	if (delta.y > 0)
	//	{
	//		move_instruction.push({ 90,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 180,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//	else
	//	{
	//		move_instruction.push({ 0,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 270,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//}
	//if (in_city() && mod100(selfx) > 93 && mod100(selfx) < 95 && mod100(selfy) > 5 && mod100(selfy) < 7)
	//{
	//	if (delta.y > 0)
	//	{
	//		move_instruction.push({ 90,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 0,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//	else
	//	{
	//		move_instruction.push({ 180,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 270,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//}
	//if (in_city() && mod100(selfx) > 5 && mod100(selfx) < 7 && mod100(selfy) > 93 && mod100(selfy) < 95)
	//{
	//	if (delta.y < 0)
	//	{
	//		move_instruction.push({ 270,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 180,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//	else
	//	{
	//		move_instruction.push({ 0,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 90,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//}
	//if (in_city() && mod100(selfx) > 93 && mod100(selfx) < 95 && mod100(selfy) > 93 && mod100(selfy) < 95)
	//{
	//	if (delta.y < 0)
	//	{
	//		move_instruction.push({ 270,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 0,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//	else
	//	{
	//		move_instruction.push({ 180,(int)(32 / VOCATION_DATA[info.self.vocation].move) });
	//		move_instruction.push({ 90,2 });
	//		follow_move_instruction();
	//		return;
	//	}
	//}
	//if (info.landform_IDs.size() <= 100)
	//{
	//	out << "Landforms [\n";
	//	for (register int i = 0; i < info.landform_IDs.size(); i++)
	//	{
	//		out << get_landform(info.landform_IDs[i]).shape << ' '
	//			<< get_landform(info.landform_IDs[i]).type << ' '
	//			<< get_landform(info.landform_IDs[i]).r << ' '
	//			<< get_landform(info.landform_IDs[i]).x0 << ' '
	//			<< get_landform(info.landform_IDs[i]).y0 << ' '
	//			<< get_landform(info.landform_IDs[i]).x1 << ' '
	//			<< get_landform(info.landform_IDs[i]).y1 << std::endl;
	//	}
	//	int nimpede = 0;
	//	for (register int i = 0; i < info.landform_IDs.size(); i++)
	//	{
	//		block now_block = get_landform(info.landform_IDs[i]);
	//		if (impede(now_block))
	//		{
	//			out << "impede: "
	//				<< get_landform(info.landform_IDs[i]).shape << ' '
	//				<< get_landform(info.landform_IDs[i]).type << ' '
	//				<< get_landform(info.landform_IDs[i]).r << ' '
	//				<< get_landform(info.landform_IDs[i]).x0 << ' '
	//				<< get_landform(info.landform_IDs[i]).y0 << ' '
	//				<< get_landform(info.landform_IDs[i]).x1 << ' '
	//				<< get_landform(info.landform_IDs[i]).y1 << ' '
	//				<< ip << std::endl;
	//			nimpede++;
	//		}
	//	}
	//	if (nimpede == 0)
	//	{
	//		out << "find no impede" << std::endl;
	//		if (isstick())
	//		{
	//			out << " stick!" << std::endl;
	//			out << "last_try " << last_try << std::endl;
	//			if (last_try == notry)
	//			{
	//				if (delta.y > 0)
	//				{
	//					move(mod360(90 - info.self.view_angle), mod360(90 - info.self.view_angle));
	//					out << "trymovey_p" << std::endl;
	//					last_try = trymovey_p;
	//				}
	//				else
	//				{
	//					move(mod360(-90 - info.self.view_angle), mod360(-90 - info.self.view_angle));
	//					last_try = trymovey_n;
	//					out << "trymovey_n" << std::endl;
	//				}
	//			}
	//			else if (last_try == trymovey_p || last_try == trymovey_n)
	//			{
	//				if (delta.x > 0)
	//				{
	//					move(mod360(0 - info.self.view_angle), mod360(0 - info.self.view_angle));
	//					last_try = trymovex_p;
	//					out << "trymovex_p" << std::endl;
	//				}
	//				else
	//				{
	//					move(mod360(180 - info.self.view_angle), mod360(180 - info.self.view_angle));
	//					last_try = trymovex_n;
	//					out << "trymovex_n" << std::endl;
	//				}
	//			}
	//			else
	//			{
	//				last_try = trymove_angle;
	//				ntryangle++;
	//				move(mod360(ntryangle * 30 + rand() % 30), mod360(ntryangle * 30 + rand() % 30));
	//				out << "trymove_angle" << mod360(ntryangle * 30 + rand() % 30) << std::endl;
	//			}
	//		}
	//		else
	//		{
	//			move(mod360(move_angle - info.self.view_angle), mod360(move_angle - info.self.view_angle));
	//			last_try = notry;
	//			out << "move to target" << mod360(move_angle - info.self.view_angle) << std::endl;
	//		}
	//	}
	//	else
	//	{
	//		if (!isstick())last_try = notry;
	//		out << "nimpede: " << nimpede << std::endl;
	//		out << "impedecondition " << ip << std::endl;
	//		if (ip == ximpede)
	//		{
	//			if (in_city() && fabs(delta.y) < 1 * VOCATION_DATA[info.self.vocation].move&&fabs(delta.x) > 10)
	//			{
	//				if (mod100(selfy) < 20.001 && mod100(selfy) > 3)
	//				{
	//					Move_instruction new_move = { 270,(mod100(selfy) - 3) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.x > 0)move_instruction.push({ 0,3 });
	//					else move_instruction.push({ 180,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfy) < 37 && mod100(selfy) > 19.999)
	//				{
	//					Move_instruction new_move = { 90,(37 - mod100(selfy)) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.x > 0)move_instruction.push({ 0,3 });
	//					else move_instruction.push({ 180,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfy) > 63 && mod100(selfy) < 80.001)
	//				{
	//					Move_instruction new_move = { 270,(mod100(selfy) - 63) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.x > 0)move_instruction.push({ 0,3 });
	//					else move_instruction.push({ 180,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfy) < 97 && mod100(selfy) > 79.999)
	//				{
	//					Move_instruction new_move = { 90,(97 - mod100(selfy)) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.x > 0)move_instruction.push({ 0,3 });
	//					else move_instruction.push({ 180,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//			}
	//			/*if (in_hill()&&fabs(delta.y)<1* VOCATION_DATA[info.self.vocation].move&&fabs(delta.x)>8)
	//			{
	//				if (selfx >= 4 && selfx <= 26 && selfy >= 9 && selfy <= 31)
	//				{

	//				}
	//				if (selfx >= 39 && selfx <= 61 && selfy >= 19 && selfy <= 46)
	//				{

	//				}
	//				if (selfx >= 69&&selfx <=91 &&selfy >=59 &&selfy <=91 )
	//				{

	//				}
	//				if (selfx >=9 &&selfx <=41 &&selfy >=69 &&selfy <=91 )
	//				{

	//				}
	//				if (selfx >=79 &&selfx <=91 &&selfy >= 9&&selfy <=51 )
	//				{

	//				}
	//			}
	//			*/
	//			if (delta.y > 0)
	//			{
	//				move(mod360(90 - info.self.view_angle), mod360(90 - info.self.view_angle));
	//				out << "move up" << std::endl;
	//			}
	//			else
	//			{
	//				move(mod360(-90 - info.self.view_angle), mod360(-90 - info.self.view_angle));
	//				out << "move down" << std::endl;
	//			}
	//		}
	//		if (ip == yimpede)
	//		{
	//			if (in_city() && fabs(delta.x) < 1 * VOCATION_DATA[info.self.vocation].move&&fabs(delta.x) > 10)
	//			{
	//				if (mod100(selfx) < 20.001 && mod100(selfx) > 3)
	//				{
	//					Move_instruction new_move = { 180,(mod100(selfx) - 3) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.y > 0)move_instruction.push({ 90,3 });
	//					else move_instruction.push({ 270,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfx) < 37 && mod100(selfx) > 19.999)
	//				{
	//					Move_instruction new_move = { 0,(37 - mod100(selfx)) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.y > 0)move_instruction.push({ 90,3 });
	//					else move_instruction.push({ 270,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfx) > 37 && mod100(selfx) < 50)
	//				{
	//					Move_instruction new_move = { 180,(mod100(selfx) - 37) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.y > 0)move_instruction.push({ 90,3 });
	//					else move_instruction.push({ 270,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfx) > 50 && mod100(selfx) < 63)
	//				{
	//					Move_instruction new_move = { 180,(63 - mod100(selfx)) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.y > 0)move_instruction.push({ 90,3 });
	//					else move_instruction.push({ 270,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfx) > 63 && mod100(selfx) < 80)
	//				{
	//					Move_instruction new_move = { 180,(mod100(selfx) - 63) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.y > 0)move_instruction.push({ 90,3 });
	//					else move_instruction.push({ 270,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//				if (mod100(selfx) < 97 && mod100(selfx) > 79.999)
	//				{
	//					Move_instruction new_move = { 0,(97 - mod100(selfx)) / VOCATION_DATA[info.self.vocation].move };
	//					move_instruction.push(new_move);
	//					if (delta.y > 0)move_instruction.push({ 90,3 });
	//					else move_instruction.push({ 270,3 });
	//					follow_move_instruction();
	//					return;
	//				}
	//			}
	//			if (delta.x > 0)
	//			{
	//				out << "move right" << std::endl;
	//				move(mod360(0 - info.self.view_angle), mod360(0 - info.self.view_angle));
	//			}
	//			else
	//			{
	//				out << "move left" << std::endl;
	//				move(mod360(180 - info.self.view_angle), mod360(180 - info.self.view_angle));
	//			}
	//		}
	//		if (ip == circleimpede)
	//		{
	//			if (mod360(move_angle - centre_angle) < 180)
	//			{
	//				move(mod360(centre_angle - info.self.view_angle + 91), mod360(centre_angle - info.self.view_angle + 91));
	//				out << "moveangle:" << mod360(centre_angle - info.self.view_angle + 91) << std::endl;
	//			}
	//			else
	//			{
	//				move(mod360(centre_angle - info.self.view_angle - 91), mod360(centre_angle - info.self.view_angle - 91));
	//				out << "moveangle:" << mod360(centre_angle - info.self.view_angle - 91) << std::endl;
	//			}
	//			last_try = trymove_angle;
	//		}
	//		if (ip == roadaimpede)
	//		{
	//			if (fabs(45 - move_angle) > 90)move(mod360(225 - info.self.view_angle), mod360(225 - info.self.view_angle));
	//			else move(mod360(45 - info.self.view_angle), mod360(45 - info.self.view_angle));
	//			last_try = trymove_angle;
	//		}
	//		if (ip == roadbimpede)
	//		{
	//			if (fabs(135 - move_angle) > 90)move(mod360(315 - info.self.view_angle), mod360(315 - info.self.view_angle));
	//			else move(mod360(135 - info.self.view_angle), mod360(135 - info.self.view_angle));
	//			last_try = trymove_angle;
	//		}
	//	}
	//}
	//else
	//{
	//	out << "nlandforms: " << info.landform_IDs.size() << std::endl;
	//}
}

bool if_to_use_drug()
{
	double damage_hp = info.self.hp_limit - info.self.hp;
	if (damage_hp < 20.00)
		return false;
	else if (info.self.hp > 42)
	{
		int flag = 0;
		for (int i = 0; i < info.self.bag.size(); i++)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 15)
				drug_to_use = info.self.bag[i];
			flag++;
		}
		return flag;
	}
	else
	{
		int flag = 0;
		for (int i = 0; i < info.self.bag.size(); i++)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 15)
				drug_to_use = info.self.bag[i];
			flag++;
		}
		for (int i = 0; i < info.self.bag.size(); i++)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 16)
				drug_to_use = info.self.bag[i];
			flag++;
		}
		return flag;
	}
}

void use_drug()
{
	shoot(drug_to_use.type, 34);
}

ACTION judge_action()
{
	if (if_to_use_drug())
	{
		return USE_DRUG;
	}
	if (closest_item.item_ID != -1 && closest_item.polar_pos.distance < 1 - mindisplacement)
	{
		return PICK_UP;
	}
	return MOVE_ATTACK_ACTION;
}


void MyMove()
{
	update_target();
	my_move();
}

void my_pick()
{
	pickup(closest_item.item_ID);
}

int seen_by_others(OtherInfo others)
{
	double angle_range = VOCATION_DATA[others.vocation].angle / 2;
	double enemy_angle_position;
	enemy_angle_position = mod360(mod360(mod360(info.self.view_angle + others.polar_pos.angle) + 180) - others.view_angle);
	if (others.polar_pos.distance <= VOCATION_DATA[others.vocation].distance)
	{
		if (enemy_angle_position <= angle_range || enemy_angle_position >= 360 - angle_range)
			return 1;
		else return 0;
	}
	return 0;
}

int is_enemy(int i)
{
	int flag = 1;
	for (int j = 0; j < teammates.size(); j++)
	{
		if (info.others[i].player_ID == teammates[j])
		{
			flag = 0;
			break;
		}
	}
	return flag;
}

XYPosition next_position(XYPosition position, double move_angle, double length)
{
	XYPosition next_position;
	next_position.x = position.x + cos(move_angle)*length;
	next_position.y = position.y + sin(move_angle)*length;
	return next_position;
}

double work_out_angle(XYPosition position)
{
	double delta_x, delta_y;
	delta_x = position.x - info.self.xy_pos.x;
	delta_y = position.y - info.self.xy_pos.y;
	double angle;
	angle = (atan(delta_y / delta_x)) / PI * 180;
	return mod360(angle);
}

int vertical_judge(double angle1, double angle2)
{

	double angle;
	angle = mod360(angle1 - angle2);
	if ((angle >= 60 && angle <= 120) || (angle >= 240 && angle <= 270))
		return 1;
	else
		return 0;

}

void my_attack()
{
	OtherInfo closest_enemy;
	closest_enemy.polar_pos.distance = 100000;
	int n_enemies = 0;
	for (int i = 0; i < info.others.size(); i++)
	{
		if (is_enemy(i))
		{
			n_enemies++;
			if (info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
			{
				closest_enemy = info.others[i];
			}
		}
	}
	if (!n_enemies)return;
	if (!seen_by_others(closest_enemy))
	{
		if (closest_enemy.polar_pos.distance > 100)//100是脚步声传播距离。若此时跑路一般不会被发现，可以对情况进行判断
		{

		}
		else//必须打了，跑会被发现
		{
			//equipped
			ITEM weapon;
			int flag = 0;
			int  weapon_order[10] = { 7,6,5,4,9,3,2,1,8,0 };
			for (int i = 0; i < 9; i++)
			{
				for (int j = 0; j < info.self.bag.size(); j++)
				{
					if (ITEM_DATA[info.self.bag[j].type].number == weapon_order[i] && info.self.bag[j].durability > 0)
					{
						weapon = info.self.bag[j].type;
						flag = 1;
						break;
					}
				}
				if (flag)
					break;
			}

			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)//射程不够
			{
				//后续添加
			}
			else
			{
				double shoot_angle;
				XYPosition position, nextposition;
				position = Convertion(closest_enemy.polar_pos);
				nextposition = next_position(position, closest_enemy.move_angle, 0.2);
				shoot_angle = work_out_angle(nextposition);
				if (info.self.attack_cd == 0)
					shoot(weapon, shoot_angle);
			}
		}
	}
	else//被看见了，钢枪模式启动
	{
		//move
		//
		//attack
		if (enemy_last_angle < 360.01)
		{
			double delta_angle;
			int fighting_case;
			delta_angle = closest_enemy.move_angle - enemy_last_angle;
			if (delta_angle <= 30 && delta_angle >= -30)
				fighting_case = 1;
			else if (vertical_judge(mod360(closest_enemy.polar_pos.angle + info.self.view_angle), closest_enemy.move_angle))
			{
				if (delta_angle <= 90 && delta_angle >= -90)
					fighting_case = 2;
				else
					fighting_case = 3;
			}
			else
				fighting_case = 4;
			enemy_last_angle = closest_enemy.move_angle;

			if (enemy_fighting_case1 == 0)
				enemy_fighting_case1 = fighting_case;
			else
			{
				if (enemy_fighting_case2 == 0)
					enemy_fighting_case2 = fighting_case;
				else
				{
					enemy_fighting_case1 = enemy_fighting_case2;
					enemy_fighting_case2 = fighting_case;
				}
			}
			if (enemy_fighting_case2 != 0)
			{
				if (enemy_fighting_case1 == 1 || enemy_fighting_case1 == 2)
				{
					if (enemy_fighting_case2 == 1 || enemy_fighting_case2 == 2)
						enemy_fighting_mode = 1;
					if (enemy_fighting_case2 == 3 || enemy_fighting_case2 == 4)
						enemy_fighting_mode = 2;
				}
				else
					enemy_fighting_mode = 2;
			}
			ITEM weapon;
			int flag = 0;
			if (enemy_fighting_mode != 0)
			{

				if (enemy_fighting_mode == 1)
				{
					int  weapon_order[10] = { 7,6,5,4,9,3,2,1,8,0 };//大威力优先
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < info.self.bag.size(); j++)
						{
							if (ITEM_DATA[info.self.bag[j].type].number == weapon_order[i] && info.self.bag[j].durability > 0)
							{
								weapon = info.self.bag[j].type;
								flag = 1;
								break;
							}
						}
						if (flag)
							break;
					}
					if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)//射程不够
					{
						//后续添加
					}
					else
					{
						double shoot_angle;
						XYPosition position, nextposition;
						position = Convertion(closest_enemy.polar_pos);
						nextposition = next_position(position, closest_enemy.move_angle, 0.2);
						shoot_angle = work_out_angle(nextposition);
						if (info.self.attack_cd == 0)
							shoot(weapon, shoot_angle);
					}
				}
				if (enemy_fighting_mode == 2)
				{
					int  weapon_order[10] = { 5,4,2,3,9,6,7,1,8,0 };//短cd优先
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < info.self.bag.size(); j++)
						{
							if (ITEM_DATA[info.self.bag[j].type].number == weapon_order[i] && info.self.bag[j].durability > 0)
							{
								weapon = info.self.bag[j].type;
								flag = 1;
								break;
							}
						}
						if (flag)
							break;
					}
					if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)//射程不够
					{
						//后续添加
					}
					else
					{
						if (info.self.attack_cd == 0)
							shoot(weapon, closest_enemy.polar_pos.angle);
					}
				}

			}
			else
			{
				int  weapon_order[10] = { 5,4,2,3,9,6,7,1,8,0 };//短cd优先
				for (int i = 0; i < 9; i++)
				{
					for (int j = 0; j < info.self.bag.size(); j++)
					{
						if (ITEM_DATA[info.self.bag[j].type].number == i && info.self.bag[j].durability > 0)
						{
							weapon = info.self.bag[j].type;
							flag = 1;
							break;
						}
					}
					if (flag)
						break;
				}
				if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)//射程不够
				{
					//后续添加
				}
				else
				{
					double shoot_angle;
					XYPosition position, nextposition;
					position = Convertion(closest_enemy.polar_pos);
					nextposition = next_position(position, closest_enemy.move_angle, 0.2);
					shoot_angle = work_out_angle(nextposition);
					if (info.self.attack_cd == 0)
						shoot(weapon, shoot_angle);
				}


			}
		}
		else
			enemy_last_angle = closest_enemy.move_angle;
	}
	if (closest_enemy.status == DEAD)
	{
		enemy_fighting_case1 = 0;
		enemy_fighting_case2 = 0;
		enemy_fighting_mode = 0;
		enemy_last_angle = 400;
	}

}


void play_game()
{
	update_info();
	out.open("test.txt", std::ios_base::app);
	out << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
	out << "XYposition: " << selfx << ' ' << selfy << std::endl;
	out << "Poisoninfo " << info.poison.current_center.x << ' ' << info.poison.current_center.y << ' ' << info.poison.current_radius << std::endl;
	ip = noimpede;
	init();
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 620,620 };
		parachute(HACK, landing_point);
		out.close();
		my_last_position = info.self.xy_pos;
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		out << "jumping" << std::endl;
		out.close();
		my_last_position = info.self.xy_pos;
		return;
	}
	next_action = judge_action();
	switch (next_action)
	{
	case MOVE_ATTACK_ACTION:
		MyMove();
		my_attack();
		break;
	case USE_DRUG:use_drug(); break;
	case PICK_UP:my_pick(); break;
	}
	my_last_position = info.self.xy_pos;
	out << std::endl;
	out.close();
	return;
}
