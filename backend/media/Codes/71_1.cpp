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
	int medicman_ID=0;

	#define pi 3.141592653

	struct enemy_coordinate
	{
		double x, y;
		int last_seen_frame, ID;
	};

	double target_angle(const double &x, const double &y)
	{
		double diffx = x - info.self.xy_pos.x, diffy = y - info.self.xy_pos.y;
		double t = atan(diffy / diffx) * 2 * pi;
		double target_angle = diffx * diffy > 0 ? (diffx > 0 ? t : 180 + t) : (diffx > 0 ? 360 + t : 180 + t);
		while (target_angle < 0)
			target_angle += 360;
		while (target_angle >= 360)
			target_angle -= 360;
		return target_angle;
	}

	void moveto(XYPosition aim);
	void moveto(PolarPosition aim, int parameter = 1);
	void moveto(double aim_angle, int parameter = 1);
	double getdis_from_point_to_line(double x0, double y0, double x1, double y1, double xp, double yp);
	double getangle_from_point_to_line(double x0, double y0, double x1, double y1, double xp, double yp);
	double getdis_from_point_to_point(double x0, double y0, double x1, double y1);
	void Print_landform();
	int k = 0;

	static std::vector<enemy_coordinate> enemy_info;

	int weapon_cnt() {
		int cnt = 0;
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			int tmp = ITEM_DATA[info.self.bag[i].type].number;
			if (tmp == 6 || tmp == 7) cnt++;
		}
		return cnt;
	}

	bool armor_on() {
		return true;//no idea, 考虑防具的血量和等级
	}

	void new_pick_up();

	void play_game()
	{
		update_info();
		std::cout << "player:frame" << frame << "\nhp:" << info.self.hp << std::endl;
		std::cout << "positon" << info.self.xy_pos.x << ' ' << info.self.xy_pos.y << std::endl;
		if (info.self.bag.size() > 1)
			std::cout << "pick succeed" << std::endl;


		if (frame == 0)
		{
			srand(time(nullptr) + teammates[0]);
			XYPosition landing_point = { 51, 50 };
			parachute(HACK, landing_point);
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
				//此处开始发信息

		int32_t msg = 0;
		msg+=((info.self.hp)*10000000);
		msg+=((int32_t)info.self.vocation*1000000);
		msg += ((info.self.xy_pos.x) * 1000);
		msg += ((info.self.xy_pos.y));
		int r=0;
		for (; r < info.sounds.size(); ++r)
		{
			if (info.sounds[r].type == RADIO_VOICE) //条件：筛选出无线电信号。无线电信号只可能来自队友。
			{
				medicman_ID = info.sounds[r].sender;
			}
		}
		radio(medicman_ID, msg);


		if (info.others.empty())
		{
			std::cout << "no others" << std::endl;
			if (info.items.empty())
			{
				//see nothing
				if (info.self.status != MOVING)
				{
					double move_angle = 0;
					double view_angle = 30;//changed
					move(move_angle, view_angle);
					std::cout << "move" << move_angle << std::endl;
				}
			}
			else
			{
				new_pick_up();
			}
		}
		else
		{
			int num_enemy = 0;
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
				if (!is_friend)
				{
					num_enemy++;
					bool has_seen = false;
					for (auto j : enemy_info)
					{
						if (j.ID == info.others[i].player_ID)
						{
							has_seen = true;
							j.last_seen_frame = frame;
							break;
						}
					}
					if (!has_seen)				//30帧内新看见的一个敌人
					{
						enemy_coordinate f;
						f.ID = info.others[i].player_ID;
						f.last_seen_frame = frame;
						f.x = info.self.xy_pos.x + cos(info.others[i].polar_pos.angle / (2 * pi)) * info.others[i].polar_pos.distance;
						f.y = info.self.xy_pos.y + sin(info.others[i].polar_pos.angle / (2 * pi)) * info.others[i].polar_pos.distance;
						enemy_info.push_back(f);
					}
				}
				if (!is_friend && info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
				{
					closest_enemy = info.others[i];
					has_enemy = true;
				}
			}
			for (int j = 0; j < enemy_info.size(); j++)
				if (frame - enemy_info[j].last_seen_frame > 30)
					enemy_info.erase(enemy_info.begin() + j);

			if (has_enemy)
			{
				if (info.self.attack_cd > 0 && info.self.status != MOVING && info.self.status != MOVING_SHOOTING)
				{
					double move_angle = closest_enemy.polar_pos.angle - info.self.view_angle;
					move_angle += ((rand() % 2) == 0 ? 1 : -1) * 90;
					while (move_angle < 0)
						move_angle += 360;
					while (move_angle >= 360)
						move_angle -= 360;
					move(move_angle, info.self.view_angle);
					return;
				}
				ITEM weapon = FIST;
				ITEM f = FIST;
				for (int i = 0; i < info.self.bag.size(); ++i)
				{
					if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
					{
						f = info.self.bag[i].type;
						if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range && ITEM_DATA[weapon].range < ITEM_DATA[f].range
							|| (closest_enemy.polar_pos.distance <= ITEM_DATA[weapon].range && double(ITEM_DATA[weapon].damage) / ITEM_DATA[weapon].cd < double(ITEM_DATA[f].damage) / ITEM_DATA[f].cd))
							weapon = f;
					}
				}
				if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
				{
					//if (weapon == FIST)
					//	goto quiet;
					move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
				}
				else
				{
					double shoot_angle = closest_enemy.polar_pos.angle;
					double enemy_x = info.self.xy_pos.x + closest_enemy.polar_pos.distance * cos(closest_enemy.polar_pos.angle / (2 * pi)) + closest_enemy.move_speed * cos(closest_enemy.move_angle / (2 * pi)),
						enemy_y = info.self.xy_pos.y + closest_enemy.polar_pos.distance * sin(closest_enemy.polar_pos.angle / (2 * pi)) + closest_enemy.move_speed * sin(closest_enemy.move_angle / (2 * pi));
					if (closest_enemy.status == MOVING || closest_enemy.status == MOVING_SHOOTING)
						shoot_angle = target_angle(enemy_x, enemy_y);
					shoot(weapon, shoot_angle);
				}
			}
		}
		return;
	}

	void new_pick_up() {
		Item closest_item = { 0, FIST, 100, 0 };
		bool flag = false;
		//closest_item.polar_pos.distance = 100;
		if (weapon_cnt() <= 3) {
			for (int i = 0; i < info.items.size(); ++i)
			{
				if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance && ITEM_DATA[info.items[i].type].type == WEAPON)
				{
					closest_item = info.items[i];
					flag = true;
				}
			}
		}

		else if (!armor_on()) {
			for (int i = 0; i < info.items.size(); ++i)
			{
				if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance && ITEM_DATA[info.items[i].type].type == ARMOR)
				{
					closest_item = info.items[i];
					flag = true;
				}
			}
		}

		else {
			for (int i = 0; i < info.items.size(); ++i)
			{
				if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance && ITEM_DATA[info.items[i].type].number == 17)
				{
					closest_item = info.items[i];
					flag = true;
					break;
				}
			}
			if (!flag) {
				for (int i = 0; i < info.items.size(); ++i)
				{
					if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
					{
						closest_item = info.items[i];
						flag = true;
						break;
					}
				}
			}
		}

		if (flag) {
			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);

				std::cout << "player" << info.player_ID << " picks up item " << ITEM_DATA[closest_item.type].type;
				std::cout << std::endl << "backpack size " << info.self.bag.size();
				std::cout << std::endl << "frame" << frame << std::endl;
			}

			else
			{
				move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);

			}

		}
	}






	void Print_landform()
	{
		unsigned int i = 0;
		block land;
		for (i = 0; i < info.landform_IDs.size(); i++)
		{
			land = get_landform(info.landform_IDs[i]);
			std::cout << i << ": " << "shape：" << land.r << " type：" << land.type << "   0r1= " << land.x0 << " " << land.y0 << " " << land.r << " " << land.x1 << " " << land.y1 << std::endl;
		}
	}

	void moveto(XYPosition aim)
	{
		double dx = aim.x - info.self.xy_pos.x;
		double dy = aim.y - info.self.xy_pos.y;
		double dis = sqrt(dx * dx + dy * dy);
		double aim_angle = atan2(dy, dx) * 180.0 / 3.14159265358979;
		if (aim_angle < 0)aim_angle = 360 + aim_angle;

		//获得偏角参数theta_left和theta_right
		unsigned int i, j;
		block land;
		double theta[4];
		double dist[4];
		double theta_left = 0, theta_right = 0;
		for (i = 0; i < info.landform_IDs.size(); i++)
		{
			land = get_landform(info.landform_IDs[i]);
			if (land.shape == RECTANGLE)
			{
				dist[0] = getdis_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x0, land.y0);
				dist[1] = getdis_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x1, land.y0);
				dist[2] = getdis_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x0, land.y1);
				dist[3] = getdis_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x1, land.y1);
				theta[0] = getangle_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x0, land.y0);
				theta[1] = getangle_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x1, land.y0);
				theta[2] = getangle_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x0, land.y1);
				theta[3] = getangle_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x1, land.y1);
				if (((theta[0] > 0 && theta[1] > 0 && theta[2] > 0 && theta[3] > 0) || (theta[0] < 0 && theta[1] < 0 && theta[2] < 0 && theta[3] < 0))
					&& (dist[0] > 0.5 && dist[1] > 0.5 && dist[2] > 0.5 && dist[3] > 0.5)
					)continue;
				else
				{
					for (j = 0; j < 4; j++)//加小偏移防撞墙
					{
						if (theta[j] > 0)theta[j] = theta[j] + asin(1 / dist[j]);
						else theta[j] = theta[j] - asin(1 / dist[j]);
						if (theta[j] > 90 || dist[j] > 50)theta[j] = 0;
						if (theta[j] < -90 || dist[j]>50)theta[j] = 0;
					}
					for (j = 0; j < 4; j++)
					{
						if (theta[j] > theta_left)theta_left = theta[j];
						if (theta[j] < theta_right)theta_right = theta[j];
					}
				}
			}
			if (land.shape == CIRCLE)
			{
				dist[0] = getdis_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x0, land.y0);
				if (dist[0] - land.r > 0.5)continue;
				else
				{
					theta[0] = getangle_from_point_to_line(info.self.xy_pos.x, info.self.xy_pos.y, aim.x, aim.y, land.x0, land.y0);//主角
					theta[1] = asin((land.r + 1.0) / getdis_from_point_to_point(info.self.xy_pos.x, info.self.xy_pos.y, land.x0, land.y0));//偏角
					if (theta[0] + theta[1] > 90)continue;
					if (theta[0] + theta[1] < -90)continue;
					if (theta[0] + theta[1] > theta_left)theta_left = theta[0] + theta[1];
					if (theta[0] - theta[1] < theta_right)theta_right = theta[0] - theta[1];
				}
			}
		}
		if (theta_left > -theta_right)aim_angle = aim_angle + theta_right;
		else aim_angle = aim_angle + theta_left;
		if (aim_angle < 0)aim_angle = 360 + aim_angle;
		if (aim_angle >= 360)aim_angle = aim_angle - 360;
		std::cout << "thetal&r:" << theta_left << "  " << theta_right << std::endl;

		double set_angle = aim_angle - info.self.view_angle;
		if (set_angle < 0)set_angle = 360 + set_angle;
		//std::cout <<"aimangle: "<< aim_angle << "   dis=" << dis << "   dxdy=" << dx << "." << dy << "   aim=" << aim.x <<"."<< aim.y << std::endl<<std::endl;
		if (dis < 1)
		{
			move(set_angle, set_angle, NOMOVE);
		}
		else move(set_angle, set_angle);
		return;
	}

	void moveto(PolarPosition aim, int parameter)
	{
		double set_angle = aim.angle - info.self.view_angle;
		if (set_angle < 0)set_angle = 360 + set_angle;
		move(set_angle, set_angle);
		if (parameter == NOMOVE)
		{
			move(set_angle, set_angle, NOMOVE);
		}
		else
		{
			move(set_angle, set_angle);
		}
	}

	void moveto(double aim_angle, int parameter)
	{
		double set_angle = aim_angle - info.self.view_angle;
		if (set_angle < 0)set_angle = 360 + set_angle;
		move(set_angle, set_angle);
		if (parameter == NOMOVE)
		{
			move(set_angle, set_angle, NOMOVE);
		}
		else
		{
			move(set_angle, set_angle);
		}
	}

	double getdis_from_point_to_line(double x0, double y0, double x1, double y1, double xp, double yp)
	{
		double k = (y1 - y0) / (x1 - x0);
		double a = k * xp - yp - k * x0 + y0;
		if (a < 0)a = -a;
		return a / (sqrt(k * k + 1));
	}

	double getangle_from_point_to_line(double x0, double y0, double x1, double y1, double xp, double yp)
	{
		double dx_0_1 = x1 - x0;
		double dy_0_1 = y1 - y0;
		double dx_0_p = xp - x0;
		double dy_0_p = yp - y0;
		//求出向量0-p和向量0-1的角度，先求出绝对角度
		double angle_0_1 = atan2(dy_0_1, dx_0_1) * 180.0 / 3.14159265358979;
		double angle_0_p = atan2(dy_0_p, dx_0_p) * 180.0 / 3.14159265358979;
		double result = angle_0_p - angle_0_1;
		if (result >= 180)result = result - 360;
		if (result < -180)result = result + 360;
		return result;
	}

	double getdis_from_point_to_point(double x0, double y0, double x1, double y1)
	{
		return sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
	}
