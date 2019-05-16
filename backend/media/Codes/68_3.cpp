#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
//#define Debug
#define VOCATION SNIPER
//MEDIC SIGNALMAN HACK SNIPER

using namespace ts20;

const int AimChangeLimit = 4;
double Inf = 1e9;
double PreDis = 0.3;
double eps = 1e-9;
int AimChange = 0;
const double pi = atan(1.0) * 4;
extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;
XYPosition land[4] = { {310, 310},{650, 650},{300, 150}, {565, 420} };
//XYPosition land[4] = { {450, 250},{890, 990},{890, 990}, {890, 990} };

//int CDlist[50];
const int MedicineNumber = 2;
int Medicine[MedicineNumber][2] = { {15,20},{16,100} };
int ViewBack = 0;
int TryPickupFrame = -2;
int ViewRate = 1;
double lastAimValue = -Inf;
bool BShoot, BMove;
bool hasEnemy;
double EnemyAngle;
int lastAimID = 0;
int move_CD;
int AttackMOD;
double move_angle_now = 0;
double view_angle_now = 0;
double lastPosX, lastPosY;
int Dur[50];
int SCOPE_change = 0;

double ItemValue[21] = {
	0,
	3,//HAND_GUN ÊÖÇ¹
	5,//SUBMACHINE_GUN ³å·æÇ¹
	7,//SEMI_AUTOMATIC_RILE °ë×Ô¶¯²½Ç¹
	8,//ASSAULT_RIFLE Í»»÷²½Ç¹
	8,//MACHINE_GUN »úÇ¹
	10,//SNIPER_RILFE ¾Ñ»÷²½Ç¹
	10,//SNIPER_BARRETT °ÍÀ×ÌØ
	1,//TIGER_BILLOW_HAMMER »¢ÌÎ´¸
	8,//CROSSBOW Ê®×Öåó
	0,//VEST_1
	0,//VEST_2
	0,//VEST_3
	0,//INSULATED_CLOTHING ¾øÔµ·þ
	0,//MUFFLER ÏûÉùÆ÷
	2,//BONDAGE ±Á´ø
	3,//FIRST_AID_CASE ¼±¾ÈÏä
	0,//CODE_CASE ÃÜÂëÏä
	-1000,//SCOPE_2
	-1000,//SCOPE_4
	-1000//SCOPE_8
};

inline void Mod(double &x, double l, double r) {
	while (x >= r) x -= (r - l);
	while (x < l) x += (r - l);
}



//block MapBlock[1000][1000];
#ifdef Debug
FILE *OUTPUT = fopen("log5.txt", "w");
FILE *OUTPUT_TMP = fopen("log_5_tmp.txt", "w");
#endif

double MyX, MyY;



inline double sqr(double x) {
	return x * x;
}

inline double DisPoison(double x, double y) {
	return sqrt(sqr(x - info.poison.current_center.x) + sqr(y - info.poison.current_center.y));
}



inline void init() {
	hasEnemy = 0;
	BShoot = BMove = 0;
	AttackMOD = 0;
	move_CD = info.self.move_cd;
	move_angle_now = info.self.move_angle;
	view_angle_now = info.self.view_angle;
	if (info.self.attack_cd != 0) {
		BShoot = 1;
	}
	MyX = info.self.xy_pos.x;
	MyY = info.self.xy_pos.y;
	if (info.self.move_cd && sqr(lastPosX - MyX) + sqr(lastPosY - MyY) < 0.01) {
#ifdef Debug
		fprintf(OUTPUT_TMP, "Blocked %lf  (%lf,%lf) (%lf,%lf)\n", sqr(lastPosX - MyX) + sqr(lastPosY - MyY), lastPosX, lastPosY, MyX, MyY);
		//eps += 0.1;
#endif
	}
	lastPosX = MyX;
	lastPosY = MyY;

	/*if (TryPickupFrame != frame - 1) {
		ViewBack = 0;
		lastAimValue = -Inf;
	}*/
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


block BlockNow;

inline BLOCK_TYPE BlockAsk(double x, double y) {
	if (x <= 0 || x >= 1000 || y <= 0 || y >= 1000) {
		return DEEP_WATER;
	}

	double _x = x, _y = y;

	int X = int(x) / 100;
	int Y = int(y) / 100;
	x -= X * 100;
	y -= Y * 100;
	double R = VOCATION_DATA[info.self.vocation].radius + eps;
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
		if (tmp.shape == RECTANGLE && (tmp.x0 - R <= x && tmp.x1 + R >= x && tmp.y0 - R <= y && tmp.y1 + R >= y)) {
#ifdef Debug
			//block _tmp = get_landform(((Y * 10 + X) << 8) + i);
			//fprintf(OUTPUT_TMP, "%d RECTANGLE : %d %d %d %d\n", _tmp.type, _tmp.x0, _tmp.y0, _tmp.x1, _tmp.y1);
#endif
			if (!WalkAble(tmp.type)) {
				BlockNow = tmp;
				return tmp.type;
			}
			res = tmp.type;
		}
		else if (tmp.shape == CIRCLE && sqr(x - tmp.x0) + sqr(y - tmp.y0) <= sqr(tmp.r + R)) {
#ifdef Debug
			//block _tmp = get_landform(((Y * 10 + X) << 8) + i);
			//fprintf(OUTPUT_TMP, "%d CIRCLE : %d %d %d\n", _tmp.type, _tmp.x0, _tmp.y0, _tmp.r);
#endif
			if (!WalkAble(tmp.type)) {
				BlockNow = tmp;
				return tmp.type;
			}
			res = tmp.type;
		}
	}
	return res;
}



