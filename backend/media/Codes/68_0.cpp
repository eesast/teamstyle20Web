#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#define Debug
#define VOCATION SNIPER
//MEDIC SIGNALMAN HACK SNIPER

using namespace ts20;

const double pi = atan(1.0) * 4;
extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;

int CDlist[50];
const int MedicineNumber = 2;
int Medicine[MedicineNumber][2] = { {15,20},{16,100} };




inline void Mod(double &x, double l,double r) {
	while (x >= r) x -= (r - l);
	while (x < l)x += (r - l);
}



//block MapBlock[1000][1000];
#ifdef Debug
FILE *OUTPUT = fopen("log.txt","w");
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

	for (int i = 0; i < 50; i++) {
		if (CDlist[i]) {
			CDlist[i]--;
		}
	}

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



inline BLOCK_TYPE BlockAsk(double x, double y) {
	int X = int(x) / 100;
	int Y = int(y) / 100;
	x -= X * 100;
	y -= Y * 100;
	AREA Area = MAP[X * 10 + Y];
	for (int i = 0; i < AREA_DATA[Area].size(); i++) {
		block tmp = AREA_DATA[Area][i];
		if (tmp.shape == RECTANGLE && (tmp.x0<x && tmp.x1>x && tmp.y0 < y&&tmp.y1 > y)) {
			return tmp.type;
		}
		else if (tmp.shape == CIRCLE && sqr(x - tmp.x0) + sqr(y - tmp.y0) <= sqr(tmp.r)) {
			return tmp.type;
		}
	}
	return BLOCK_TYPE_SZ;
}



bool WalkAble(BLOCK_TYPE B) {
	if (B == DEEP_WATER) return 0;
	if (B == WALL) return 0;
	if (B == RECTANGLE_BUILDING) return 0;
	if (B == CIRCLE_BUILDING) return 0;
	if (B == TREE) return 0;
	return 1;
}



void TryToMove(double move_angle, double view_angle) {

	for (;; move_angle += 10) {
		double angle = info.self.view_angle + move_angle;
		bool accessable = 1;
		for (double d = 0; d <= 0.5; d += 0.1) {
			BLOCK_TYPE tmp = BlockAsk(MyX + d * cos(angle), MyY + d * sin(angle));
			if (!WalkAble(tmp)) {
				accessable = 0;
			}
		}
		if (accessable) {
			break;
		}
	}
	move(move_angle, view_angle);
#ifdef Debug
	fprintf(OUTPUT, "NormalMove:%lf %lf\n", move_angle, view_angle);
#endif

}



void NormalMove() {
	bool UseMedicine = 0;
	
	if (info.self.attack_cd == 0 && info.self.move_cd == 1)
	for (int i = 0; i < MedicineNumber; i++)
	if (CDlist[Medicine[i][0]] == 0){
		for (int j = 0; j < info.self.bag.size(); ++j) {
			if (info.self.bag[j].type == ITEM_TYPE(Medicine[i][0]) && info.self.bag[j].durability>0 && info.self.hp + Medicine[i][1] <= info.self.hp_limit) {
				shoot(info.self.bag[j].type,0,info.player_ID);
				CDlist[Medicine[i][0]] = ITEM_DATA[Medicine[i][0]].cd;
#ifdef Debug
				fprintf(OUTPUT, "Use Medicine:%d\n", Medicine[i][0]);
#endif
				return;
			}
		}
	}
	
	if (info.self.status != MOVING || info.self.move_cd <= 1)
	{
		double move_angle = 0;

		if (frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.7) {
			move_angle = atan2(info.poison.current_center.y - MyY, info.poison.current_center.x - MyX)/pi*180 - info.self.view_angle;
			Mod(move_angle, 0, 360);
#ifdef Debug
			fprintf(OUTPUT, "Avoid Poison!\n");
#endif
		}

		TryToMove(move_angle, frame % 6 * 60);
	}
}


