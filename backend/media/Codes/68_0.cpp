#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
//#define Debug
#define VOCATION MEDIC
//MEDIC SIGNALMAN HACK SNIPER

using namespace ts20;

double eps = 0.5;
const double pi = atan(1.0) * 4;
extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

//int CDlist[50];
const int MedicineNumber = 2;
int Medicine[MedicineNumber][2] = { {15,20},{16,100} };
int ViewBack = 0;
int TryPickupFrame = -2;
double lastClosest = 1e9;
double lastPosX, lastPosY;



inline void Mod(double &x, double l,double r) {
	while (x >= r) x -= (r - l);
	while (x < l) x += (r - l);
}



//block MapBlock[1000][1000];
#ifdef Debug
FILE *OUTPUT = fopen("log2.txt","w");
FILE *OUTPUT_TMP = fopen("log_tmp.txt", "w");
#endif

double MyX, MyY;



inline double sqr(double x) {
	return x * x;
}

inline double DisPoison(double x, double y) {
	return sqrt(sqr(x - info.poison.current_center.x) + sqr(y - info.poison.current_center.y));
}



inline void init() {
	MyX = info.self.xy_pos.x;
	MyY = info.self.xy_pos.y;
	if (info.self.move_cd && sqr(lastPosX - MyX) + sqr(lastPosY - MyY) < 0.01) {
#ifdef Debug
		fprintf(OUTPUT_TMP, "Blocked %lf\n", sqr(lastPosX - MyX) + sqr(lastPosY - MyY));
		eps += 0.1;
#endif
	}
	lastPosX = MyX;
	lastPosY = MyY;

	if (TryPickupFrame != frame - 1) {
		ViewBack = 0;
		lastClosest = 1e9;
	}
	/*
	for (int i = 0; i < 50; i++) {
		if (CDlist[i]) {
			CDlist[i]--;
		}
	}
	*/
	/*for (int X = 0; X < 10; X++) {
		for (int Y = 0; Y < 10; Y++) {
			AREA Area = MAP[X * 10 + Y];
			for (int k = 0; k < AREA_DATA[Area].size(); k++) {
				block tmp;
				if (tmp.type == RECTANGLE) {
					for (int i = tmp.x0; i < tmp.x1; i++) {
						for (int j = tmp.y0; j < tmp.y1; j++) {

							MapBlock[X * 100 + i][Y * 100 + j] = tmp;
						}
					}
				}
				else {
					for (int i = -r; i <= r; i++) {
						for (int j = -r; j < r; j++) {

							MapBlock[X * 100 + i][Y * 100 + j] = tmp;
						}
					}
				}

			}
		}
	}*/
}


bool WalkAble(BLOCK_TYPE B) {
	if (B == DEEP_WATER) return 0;
	if (B == WALL) return 0;
	if (B == RECTANGLE_BUILDING) return 0;
	if (B == CIRCLE_BUILDING) return 0;
	if (B == TREE) return 0;
	return 1;
}


inline BLOCK_TYPE BlockAsk(double x, double y) {
	double _x = x, _y = y;

	int X = int(x) / 100;
	int Y = int(y) / 100;
	x -= X * 100;
	y -= Y * 100;
	BLOCK_TYPE res = BLOCK_TYPE_SZ;
	
	AREA Area = MAP[Y * 10 + X];
	for (int i = 0; i < AREA_DATA[Area].size(); i++) {
		block tmp = AREA_DATA[Area][i];
		if (tmp.shape == RECTANGLE) {
			if (tmp.x0 > tmp.x1) {
				std::swap(tmp.x0, tmp.x1);
			}
			if (tmp.y0 > tmp.y1) {
				std::swap(tmp.y0, tmp.y1);
			}
		}
		if (tmp.shape == RECTANGLE && (tmp.x0 - eps <= x && tmp.x1 + eps >= x && tmp.y0 - eps <= y && tmp.y1 + eps >= y)) {
#ifdef Debug
			block _tmp = get_landform(((Y * 10 + X) << 8) + i);
			fprintf(OUTPUT_TMP, "%d RECTANGLE : %d %d %d %d\n", _tmp.type, _tmp.x0,_tmp.y0,_tmp.x1,_tmp.y1);
#endif
			if (!WalkAble(tmp.type)) {
				return tmp.type;
			}
			res = tmp.type;
		}
		else if (tmp.shape == CIRCLE && sqr(x - tmp.x0) + sqr(y - tmp.y0) <= sqr(tmp.r + eps) ) {
#ifdef Debug
			block _tmp = get_landform(((Y * 10 + X) << 8) + i);
			fprintf(OUTPUT_TMP, "%d CIRCLE : %d %d %d\n", _tmp.type, _tmp.x0, _tmp.y0, _tmp.r);
#endif
			if (!WalkAble(tmp.type)) {
				return tmp.type;
			}
			res = tmp.type;
		}
	}
	return res;
}