bool WalkABit(double move_angle) {
	double angle = info.self.view_angle + move_angle;
	for (double d = 0; d <= 0.2 * VOCATION_DATA[VOCATION].move + 0.1; d += 0.1) {
		BLOCK_TYPE tmp = BlockAsk(MyX + d * cos(angle / 180 * pi), MyY + d * sin(angle / 180 * pi));

#ifdef Debug
		fprintf(OUTPUT_TMP, "%d : %lf %lf %lf %lf Block:%d  %d\n", frame, MyX, MyY, MyX + d * cos(angle / 180 * pi), MyY + d * sin(angle / 180 * pi), tmp, WalkAble(tmp));
#endif
		if (!WalkAble(tmp)) {
			return 0;
		}
	}
	return 1;
}

void TryToMove(double move_angle, double view_angle) {
	if (!WalkABit(move_angle) && BlockNow.type == WALL) {
		if (abs(BlockNow.x0 - BlockNow.x1) < 1.1) {
			if (BlockNow.y0 < 50) {
				move_angle = 90 - info.self.view_angle;
			}
			else {
				move_angle = 270 - info.self.view_angle;

			}
		}
		else {
			if (BlockNow.x0 < 50) {
				move_angle = 0 - info.self.view_angle;
			}
			else {
				move_angle = 180 - info.self.view_angle;

			}

		}
	}
#ifdef Debug
	fprintf(OUTPUT, "TryToMove:%lf %lf\n", move_angle, view_angle);
#endif

	for (double delta = 0; delta < 180; delta += 5) {
		if (WalkABit(move_angle + delta)) {
			move_angle += delta;
			break;
		}
		if (WalkABit(move_angle - delta)) {
			move_angle -= delta;
			break;
		}
	}
	Mod(move_angle, 0, 360);
	Mod(view_angle, 0, 360);
	move(move_angle, view_angle);
	BMove = 1;
	move_CD = 3;
	move_angle_now = move_angle + info.self.view_angle;
	view_angle_now = view_angle + info.self.view_angle;
#ifdef Debug
	fprintf(OUTPUT, "Move:%lf %lf\n", move_angle, view_angle);
#endif
}



void NormalMove() {

	if (BMove) return;
	double move_angle = move_angle_now - info.self.view_angle;
	double view_angle = move_angle + (info.self.view_width / ViewRate - 20) / 2 * ViewBack;
	Mod(move_angle, 0, 360);
	Mod(view_angle, 0, 360);
	if (ViewBack != 0) {
		if (info.self.move_cd <= 1) {
			move(move_angle, view_angle);
			BMove = 1;
			move_CD = 3;
			move_angle_now = move_angle + info.self.view_angle;
			view_angle_now = view_angle + info.self.view_angle;
		}
		else {
			move(move_angle, view_angle, NOMOVE);
			BMove = 1;
			move_angle_now = move_angle + info.self.view_angle;
			view_angle_now = view_angle + info.self.view_angle;
		}
		ViewBack = 0;
		return;
	}

	if (info.self.move_cd <= 1)
	{

		if (frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 2) {
			move_angle = atan2(info.poison.current_center.y - MyY, info.poison.current_center.x - MyX) / pi * 180 - info.self.view_angle;
			Mod(move_angle, 0, 360);
#ifdef Debug
			fprintf(OUTPUT, "Avoid Poison! %lf %lf\n", atan2(info.poison.current_center.y - MyY, info.poison.current_center.x - MyX) / pi * 180, move_angle);
#endif
		}

		TryToMove(move_angle, 90);
		return;
	}
}