void play_game()
{
	/* Your code in this function */
	/* sample AI */
	update_info();
	init();
#ifdef Debug
	fprintf(OUTPUT, "\n\n\n");
#endif
#ifdef Debug
	fprintf(OUTPUT, "*****************************\n");
	fprintf(OUTPUT, "player:frame %d  Vocation:%d hp:%lf\n", frame, info.self.vocation, info.self.hp);
	fprintf(OUTPUT, "Position:%lf %lf\n", MyX, MyY);
	fprintf(OUTPUT, "Poison:%lf   %lf   r:%lf   dis:%lf\n", info.poison.current_center.x, info.poison.current_center.y, info.poison.current_radius, DisPoison(MyX,MyY));
	fprintf(OUTPUT, "attack cd:%d   move cd:%d\n", info.self.attack_cd, info.self.move_cd);
	fprintf(OUTPUT, "move angle:%lf   view angle:%lf\n", info.self.move_angle, info.self.view_angle);
	fprintf(OUTPUT, "Status:%d\n", info.self.status);

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

	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 150 + rand() % 100, 150 + rand() % 100 };
		parachute(VOCATION, landing_point);
		return;
	}
	else
	{
		srand(/*time(nullptr)*/ + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{

#ifdef Debug
		fprintf(OUTPUT, "jumping\n");
#endif
		return;
	}


	if (info.self.move_cd <= 1 && frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.2) {
		NormalMove();
		return;
	}
	if (info.others.empty() || info.self.attack_cd != 0)
	{
		if (info.items.empty())
		{
			NormalMove();
		}
		else
		{
			Item closest_item;
			closest_item.polar_pos.distance = 100000;
			for (int i = 0; i < info.items.size(); ++i)
			if ((info.items[i].type!= CODE_CASE || VOCATION == HACK) && (frame<210 || info.poison.current_radius > 1.5*
				DisPoison(MyX + info.items[i].polar_pos.distance*cos(info.items[i].polar_pos.angle), MyY + info.items[i].polar_pos.distance*sin(info.items[i].polar_pos.angle)))){
				double move_angle = info.self.view_angle + info.items[i].polar_pos.angle;
				Mod(move_angle, 0, 360);
				double tmpX, tmpY;
				bool accessible = 1;
				/*for (double d = 0; d < info.items[i].polar_pos.distance; d += 3) {
					tmpX = MyX + d * cos(move_angle);
					tmpY = MyY + d * sin(move_angle);

					BLOCK_TYPE tmp=BlockAsk(tmpX, tmpY);
					if (!WalkAble(tmp)) {
#ifdef Debug
						//fprintf(OUTPUT, "%lf %lf %d\n", tmpX, tmpY, tmp);
#endif
						accessible = 0;
						break;
					}
				}
				*/
#ifdef Debug
				//fprintf(OUTPUT, "ID:%d    Acc:%d\n", info.items[i].item_ID, accessible);
#endif
				
				if (accessible && info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
				{
					closest_item = info.items[i];
				}
			}
#ifdef Debug
			fprintf(OUTPUT, "closest item angle:%lf   distance:%lf\n", closest_item.polar_pos.angle, closest_item.polar_pos.distance);
#endif
			if (closest_item.polar_pos.distance > 99999) {
				NormalMove();
			}else
			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);
#ifdef Debug
				fprintf(OUTPUT, "try pickup:%d\n", closest_item.item_ID);
#endif
			}
			else if (info.self.status != MOVING || info.self.move_cd <= 0)
			{
				Mod(closest_item.polar_pos.angle, 0, 360);
				double view = closest_item.polar_pos.angle;
				if (frame & 3) {
					view += 50;
				}
				else {
					view -= 50;
				}
				Mod(view, 0, 360);
				TryToMove(closest_item.polar_pos.angle, view);
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
#ifdef Debug
			fprintf(OUTPUT, "Enemy:%d   angle:%lf  dis:%lf\n", closest_enemy.player_ID, closest_enemy.polar_pos.angle, closest_enemy.polar_pos.distance);
#endif
			ITEM weapon = FIST;

			for (int i = 0; i < info.self.bag.size(); ++i)
			{
				if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 && CDlist[info.self.bag[i].type] == 0
					&& closest_enemy.polar_pos.distance < ITEM_DATA[info.self.bag[i].type].range 
					&& (ITEM_DATA[info.self.bag[i].type].damage > ITEM_DATA[weapon].damage))
				{
					weapon = info.self.bag[i].type;

				}
			}

			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				TryToMove(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			}
			else
			{
				shoot(weapon, closest_enemy.polar_pos.angle);
#ifdef Debug
				fprintf(OUTPUT, "Shoot:%d %d %lf\n", weapon, closest_enemy.player_ID, closest_enemy.polar_pos.angle);
#endif
				CDlist[weapon] = ITEM_DATA[weapon].cd;
			}
		}
	}
	return;
}