void TryToMove(double move_angle, double view_angle) {

	for (;; move_angle += 1) {
		bool accessable = 1;
		double angle = info.self.view_angle + move_angle;
		for (double d = 0; d <= 3; d += 0.1) {
			BLOCK_TYPE tmp = BlockAsk(MyX + d * cos(angle / 180 * pi), MyY + d * sin(angle / 180 * pi));

#ifdef Debug
			fprintf(OUTPUT_TMP, "%lf %lf Block:%d  %d\n", MyX + d * cos(angle / 180 * pi), MyY + d * sin(angle / 180 * pi), tmp, WalkAble(tmp));
#endif
			if (!WalkAble(tmp)) {
				accessable = 0;
			}
		}

		if (accessable) {
			break;
		}
	}
	Mod(move_angle, 0, 360);
	move(move_angle, view_angle);
#ifdef Debug
	fprintf(OUTPUT, "NormalMove:%lf %lf\n", move_angle, view_angle);
#endif

}



void NormalMove() {
	
	if (info.self.status != MOVING || info.self.move_cd <= 1)
	{
		double move_angle = 0;

		if (frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.3) {
			move_angle = atan2(info.poison.current_center.y - MyY, info.poison.current_center.x - MyX) / pi * 180 - info.self.view_angle;
			Mod(move_angle, 0, 360);
#ifdef Debug
			fprintf(OUTPUT, "Avoid Poison! %lf %lf\n", atan2(info.poison.current_center.y - MyY, info.poison.current_center.x - MyX) / pi * 180, move_angle);
#endif
		}

		TryToMove(move_angle, frame % 6 * 60);
	}
	else{
		move(180, 180 + (info.self.view_width - 10) / 2 * ViewBack, NOMOVE);
		ViewBack = 0;
	}
}



void Print() {
#ifdef Debug
	fprintf(OUTPUT, "\n\n\n");
#endif
#ifdef Debug
	fprintf(OUTPUT, "*****************************\n");
	//printf("%d\n", frame);
	fprintf(OUTPUT, "player:frame %d  Vocation:%d hp:%lf\n", frame, info.self.vocation, info.self.hp);
	fprintf(OUTPUT, "Position:%lf %lf\n", MyX, MyY);
	fprintf(OUTPUT, "Poison:%lf   %lf   r:%lf   dis:%lf   flag:%d   test:%d\n", info.poison.current_center.x, info.poison.current_center.y, info.poison.current_radius, DisPoison(MyX, MyY),info.poison.move_flag,info.poison.rest_frames);
	fprintf(OUTPUT, "attack cd:%d   move cd:%d\n", info.self.attack_cd, info.self.move_cd);
	fprintf(OUTPUT, "move angle:%lf   view angle:%lf\n", info.self.move_angle, info.self.view_angle);
	fprintf(OUTPUT, "Status:%d\n    ViewBack:%d", info.self.status, ViewBack);

	fprintf(OUTPUT, "\nBag:\n", info.self.status);
	for (int i = 0; i < info.self.bag.size(); ++i) {
		Item tmp = info.self.bag[i];
		fprintf(OUTPUT, "ID:%d   Type:%d   durability:%d\n", tmp.item_ID, tmp.type, tmp.durability);

	}
	fprintf(OUTPUT, "\nItem:\n", info.self.status);
	for (int i = 0; i < info.items.size(); ++i) {
		Item tmp = info.items[i];
		fprintf(OUTPUT, "ID:%d   Type:%d   durability:%d   Pos:%lf  %lf\n", tmp.item_ID, tmp.type, tmp.durability, tmp.polar_pos.angle, tmp.polar_pos.distance);
	}

#endif
}



bool UseMedicine() {

	if (info.self.attack_cd == 0 && info.self.move_cd >= 1) {

		for (int i = 0; i < MedicineNumber; i++)
		//if (CDlist[Medicine[i][0]] == 0)
		{
			for (int j = 0; j < info.self.bag.size(); ++j) {
				if (info.self.bag[j].type == ITEM_TYPE(Medicine[i][0]) && info.self.bag[j].durability > 0 && info.self.hp + Medicine[i][1] <= info.self.hp_limit) {
					shoot(info.self.bag[j].type, 0, info.player_ID);
					//CDlist[Medicine[i][0]] = ITEM_DATA[Medicine[i][0]].cd;
#ifdef Debug
					fprintf(OUTPUT, "Use Medicine: %d  %lf  %d\n", info.self.bag[j].type, 0, info.player_ID);
#endif
					return 1;
				}
			}
		}
	}

	return 0;
}