void Print() {
#ifdef Debug
	fprintf(OUTPUT, "\n\n\n");
#endif
#ifdef Debug
	fprintf(OUTPUT, "*****************************\n");
	//printf("%d\n", frame);
	fprintf(OUTPUT, "player:%d frame %d  Vocation:%d hp:%lf\n", info.player_ID, frame, info.self.vocation, info.self.hp);
	fprintf(OUTPUT, "Position:%lf %lf\n", MyX, MyY);
	fprintf(OUTPUT, "Poison:%lf   %lf   r:%lf   dis:%lf   flag:%d   test:%d\n", info.poison.current_center.x, info.poison.current_center.y, info.poison.current_radius, DisPoison(MyX, MyY), info.poison.move_flag, info.poison.rest_frames);
	fprintf(OUTPUT, "attack cd:%d   move cd:%d\n", info.self.attack_cd, info.self.move_cd);
	fprintf(OUTPUT, "move angle:%lf %lf   view angle:%lf\n", info.self.move_angle, move_angle_now, info.self.view_angle);
	fprintf(OUTPUT, "Status:%d\n   ViewBack:%d  AimID:%d  Value:%lf\n", info.self.status, ViewBack, lastAimID, lastAimValue);

	memset(Dur, 0, sizeof(Dur));
	fprintf(OUTPUT, "\nBag:\n");
	for (int i = 0; i < info.self.bag.size(); ++i) {
		Item tmp = info.self.bag[i];
		Dur[tmp.type] = tmp.durability;
		fprintf(OUTPUT, "ID:%d   Type:%d   durability:%d\n", tmp.item_ID, tmp.type, tmp.durability);

	}
	fprintf(OUTPUT, "\nItem:\n", info.self.status);
	for (int i = 0; i < info.items.size(); ++i) {
		Item tmp = info.items[i];
		fprintf(OUTPUT, "ID:%d   Type:%d   durability:%d   Pos:%lf  %lf\n", tmp.item_ID, tmp.type, tmp.durability, tmp.polar_pos.angle, tmp.polar_pos.distance);
	}
	fprintf(OUTPUT, "\Human:\n", info.self.status);
	for (int i = 0; i < info.others.size(); ++i) {
		OtherInfo tmp = info.others[i];
		fprintf(OUTPUT, "ID:%d   Status:%d   angle:%lf   dist:%lf\n", tmp.player_ID, tmp.status, tmp.polar_pos.angle, tmp.polar_pos.distance);
	}

#endif
}



bool UseMedicine() {
	if (BShoot) return 0;

	if (info.self.attack_cd == 0 && info.self.move_cd >= 1) {

		for (int i = 0; i < MedicineNumber; i++)
			//if (CDlist[Medicine[i][0]] == 0)
		{
			for (int j = 0; j < info.self.bag.size(); ++j) {
				if (info.self.bag[j].type == ITEM(Medicine[i][0]) && info.self.bag[j].durability > 0 && info.self.hp + Medicine[i][1] <= info.self.hp_limit) {
					shoot(info.self.bag[j].type, 0);
					BShoot = 1;
					//CDlist[Medicine[i][0]] = ITEM_DATA[Medicine[i][0]].cd;
#ifdef Debug
					fprintf(OUTPUT, "Use Medicine: %d\n", info.self.bag[j].type);
#endif
					return 1;
				}
			}
		}
	}

	return 0;
}
bool CanPickUp() {

	if (info.items.empty())
	{
		return 0;
	}
	else
	{
		for (int i = 0; i < info.items.size(); ++i) {
			if ((info.items[i].type != CODE_CASE || VOCATION == HACK) && info.items[i].polar_pos.distance < 1) {
				pickup(info.items[i].item_ID);
#ifdef Debug
				fprintf(OUTPUT, "try pickup:%d\n", info.items[i].item_ID);
#endif
				ViewBack = 0;
				AimChange = 0;
				lastAimID = 0;
				return 1;
			}
		}
		return 0;
	}
}

bool TryPickup() {

	if (BMove) return 0;
	if (info.items.empty())
	{
		return 0;
	}
	else
	{
		Item closest_item;
		double MaxValue = -Inf;
		closest_item.polar_pos.distance = 100000;
		for (int i = 0; i < info.items.size(); ++i)
			if ((info.items[i].type != CODE_CASE || VOCATION == HACK) && (frame<210 || info.poison.current_radius > 1.7*
				DisPoison(MyX + info.items[i].polar_pos.distance * cos((info.items[i].polar_pos.angle + info.self.view_angle) / 180 * pi),
					MyY + info.items[i].polar_pos.distance * sin((info.items[i].polar_pos.angle + info.self.view_angle) / 180 * pi)))) {

				if (ItemValue[info.items[i].type] - info.items[i].polar_pos.distance > MaxValue)
				{
					MaxValue = ItemValue[info.items[i].type] - info.items[i].polar_pos.distance;
					closest_item = info.items[i];
				}
			}
#ifdef Debug
		fprintf(OUTPUT, "closest item  %d  angle:%lf   distance:%lf\n", closest_item.item_ID, closest_item.polar_pos.angle, closest_item.polar_pos.distance);
#endif
		if (closest_item.polar_pos.distance > 9999) {
			return 0;
		}

		if (closest_item.polar_pos.distance < 1) {
			pickup(closest_item.item_ID);
#ifdef Debug
			fprintf(OUTPUT, "try pickup:%d\n", closest_item.item_ID);
#endif
			ViewBack = 0;
			AimChange = 0;
			lastAimID = 0;
			return 1;
		}
		else if (info.self.move_cd <= 1 || (lastAimID != closest_item.item_ID && lastAimValue < MaxValue && AimChange < AimChangeLimit))
		{
			Mod(closest_item.polar_pos.angle, 0, 360);
			double view = closest_item.polar_pos.angle;
			lastAimValue = MaxValue;
			if (lastAimID != closest_item.item_ID) {
				AimChange++;
				lastAimID = closest_item.item_ID;
			}
			TryPickupFrame = frame;
			ViewBack = 0;
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

	if (BShoot) return 0;
	if (info.self.attack_cd != 0) {
		return 0;
	}
	OtherInfo closest_enemy;
	closest_enemy.polar_pos.distance = 100000;
	//check teammate
	for (int i = 0; i < info.others.size(); ++i)
		if (info.others[i].status != DEAD && info.others[i].status != REAL_DEAD) {
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
				hasEnemy = true;
			}
		}
	if (hasEnemy)
	{
#ifdef Debug
		fprintf(OUTPUT, "Enemy:%d   angle:%lf  dis:%lf\n", closest_enemy.player_ID, closest_enemy.polar_pos.angle, closest_enemy.polar_pos.distance);
#endif
		ITEM weapon = FIST;

		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 //&& CDlist[info.self.bag[i].type] == 0
				&& closest_enemy.polar_pos.distance < ITEM_DATA[info.self.bag[i].type].range
				&& (1. * ITEM_DATA[info.self.bag[i].type].damage / ITEM_DATA[info.self.bag[i].type].cd > 1. * ITEM_DATA[weapon].damage / ITEM_DATA[weapon].cd))
			{
				weapon = info.self.bag[i].type;

			}
		}

		EnemyAngle = closest_enemy.polar_pos.angle + view_angle_now;
		double x = MyX + closest_enemy.polar_pos.distance*cos(EnemyAngle / 180 * pi), y = MyY + closest_enemy.polar_pos.distance*sin(EnemyAngle / 180 * pi);
		x += PreDis * VOCATION_DATA[closest_enemy.vocation].move * cos(closest_enemy.move_angle / 180 * pi);
		y += PreDis * VOCATION_DATA[closest_enemy.vocation].move * sin(closest_enemy.move_angle / 180 * pi);
		EnemyAngle = atan2(y - MyY, x - MyX) / pi * 180 - view_angle_now;
		Mod(EnemyAngle, 0, 360);

		if (weapon == FIST && closest_enemy.polar_pos.distance >= 1) {
			return 0;
			double Angle = -closest_enemy.polar_pos.angle;
			Mod(Angle, 0, 360);
			if (info.self.move_cd <= 1) {
				TryToMove(Angle, Angle);
			}
			return 1;
		}
		if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
		{
			return 0;
			if (info.self.move_cd <= 1) {
				TryToMove(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);

			}
			return 1;
		}
		else
		{
			//printf("frame:%d %lf %lf %lf ID:%d STATUS:%d HP:%lf Weapon:%d\n",frame , closest_enemy.polar_pos.angle, EnemyAngle, closest_enemy.polar_pos.distance, closest_enemy.player_ID, closest_enemy.status, info.self.hp, weapon);
#ifdef Debug
			fprintf(OUTPUT,"%lf %lf %lf ID:%d STATUS:%d HP:%lf Weapon:%d\n", closest_enemy.polar_pos.angle, EnemyAngle, closest_enemy.polar_pos.distance, closest_enemy.player_ID, closest_enemy.status, info.self.hp, weapon);
#endif
			shoot(weapon, EnemyAngle);
			BShoot = 1;
#ifdef Debug
			fprintf(OUTPUT, "Shoot:%d %d %lf\n", weapon, closest_enemy.player_ID, closest_enemy.polar_pos.angle);
#endif
			//CDlist[weapon] = ITEM_DATA[weapon].cd;
			return 1;
		}
	}
	return 0;
}
bool FindEnemy() {
	OtherInfo closest_enemy;
	closest_enemy.polar_pos.distance = 100000;
	for (int i = 0; i < info.others.size(); ++i)
		if (info.others[i].status != DEAD && info.others[i].status != REAL_DEAD) {
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
				hasEnemy = true;
			}
		}
	if (hasEnemy)
	{
#ifdef Debug
		fprintf(OUTPUT, "Enemy:%d   angle:%lf  dis:%lf\n", closest_enemy.player_ID, closest_enemy.polar_pos.angle, closest_enemy.polar_pos.distance);
#endif
		ITEM weapon = FIST;

		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0 //&& CDlist[info.self.bag[i].type] == 0
				&& closest_enemy.polar_pos.distance < ITEM_DATA[info.self.bag[i].type].range
				&& (1. * ITEM_DATA[info.self.bag[i].type].damage / ITEM_DATA[info.self.bag[i].type].cd > 1. * ITEM_DATA[weapon].damage / ITEM_DATA[weapon].cd))
			{
				weapon = info.self.bag[i].type;

			}
		}

		EnemyAngle = closest_enemy.polar_pos.angle + view_angle_now;
		double x = MyX + closest_enemy.polar_pos.distance*cos(EnemyAngle / 180 * pi), y = MyY + closest_enemy.polar_pos.distance*sin(EnemyAngle / 180 * pi);
		x += PreDis * VOCATION_DATA[closest_enemy.vocation].move * cos(closest_enemy.move_angle / 180 * pi);
		y += PreDis * VOCATION_DATA[closest_enemy.vocation].move * sin(closest_enemy.move_angle / 180 * pi);
		EnemyAngle = atan2(y - MyY, x - MyX) / pi * 180 - view_angle_now;
		Mod(EnemyAngle, 0, 360);
		if (weapon == FIST && closest_enemy.polar_pos.distance >= 1) {
			return 0;
		}
		return 1;
	}
	return 0;
}