bool TryPickup() {

	if (info.items.empty())
	{
		return 0;
	}
	else
	{
		Item closest_item;
		closest_item.polar_pos.distance = 100000;
		for (int i = 0; i < info.items.size(); ++i)
			if ((info.items[i].type != CODE_CASE || VOCATION == HACK) && (frame<210 || info.poison.current_radius > 1.3*
				DisPoison(MyX + info.items[i].polar_pos.distance*cos(info.items[i].polar_pos.angle / 180 * pi), MyY + info.items[i].polar_pos.distance*sin(info.items[i].polar_pos.angle / 180 * pi)))) {
				
				if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
				{
					closest_item = info.items[i];
				}
			}
#ifdef Debug
		fprintf(OUTPUT, "closest item  %d  angle:%lf   distance:%lf\n", closest_item.item_ID, closest_item.polar_pos.angle, closest_item.polar_pos.distance);
#endif
		if (closest_item.polar_pos.distance > 9999) {
			return 0;
		}
		
		if (closest_item.polar_pos.distance < 1){
			pickup(closest_item.item_ID);
#ifdef Debug
			fprintf(OUTPUT, "try pickup:%d\n", closest_item.item_ID);
#endif
			return 1;
		}
		else if (info.self.status != MOVING || info.self.move_cd <= 1 || lastClosest > closest_item.polar_pos.distance)
		{
			Mod(closest_item.polar_pos.angle, 0, 360);
			double view = closest_item.polar_pos.angle;
			lastClosest = closest_item.polar_pos.distance;
			TryPickupFrame = frame;
			if (closest_item.polar_pos.distance > 2) {
				view += 180;
				if (frame & 3) {
					ViewBack = 1;
				}
				else {
					ViewBack = -1;
				}
			}
			Mod(view, 0, 360);
			TryToMove(closest_item.polar_pos.angle, view);
			return 1;
		}
		return 0;
	}
}


bool TryAttack() {

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
#ifdef Debug
		fprintf(OUTPUT, "Enemy:%d   angle:%lf  dis:%lf\n", closest_enemy.player_ID, closest_enemy.polar_pos.angle, closest_enemy.polar_pos.distance);
#endif
		ITEM weapon = FIST;

		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 //&& CDlist[info.self.bag[i].type] == 0
				&& closest_enemy.polar_pos.distance < ITEM_DATA[info.self.bag[i].type].range
				&& (ITEM_DATA[info.self.bag[i].type].damage > ITEM_DATA[weapon].damage))
			{
				weapon = info.self.bag[i].type;

			}
		}

		if (weapon == FIST && closest_enemy.polar_pos.distance > 5) {
			double Angle = -closest_enemy.polar_pos.angle;
			Mod(Angle, 0, 360);
			if (info.self.status != MOVING || info.self.move_cd <= 1) {
				TryToMove(Angle, Angle);
			}
			return 1;
		}
		if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
		{
			if (info.self.status != MOVING || info.self.move_cd <= 1) {
				TryToMove(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);

			}
			return 1;
		}
		else
		{
			shoot(weapon, closest_enemy.polar_pos.angle);
#ifdef Debug
			fprintf(OUTPUT, "Shoot:%d %d %lf\n", weapon, closest_enemy.player_ID, closest_enemy.polar_pos.angle);
#endif
			//CDlist[weapon] = ITEM_DATA[weapon].cd;
			return 1;
		}
	}
	return 0;
}



void play_game()
{
	/* Your code in this function */
	/* sample AI */
	update_info();
	Print();
	init();

	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 150 + rand() % 100, 150 + rand() % 100 };
		parachute(VOCATION, landing_point);
		return;
	}
	else
	{
		srand(/*time(nullptr)*/ +info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{

#ifdef Debug
		fprintf(OUTPUT, "jumping\n");
#endif
		return;
	}

	if (UseMedicine()) {
		return;
	}


	if (info.self.move_cd <= 1 && frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.1) {
		NormalMove();
		return;
	}
	if (info.self.move_cd <= 1 && frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.3 && info.poison.move_flag==1) {
		NormalMove();
		return;
	}
	if (info.others.empty() || info.self.attack_cd != 0)
	{
		if (TryPickup()) {
			return;
		}
		else {
			NormalMove();
			return;
		}
	}
	else
	{
		if (TryAttack()) {
			return;
		} else {
			if (TryPickup()) {
				return;
			}
			else {
				NormalMove();
				return;
			}
		}
	}
	return;
}