void play_game()
{
	/* Your code in this function */
	/* sample AI */
	update_info();
	init();
	Print();
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = land[VOCATION];
		landing_point.x += rand() % 11 - 5;
		landing_point.y += rand() % 11 - 5;
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
	if (CanPickUp()) {
		return;
	}

	AttackMOD = FindEnemy();
#ifdef Debug
	fprintf(OUTPUT, "MOD:%d\n", AttackMOD);
#endif
	if (!AttackMOD) {

		if (info.self.move_cd <= 1 && frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.3) {
			NormalMove();
		}
		else {
			if (info.self.move_cd <= 1 && frame > 200 && info.poison.current_radius > 5 && info.poison.current_radius < DisPoison(MyX, MyY) * 1.7 && info.poison.move_flag == 1) {
				NormalMove();
			}
			else
				if (!TryPickup()) {
					NormalMove();
				}
		}
		UseMedicine();
	}
	else {
		ViewBack = 0;
		lastAimID = -1;
		lastAimValue = -Inf;
		AimChange = 0;
		if (info.self.move_cd <= 1) {
			TryToMove(90 + EnemyAngle + rand()%21 - 10 + rand()%2 * 180, EnemyAngle);
		}

		double D = VOCATION_DATA[VOCATION].move;
		if (move_CD == 3) D *= 0.2;
		if (move_CD == 2) D *= 0.5;
		if (move_CD == 1) D *= 0.3;
		MyX += D * cos(move_angle_now / 180 * pi);
		MyY += D * sin(move_angle_now / 180 * pi);
		TryAttack();
	}


	/*
	if (!BShoot) {
		if ((frame - SCOPE_change > 80) && (Dur[SCOPE_2] != 0) && (Dur[SNIPER_BARRETT] || Dur[SNIPER_RILFE]) && ViewRate == 1) {
			shoot(SCOPE_2, 0);
			SCOPE_change = frame;
			ViewRate = 2;
#ifdef Debug
			fprintf(OUTPUT, "Shoot:%d SCOPE_2\n", SCOPE_2);
#endif
		}
		

		if ((frame - SCOPE_change > 80 || (Dur[SNIPER_BARRETT] == 0 && Dur[SNIPER_RILFE] == 0)) && ViewRate == 2) {
			shoot(ITEM(-1), 0);
			SCOPE_change = frame;
			ViewRate = 1;
#ifdef Debug
			fprintf(OUTPUT, "Shoot:%d -1\n", SCOPE_2);
#endif
		}
	}*/

	return;
}