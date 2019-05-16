#include "api.h"
#include "base.h"
#include "constant.h"
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <unordered_map>

#define M_PI (acos(-1.0))

// 0 - MEDIC, 1 - SIGNALMAN, 2 - HACK, 3 - SNIPER
#define AI_VOCATION MEDIC 
// Debug Info
//std::fstream f("./playback/1.txt", std::ios::out);
std::fstream f;

#define REFRESH_PERIOD 5
#define REFRESH_PHASE 4
#define TURN (90.0 * (frame % 4))

enum GAMEPLAYMODE { GAME_NO_OUTPUT = 0, GAME, TEST, OTHER_MODE };
const GAMEPLAYMODE MODE = GAME_NO_OUTPUT;


const XYPosition landing_point = {
	AI_VOCATION >= 2 ? 420 + rand() % 160 : 720 + rand() % 160, 
    AI_VOCATION >= 2 ? 420 + rand() % 160 : 420 + rand() % 160
};
//const XYPosition landing_point = { 200 + rand() % 400, 400 + rand() % 400};
//const XYPosition landing_point = {450, 310};
const XYPosition destination = {450, 550};



using namespace ts20;

// Annotation labels
// "Temporary"
// "Unfinished"
// "Unused"
// "Debug Info"



// interface
extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;
SelfInfo aiPrevSelf;



// general functions 
XYPosition vPolarToXY(PolarPosition);
void vAngleScale(double &);
inline double vCalcDist(XYPosition, XYPosition);
inline double vCalcAngle(XYPosition, XYPosition);

void mainfunc();



// finding path begin (zms)

const int BeachNode = 13;
int BeachLoc[BeachNode][2] = {
    {25,  5}, { 5, 25}, {10, 50}, { 5, 75},
    {30, 50}, {50, 30}, {70, 10}, {90, 10},
    {90, 50}, {70, 50}, {50, 70}, {25, 95},
    {90, 90}
};
int BeachAdj[BeachNode][BeachNode] = {
    { 0, 28, -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1},
    {28,  0, 26, 50, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, 26,  0, 26, 20, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, 50, 26,  0, 35, -1, -1, -1, -1, -1, -1, 28, -1},
    {-1, -1, 20, 35,  0, 28, -1, -1, -1, -1, 28, -1, -1},
    {35, -1, -1, -1, 28,  0, 28, -1, -1, 28, -1, -1, -1},
    {-1, -1, -1, -1, -1, 28,  0, 20, -1, 40, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, 20,  0, 40, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, 40,  0, 20, -1, -1, -1},
    {-1, -1, -1, -1, -1, 28, 40, -1, 20,  0, 28, -1, 44},
    {-1, -1, -1, -1, 28, -1, -1, -1, -1, 28,  0, 35, -1},
    {-1, -1, -1, 28, -1, -1, -1, -1, -1, -1, 35,  0, 65},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 44, -1, 65,  0}
};
int BeachNext[BeachNode][BeachNode] = {
	{  0,  1,  1,  1,  5,  5,  5,  5,  5,  5,  5,  1,  5},
	{  0,  1,  2,  3,  2,  0,  0,  0,  0,  0,  2,  3,  0},
	{  1,  1,  2,  3,  4,  4,  4,  4,  4,  4,  4,  3,  3},
	{  1,  1,  2,  3,  4,  4,  4,  4,  4,  4,  4, 11, 11},
	{  5,  2,  2,  3,  4,  5,  5,  5,  5,  5, 10,  3,  5},
	{  0,  0,  4,  4,  4,  5,  6,  6,  9,  9,  4,  4,  9},
	{  5,  5,  5,  5,  5,  5,  6,  7,  7,  9,  9,  9,  9},
	{  6,  6,  6,  6,  6,  6,  6,  7,  8,  6,  6,  6,  6},
	{  9,  9,  9,  9,  9,  9,  7,  7,  8,  9,  9,  9,  9},
	{  5,  5,  5,  5,  5,  5,  6,  6,  8,  9, 10, 10, 12},
	{  4,  4,  4,  4,  4,  4,  9,  9,  9,  9, 10, 11,  9},
	{  3,  3,  3,  3,  3,  3, 10, 10, 10, 10, 10, 11, 12},
	{  9,  9, 11, 11,  9,  9,  9,  9,  9,  9,  9, 11, 12}
};

const int CityNode = 24;
int CityLoc[CityNode][2] = {
    { 2,  2}, {50,  2}, {98,  2}, {98, 50}, {98, 98}, {50, 98}, { 2, 50},
    { 8, 92}, {35, 92}, {65, 92}, {92, 92},
    {92, 68}, {65, 68}, {35, 68}, { 8, 68},
    { 8, 32}, {35, 32}, {65, 32}, {92, 32},
    {92,  8}, {65,  8}, {35,  8}, { 8,  8},
	{2, 98}
};
int CityAdj[CityNode][CityNode] = {
    { 0, 48, -1, -1, -1, -1, 48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {48,  0, 48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16, 16, -1, -1},
    {-1, 48,  0, 48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, 48,  0, 48, -1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1, -1},
    {-1, -1, -1, 48,  0, 48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, 48,  0, -1, -1, 16, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 48},
    {48, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, 19, 19, -1, -1, -1, -1, -1, -1, -1, 48},
    {-1, -1, -1, -1, -1, -1, -1,  0, 27, -1, -1, -1, -1, -1, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, 16, -1, 27,  0, 30, -1, -1, -1, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, 16, -1, -1, 30,  0, 27, -1, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 27,  0, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, 19, -1, -1, -1, -1, -1, -1, 24,  0, 27, -1, -1, -1, -1, 45, 36, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, 27,  0, 30, -1, -1, 47, -1, 45, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, 30,  0, 27, 45, -1, 47, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, 19, 24, -1, -1, -1, -1, -1, 27,  0, 36, 45, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1, -1, -1, 45, 36,  0, 27, -1, -1, -1, -1, -1, 24, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 47, -1, 45, 27,  0, 30, -1, -1, -1, 24, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 45, -1, 47, -1, -1, 30,  0, 27, -1, 24, -1, -1, -1},
    {-1, -1, -1, 19, -1, -1, -1, -1, -1, -1, -1, 36, 45, -1, -1, -1, -1, 27,  0, 24, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24,  0, 27, -1, -1, -1},
    {-1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, 27,  0, 30, -1, -1},
    {-1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, 30,  0, 27, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, -1, -1, 27,  0, -1},
	{-1, -1, -1, -1, -1, 48, 48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0}
};
int CityNext[CityNode][CityNode]{
	{  0,  1,  1,  1,  6,  6,  6,  6,  6,  6,  1,  1,  6,  6,  6,  6,  1,  1,  1,  1,  1,  1,  6,  6},
	{  0,  1,  2, 20, 20, 20, 21, 21, 20, 21, 20, 20, 21, 20, 21, 21, 21, 20, 20, 20, 20, 21, 21, 21},
	{  1,  1,  2,  3,  3,  3,  1,  1,  3,  3,  3,  3,  3,  3,  1,  1,  1,  1,  3,  3,  1,  1,  1,  1},
	{ 18, 18,  2,  3,  4, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 18, 18, 18, 18, 18, 18, 18, 18, 11},
	{  5,  3,  3,  3,  4,  5,  5,  5,  5,  5,  5,  3,  5,  5,  5,  5,  3,  3,  3,  3,  3,  3,  5,  5},
	{  8,  8,  9,  9,  4,  5,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  9,  8,  9,  9,  8,  9,  8, 23},
	{  0, 15, 15, 14, 14, 14,  6, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 23},
	{ 14, 14, 14,  8,  8,  8, 14,  7,  8,  8,  8,  8,  8,  8, 14, 14, 14,  8,  8,  8,  8, 14, 14,  8},
	{  7, 13,  9,  9,  5,  5,  7,  7,  8,  9,  9,  9,  9, 13,  7, 13,  7, 13, 13, 13, 13,  7, 13,  5},
	{  8, 12, 10, 10,  5,  5,  8,  8,  8,  9, 10, 10, 12,  8,  8, 12, 12, 10, 12, 12, 10, 12, 12,  5},
	{ 11, 11, 11, 11,  9,  9,  9,  9,  9,  9, 10, 11,  9,  9,  9,  9,  9, 11, 11, 11, 11,  9,  9,  9},
	{ 17, 17,  3,  3,  3, 10, 12, 10, 10, 10, 10, 11, 12, 12, 12, 12, 12, 17, 18, 18, 17, 12, 12, 10},
	{ 13, 16, 11, 11,  9,  9, 13,  9,  9,  9,  9, 11, 12, 13, 13, 16, 16, 11, 18, 18, 11, 16, 16,  9},
	{ 14, 17, 12, 12,  8,  8, 14,  8,  8,  8,  8, 12, 12, 13, 14, 15, 14, 17, 17, 17, 17, 14, 15,  8},
	{  6, 16, 16, 13,  7,  7,  6,  7,  7,  7,  7, 13, 13, 13, 14, 15, 16, 13, 13, 13, 13, 16, 15,  6},
	{  6, 16, 16, 16, 13, 13,  6, 14, 13, 16, 16, 16, 16, 13, 14, 15, 16, 16, 16, 16, 16, 16, 22,  6},
	{ 21, 21, 21, 17, 17, 12, 15, 14, 14, 12, 12, 12, 12, 14, 14, 15, 16, 17, 17, 17, 17, 21, 15, 15},
	{ 20, 20, 20, 18, 18, 13, 16, 13, 13, 11, 11, 11, 11, 13, 13, 16, 16, 17, 18, 18, 20, 16, 16, 16},
	{ 17, 17,  3,  3,  3, 12, 17, 17, 17, 12, 11, 11, 12, 17, 17, 17, 17, 17, 18, 19, 17, 17, 17, 17},
	{ 20, 20, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 20, 20, 20, 20},
	{  1,  1,  1, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 19, 20, 21, 21, 21},
	{  1,  1,  1, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 20, 20, 21, 22, 22},
	{ 15, 21, 21, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 21, 21, 21, 22, 15},
	{  6,  6,  6,  5,  5,  5,  6,  6,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6, 23}
};

const int HillNode = 12;
int HillLoc[HillNode][2] = {
    { 3,  5}, {40,  5}, {70, 25}, {95,  5},
    {95, 55}, {60, 55}, {25, 45}, { 3, 45},
    { 5, 70}, { 5, 95}, {60, 95}, {95, 95}
};
int HillAdj[HillNode][HillNode] = {
    { 0, 40, -1, 95, -1, -1, -1, 40, -1, -1, -1, -1},
    {40,  0, 31, 56, -1, -1, 38, -1, -1, -1, -1, -1},
    {-1, 31,  0, -1, -1, 36, -1, -1, -1, -1, -1, -1},
    {95, 56, -1,  0, 50, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, 50,  0, 35, -1, -1, -1, -1, -1, 40},
    {-1, -1, 36, -1, 35,  0, 36, -1, -1, -1, 40, -1},
    {-1, 38, -1, -1, -1, 36,  0, 20, 32, -1, 61, -1},
    {40, -1, -1, -1, -1, -1, 20,  0, 25, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, 32, 25,  0, 25, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, 25,  0, 55, -1},
    {-1, -1, -1, -1, -1, 40, 61, -1, -1, 55,  0, 35},
    {-1, -1, -1, -1, 40, -1, -1, -1, -1, -1, 35,  0}
};
int HillNext[HillNode][HillNode] = {
	{  0,  1,  1,  3,  7,  7,  7,  7,  7,  7,  7,  7},
	{  0,  1,  2,  3,  2,  2,  6,  6,  6,  6,  2,  6},
	{  1,  1,  2,  1,  5,  5,  1,  1,  1,  1,  5,  5},
	{  0,  1,  1,  3,  4,  4,  1,  1,  1,  1,  4,  4},
	{  5,  5,  5,  3,  4,  5,  5,  5,  5,  5,  5, 11},
	{  6,  2,  2,  4,  4,  5,  6,  6,  6,  6, 10,  4},
	{  7,  1,  1,  1,  5,  5,  6,  7,  8,  8, 10, 10},
	{  0,  6,  6,  6,  6,  6,  6,  7,  8,  8,  6,  6},
	{  7,  6,  6,  6,  6,  6,  6,  7,  8,  9,  9,  9},
	{  8,  8,  8,  8,  8,  8,  8,  8,  8,  9, 10, 10},
	{  6,  6,  5,  5,  5,  5,  6,  6,  9,  9, 10, 11},
	{ 10, 10,  4,  4,  4,  4, 10, 10, 10, 10, 10, 11}
};

const int FarmlandNode = 2;
int FarmlandLoc[FarmlandNode][2] = {
	{75, 25}, {75, 75}
};
int FarmlandAdj[FarmlandNode][FarmlandNode] = {
	{ 0, 40},
	{40,  0}
};
int FarmlandNext[FarmlandNode][FarmlandNode] = {
	{0, 1}, {0, 1}
};

const int PoolNode = 6;
int PoolLoc[PoolNode][2] = {
    { 5,  5}, {55, 10}, {95, 30},
    { 5, 90}, {65, 98}, {95, 70}
};
int PoolAdj[PoolNode][PoolNode] = {
    { 0, 50, -1, 85, -1, -1},
    {50,  0, 45, 94, 88, -1},
    {-1, 45,  0, -1, -1, 40},
    {85, 94, -1,  0, 60, -1},
    {-1, 88, -1, 60,  0, 41},
    {-1, -1, 40, -1, 41,  0}
};
int PoolNext[PoolNode][PoolNode] = {
    {0, 1, 1, 3, 3, 1},
    {0, 1, 2, 3, 4, 2},
    {1, 1, 2, 5, 5, 5},
    {0, 1, 1, 3, 4, 4},
    {3, 1, 5, 3, 4, 5},
    {2, 2, 2, 4, 4, 5}
};

int GetAreaId(XYPosition CurPosition);
int GetAreaId(XYPosition CurPosition, bool nothing);
double GetMoveAngle(XYPosition CurPosition, XYPosition TargetPosition);
double GetMoveAngle_small(XYPosition CurPosition, XYPosition TargetPosition, int id);
double GetMoveAngle_big(XYPosition CurPosition, XYPosition TargetPosition, int cur_id, int tar_id);

// finding path end (zms)



// item queue
// caution: 'Item' is the struct of an item, 'ITEM' is the type
std::vector<Item> aiWeaponCase;
std::vector<Item> aiFilterWeaponCase;   // temporary usage
std::vector<Item> aiMedCase;
std::vector<Item> aiFilterMedCase;      // temporary usage
int aiArmor = VEST_1 - 1;
std::unordered_map<int, int> vWeaponPriority;	// pre-decided
std::unordered_map<int, int> vMedPriority;		// pre-decided
std::unordered_map<int, int> vAllPriority;		// pre-decided
bool aiFilterWeaponFlag = false;
bool aiFilterMedFlag = false;
void vUpdateWeapon(Item);   // update when picked up or used
void vUpdateMed(Item);      // update when picked up or used
void vInitWeaponPriority();
void vInitMedPriority();
void vInitAllPriority();
inline bool isWeapon(ITEM);
inline bool isArmor(ITEM);
inline bool isMed(ITEM);
int vGetWeaponDurabilitySum();
bool aiFirstShotFlag = false;
void vFilterWeapon(ITEM);   // filter via weapon type
void vFilterMed(ITEM);      // filter via med type
void vFilterWeapon(std::vector<ITEM>);
void vFilterMed(std::vector<ITEM>);
void vFilterWeapon(int);    // filter via weapon shooting range
void vCheckItemStatus();	// sync with items in bag
void vClearFilter();	// clear filter at the end of each frame



// ai behavior
enum VSTATUS { Attack, Retreat, MedSelf, MedTeam,
    Radio, Pick, Turn, Trek, Stand, Undecided };
struct vAiBehavior {
    VSTATUS act;
    double move_angle;
    double view_angle;
    int target_ID;
    int msg;
};
vAiBehavior aiBehavior;
std::vector<vAiBehavior> aiPrevAct;
bool isNoMove();
void vClearBehavior();



// ai decision
bool vEncounterEnemy();
bool vLoseHp();
bool vDeadTeammate();
bool vRunPoison();
bool vPickItem();
bool vPickNearItem();
bool vWalkAround();
bool vStandBy();



/*
// ai evaluation
double aiEntireMap[8][12]; // radius: 8 * 50, angle: 360 / 12
double aiThreatMap[8][12];
double aiPoisonMap[8][12];
std::unordered_map<int, double> aiAroundMap_1; // index: delta_x * 100 + delta_y
std::unordered_map<int, double> aiAroundMap_2; // index :delta_x * 20 + delta_y / 5
std::unordered_map<int, double> aiAroundMap_3; // index: delta_x * 4 + delta_y / 25
void vUpdateEvaluation();
*/



// ai audio: provide entire evaluation
// Unused
void vAudioHandler();
void vRadioHandler(Sound);
//void vRadioEncode();
//void vRadioDecode();



// ai sight: provide around evaluation ( and entire evaluation )
struct vAiInfo {
    int lastUpdateFrame;
	int player_ID;
    float value;
    float threat;
    float priority;

    VOCATION vocation;
    STATUS status;
    double move_angle;
    double view_angle;
    double move_speed;
    PolarPosition rel_polar_pos;
    XYPosition abs_xy_pos;
};
std::unordered_map<int, vAiInfo> aiKV;
std::vector<int> aiFriend;	// friend array
std::vector<int> aiEnemy;	// enemy priority queue
void vSightHandler();
void vDetectPlayer(int);    // para: index in info
void vUpdateEnemy(int);     // para: player_ID
void vLostEnemy();
void vCalcEnemyPriority(vAiInfo &);





/* *********************************** */
/* ********** Main Function ********** */

void play_game() {
    // update game info
	update_info();
    info.self.move_angle = vCalcAngle(info.self.xy_pos, aiPrevSelf.xy_pos);

    // Debug Info
    if (MODE) {
        f << "frame (" << frame << ") hp (" << info.self.hp
            << ") view_angle (" << info.self.view_angle << ") move_angle (" << info.self.move_angle
            << ") \npos (" << info.self.xy_pos.x << " " << info.self.xy_pos.y
            << ") poison (" << info.poison.next_center.x << " " << info.poison.next_center.y 
            << ") poison_dist (" << sqrt(vCalcDist(info.self.xy_pos, info.poison.next_center)) << ") " << std::endl;
	}
    // Debug Info End

    // check weapon and med queue
    if (frame % REFRESH_PERIOD == REFRESH_PHASE) {
        vCheckItemStatus();

		// Debug Info
        if (MODE) {
            f << "Bag: \n";
            for (int i = 0; i < info.self.bag.size(); ++i) {
                Item b = info.self.bag[i];
                f << "\tItem (id: " << b.item_ID << "): "
                    << "type (" << b.type << ") durability (" << b.durability << ")\n";
            }
            f << "Weapon Case: \n";
            for (int i = 0; i < aiWeaponCase.size(); ++i) {
                Item w = aiWeaponCase[i];
                f << "\tWeapon (type: " << w.type << "): "
                    << "durability (" << w.durability << ")\n";
            }
            f << "Med Case: \n";
            if (aiMedCase.size() == 0) {
                f << "\tNone\n";
            } else {
                for (int i = 0; i < aiMedCase.size(); ++i) {
                    Item m = aiMedCase[i];
                    f << "\tMed (type: " << m.type << "): "
                        << "durability (" << m.durability << ")\n";
                }
            }
        }
        // Debug Info End
    }
    
    // landing
	if (frame == 0) {
		srand(time(nullptr) + teammates[0]);
		parachute(AI_VOCATION, landing_point);
        
        vInitWeaponPriority();
        vInitMedPriority();
        vInitAllPriority();

		aiWeaponCase.push_back({ 0, FIST, {-1, -1}, 9999 });

		aiPrevAct.push_back({ Undecided, 0, 0, 0, 0 });

		return;
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING || 
		info.self.status == DEAD || info.self.status == REAL_DEAD) {
		return;
	}

	// Debug Info
    if (MODE) {
        if (info.self.status == ON_PLANE || info.self.status == JUMPING) {
            f << "jumping" << std::endl;
            return;
        }
    }
    // Debug Info End
    


    // check gameplay mode
    if (MODE == TEST) {
    	double angle = GetMoveAngle(info.self.xy_pos, destination) - info.self.view_angle;
        aiBehavior = { Trek, angle, angle + TURN, 0, 0 };
        /*
        if (fabs(aiPrevAct[0].move_angle - angle) < 0.2) {
            aiBehavior = { Trek, angle, angle + TURN, 0, 0 };
        } else {
            aiBehavior = { Turn, 0.0, angle + TURN, 0, 0 };
        }
        */

        mainfunc();

        return;
    }
    


    // stage 0: evaluate battle field status
    //vUpdateEvaluation();
    vAudioHandler();
    vSightHandler();



	// stage 1: decide ai behavior & parameters
    // Temporary
    bool decided = false;

	if (!aiFirstShotFlag && vGetWeaponDurabilitySum() < 6) {
		if (!decided)
			decided = vPickItem();
	}
    if (!decided)
        decided = vEncounterEnemy();
    if (!decided)
        decided = vLoseHp();
	if (!decided)
		decided = vPickNearItem();
    if (aiPrevAct[0].msg != 0) {
        if (aiPrevAct[0].act == Trek && aiPrevAct[0].msg == -1) {
            aiBehavior = { Turn, 0.0, 90.0, 0, -1 };
            decided = true;
            if (MODE) f << "step 2 (+90)\n";
        }
        else if (aiPrevAct[0].act == Turn && aiPrevAct[0].msg == -1) {
            aiBehavior = { Trek, -90.0, 90.0, 0, -2 };
            decided = true;
            if (MODE) f << "step 3 (+180)\n";
        }
        else if (aiPrevAct[0].act == Trek && aiPrevAct[0].msg == -2) {
            aiBehavior = { Turn, 0.0, 90.0, 0, -2 };
            decided = true;
            if (MODE) f << "step 4 (+270)\n";
        }
        else if (aiPrevAct[0].act == Turn && aiPrevAct[0].msg == -2) {
            aiBehavior = { Turn, 0.0, 90.0, 0, 0 };
            decided = true;
            if (MODE) f << "step 5 (+360)\n";
        }
    }
	if (!decided && AI_VOCATION == MEDIC)
		decided = vDeadTeammate();
    if (!decided && frame > 300)
        decided = vRunPoison();
    if (!decided)
        decided = vPickItem();
    if (!decided)
        decided = vWalkAround();
    if (!decided)
        decided = vStandBy();
    if (!decided)
        aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };
    
	// Debug Info
    if (MODE) {
        if (aiBehavior.act == Undecided) {
            f << ">>> Warning: AI behavior undecided. <<<\n";
        } else {
            f << ">>> Behavior decided: ";
            switch (aiBehavior.act) {
                case Attack: f << "Attack <<<\n"; break;
                case Retreat: f << "Retreat <<<\n"; break;
                case MedSelf: f << "MedSelf <<<\n"; break;
                case MedTeam: f << "MedTeam <<<\n"; break;
                case Radio: f << "Radio <<<\n"; break;
                case Pick: f << "Pick <<<\n"; break;
                case Turn: f << "Turn <<<\n"; break;
                case Trek: f << "Trek <<<\n"; break;
                case Stand: f << "Stand By <<<\n"; break;
            }
        }
    }
    // Debug Info End

    mainfunc();

	return;
}

/* ********** Main Function ********** */
/* *********************************** */





/* ********** general definations ********** */
XYPosition vPolarToXY(PolarPosition pos) {
	XYPosition xy_pos = {
		info.self.xy_pos.x + pos.distance * cos((pos.angle + info.self.view_angle) * M_PI / 180.0),
		info.self.xy_pos.y + pos.distance * sin((pos.angle + info.self.move_angle) * M_PI / 180.0)
	};
    return xy_pos;
}

void vAngleScale(double & angle) {
	while (angle < 0.0) {
		angle += 360.0;
	}
	while (angle > 360.0) {
		angle -= 360.0;
	}
	if (fabs(angle - 360.0) < 0.0001 || fabs(angle - 0.0) < 0.0001)
		angle = 0.0001;
}

inline double vCalcDist(XYPosition pos1, XYPosition pos2) {
	return (pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y);
}


inline double vCalcAngle(XYPosition tgt, XYPosition crt) {
	return atan2(tgt.y - crt.y, tgt.x - crt.x) * 180.0 / M_PI;
}

void mainfunc() {
    // stage 2: execute ai actions
    VSTATUS act = aiBehavior.act;

    // No movement check
    if (isNoMove() &&
        (aiBehavior.act == Trek || aiBehavior.act == Retreat || aiBehavior.act == Turn) &&
        (aiPrevAct[0].act == Trek || aiPrevAct[0].act == Retreat || aiPrevAct[0].act == Turn)) {
        // Debug Info
        if (MODE) f << "No Movement!\n";
        aiBehavior = aiPrevAct[0];
        aiBehavior.view_angle = -65.43;
        aiBehavior.move_angle = -65.43;
        aiBehavior.msg = 0;
    }

    // Check angle parameters
    vAngleScale(aiBehavior.move_angle);
    vAngleScale(aiBehavior.view_angle);

    if (act == Attack) {
        Item weapon = aiFilterWeaponFlag ? aiFilterWeaponCase[0] : aiWeaponCase[0];
        shoot(weapon.type, aiBehavior.move_angle);
        weapon.durability = -1;
        vUpdateWeapon(weapon);
        aiFirstShotFlag = true;
        // Debug Info
        if (MODE) f << "Attack: " << weapon.type << " " << aiBehavior.move_angle << std::endl;
    }
    else if (act == MedSelf) {
        Item med = aiFilterMedFlag ? aiFilterMedCase[0] : aiMedCase[0];
        shoot(med.type, 0);
        med.durability = -1;
        vUpdateMed(med);
        // Debug Info
        if (MODE) f << "MedSelf: " << med.type << std::endl;
    }
    else if (act == MedTeam) {
        Item med = aiFilterMedFlag ? aiFilterMedCase[0] : aiMedCase[0];
        shoot(med.type, 0, aiBehavior.target_ID);
        med.durability = -1;
        vUpdateMed(med);
        // Debug Info
        if (MODE) f << "MedTeam: " << aiBehavior.target_ID << std::endl;
    }
    else if (act == Radio) {
        radio(aiBehavior.target_ID, aiBehavior.msg);
        // Debug Info
        if (MODE) f << "Radio: " << aiBehavior.msg << std::endl;
    }
    else if (act == Retreat) {
        // Temporary 
        move(aiBehavior.move_angle, aiBehavior.view_angle);
    }
    else if (act == Pick) {
        pickup(aiBehavior.target_ID);
        for (int i = 0; i < info.items.size(); ++i) {
            if (info.items[i].item_ID == aiBehavior.target_ID) {
                if (isWeapon(info.items[i].type)) {
                    vUpdateWeapon(info.items[i]);
                }
                else if (isMed(info.items[i].type)) {
                    vUpdateMed(info.items[i]);
                }
                else if (isArmor(info.items[i].type)) {
                    ITEM a = info.items[i].type;
                    aiArmor = aiArmor > a ? aiArmor : a;
                }
                break;
            }
        }
    }
    else if (act == Turn) {
        move(0, aiBehavior.view_angle, NOMOVE);
        // Debug Info
        if (MODE) f << "Turn: " << aiBehavior.view_angle << std::endl;
    }
    else if (act == Trek) {
        move(aiBehavior.move_angle, aiBehavior.view_angle);
        // Debug Info
        if (MODE) f << "Trek: " << aiBehavior.move_angle << " " << aiBehavior.view_angle << std::endl;
    }
    else {
        ;// do nothing
    }



    // after processing
    vClearFilter();
    vClearBehavior();
    aiPrevSelf = info.self;
    // Debug Info
    if (MODE) f << "-\n--\n-\n";

    return;
}





/* ********** item queue ********** */
void vUpdateWeapon(Item weapon) {
    ITEM w = weapon.type;
    int p = 0;
    bool isFind = false;
    for (; p < aiWeaponCase.size(); ++p) {
        if (aiWeaponCase[p].type == w) {
            isFind = true;
            aiWeaponCase[p].durability += weapon.durability;
            weapon.durability = aiWeaponCase[p].durability;
			if (weapon.durability <= 0) {
				aiWeaponCase.erase(aiWeaponCase.begin() + p);
			}
            break;
        }
    }
    if (!isFind) {
        bool isInsert = false;
        weapon.polar_pos = {-1, -1};
        for (p = 0; p < aiWeaponCase.size(); ++p) {
            if (vWeaponPriority[w] >= vWeaponPriority[aiWeaponCase[p].type]) {
                isInsert = true;
                aiWeaponCase.insert(aiWeaponCase.begin() + p, weapon);
                break;
            }
        }
        if (!isInsert){
            aiWeaponCase.push_back(weapon);
        }
    }

	// Debug Info
    if (MODE) {
        f << "Weapon Update: type (" << w << "), place (" << p
            << "), durability (" << weapon.durability << ")\n";
    }
    // Debug Info End
}

void vUpdateMed(Item med) {
    ITEM m = med.type;
    int p = 0;
    bool isFind = false;
    for (; p < aiMedCase.size(); ++p) {
        if (aiMedCase[p].type == m) {
            isFind = true;
            aiMedCase[p].durability += med.durability;
            med.durability = aiMedCase[p].durability;
			if (med.durability == 0) {
				aiMedCase.erase(aiMedCase.begin() + p);
			}
            break;
        }
    }
    if (!isFind) {
        bool isInsert = false;
        med.polar_pos = {-1, -1};
        for (p = 0; p < aiMedCase.size(); ++p) {
            if (vMedPriority[m] >= vMedPriority[aiMedCase[p].type]) {
                isInsert = true;
                aiMedCase.insert(aiMedCase.begin() + p, med);
                break;
            }
        }
        if (!isInsert) {
            aiMedCase.push_back(med);
        }
    }

	// Debug Info
    if (MODE) {
        f << "Med Update: type (" << m << "), place (" << p
            << "), durability (" << med.durability << ")\n";
    }
    // Debug Info End
}

// Temporary
void vInitWeaponPriority() {
    std::vector<ITEM> temp;
    if (AI_VOCATION == SNIPER) {
        temp = {
            FIST, TIGER_BILLOW_HAMMER,
            CROSSBOW, HAND_GUN, 
            SEMI_AUTOMATIC_RILE, SUBMACHINE_GUN, 
            ASSAULT_RIFLE, MACHINE_GUN,
            SNIPER_RILFE, SNIPER_BARRETT 
        };
    } else {
        temp = {
            FIST, TIGER_BILLOW_HAMMER,
            CROSSBOW, HAND_GUN, 
            SEMI_AUTOMATIC_RILE, SUBMACHINE_GUN, 
            SNIPER_RILFE, SNIPER_BARRETT, 
            ASSAULT_RIFLE, MACHINE_GUN
        };
    }

    vWeaponPriority.clear();
    for (int i = 0; i < temp.size(); ++i) {
        vWeaponPriority.insert({temp[i], i});
    }
}

void vInitMedPriority() {
	std::vector<ITEM> temp = {
        BONDAGE, FIRST_AID_CASE
    };

    vMedPriority.clear();
    for (int i = 0; i < temp.size(); ++i) {
        vMedPriority.insert({temp[i], i});
    }
}

// Temporary
void vInitAllPriority() {
    std::vector<ITEM> temp = {
        FIST, TIGER_BILLOW_HAMMER, CROSSBOW, HAND_GUN,
        SCOPE_2, SCOPE_4, SCOPE_8, MUFFLER,
        BONDAGE, SEMI_AUTOMATIC_RILE, SUBMACHINE_GUN,
        VEST_1, INSULATED_CLOTHING, FIRST_AID_CASE,
        SNIPER_RILFE, ASSAULT_RIFLE, SNIPER_BARRETT, MACHINE_GUN,
        VEST_2, VEST_3
    };
    if (AI_VOCATION == HACK)
        temp.push_back(CODE_CASE);
    else
        temp.insert(temp.begin(), CODE_CASE);

    vAllPriority.clear();
    for (int i = 0; i < ITEM_SZ; ++i) {
        vAllPriority.insert({ i, 0 });
    }
    for (int i = 0; i < temp.size(); ++i) {
        vAllPriority[temp[i]] = i;
    }
}

inline bool isWeapon(ITEM w) {
	return ITEM_DATA[w].type == WEAPON;
}

inline bool isArmor(ITEM a) {
    return (ITEM_DATA[a].type == ARMOR && a != INSULATED_CLOTHING);
}

inline bool isMed(ITEM m) {
	return ITEM_DATA[m].param < 0.0;
}

int vGetWeaponDurabilitySum() {
	int sum = 0;
	for (int i = 0; i < aiWeaponCase.size(); ++i) {
		if (aiWeaponCase[i].type != FIST && 
            aiWeaponCase[i].type != TIGER_BILLOW_HAMMER &&
            aiWeaponCase[i].type != CROSSBOW)
			sum += aiWeaponCase[i].durability;
	}
	return sum;
}

void vFilterWeapon(ITEM w) {
	for (int i = 0; i < aiWeaponCase.size(); ++i) {
		if (aiWeaponCase[i].type == w) {
			aiFilterWeaponFlag = true;
			aiFilterWeaponCase.push_back(aiWeaponCase[i]);
			break;
		}
	}
}

void vFilterWeapon(std::vector<ITEM> wVector) {
	for (int i = 0; i < wVector.size(); ++i) {
		for (int j = 0; j < aiWeaponCase.size(); ++j) {
			if (wVector[i] == aiWeaponCase[j].type) {
				aiFilterWeaponFlag = true;
				aiFilterWeaponCase.push_back(aiWeaponCase[i]);
			}
		}
	}
}

void vFilterWeapon(int range) {
	for (int i = 0; i < aiWeaponCase.size(); ++i) {
        ITEM w = aiWeaponCase[i].type;
        if (ITEM_DATA[w].range > range) {
            aiFilterWeaponFlag = true;
            aiFilterWeaponCase.push_back(aiWeaponCase[i]);
        }
	}
}

void vFilterMed(ITEM m) {
	for (int i = 0; i < aiMedCase.size(); ++i) {
		if (aiMedCase[i].type == m) {
			aiFilterMedFlag = true;
			aiFilterMedCase.push_back(aiMedCase[i]);
			break;
		}
	}
}

void vFilterMed(std::vector<ITEM> mVector) {
	for (int i = 0; i < mVector.size(); ++i) {
		for (int j = 0; j < aiMedCase.size(); ++j) {
			if (mVector[i] == aiMedCase[j].type) {
				aiFilterMedFlag = true;
				aiFilterMedCase.push_back(aiMedCase[i]);
			}
		}
	}
}

void vClearFilter() {
	aiFilterWeaponFlag = aiFilterMedFlag = false;
	aiFilterWeaponCase.clear();
	aiFilterMedCase.clear();
}

void vCheckItemStatus() {
    aiArmor = VEST_1 - 1;
	for (int i = 0; i < info.self.bag.size(); ++i) {
        Item item = info.self.bag[i];
		if (item.durability == 0) continue;
        // check weapons
		if (isWeapon(item.type)) {
			bool isFind = false;
			for (int j = 0; j < aiWeaponCase.size(); ++j) {
				if (item.type == aiWeaponCase[j].type) {
					isFind = true;
					int realDur = item.durability;
					int curDur = aiWeaponCase[j].durability;
					if (realDur != curDur) {
						Item temp = item;
						temp.durability = realDur - curDur;
						vUpdateWeapon(temp);
					}
					break;
				}
			}
			if (!isFind) {
				if (item.durability != 0) vUpdateWeapon(item);
			}
		}
        // check armor
        else if (isArmor(item.type)) {
            if (item.type > aiArmor) aiArmor = item.type;
        }
        // check meds
		else if (isMed(item.type)) {
			bool isFind = false;
			for (int j = 0; j < aiMedCase.size(); ++j) {
				if (item.type == aiMedCase[j].type) {
					isFind = true;
					int realDur = item.durability;
					int curDur = aiMedCase[j].durability;
					if (realDur != curDur) {
						Item temp = item;
						temp.durability = realDur - curDur;
						vUpdateMed(temp);
					}
					break;
				}
			}
			if (!isFind) {
				vUpdateMed(item);
			}
		}

	}
}





/* ********** ai behavior ********** */
bool isNoMove() {
	return (
		fabs(aiPrevSelf.xy_pos.x - info.self.xy_pos.x) < 0.01 &&
		fabs(aiPrevSelf.xy_pos.y - info.self.xy_pos.y) < 0.01
	);
}

void vClearBehavior() {
    aiPrevAct.insert(aiPrevAct.begin(), aiBehavior);
    while (aiPrevAct.size() > 5)
        aiPrevAct.pop_back();
	aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };
}





/* ********** ai decision ********** */
// 当遇到敌人时
bool vEncounterEnemy() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    float valueTH = -100.0;     // enemy value thres
    float threatTH = 100.0;     // enemy threat thres

    double angleDT = 35.0;      // change in direction
    double trekTH = 50.0;

    OtherInfo enemy;
    enemy.player_ID = -1;
    double minDist = 1000.0;
    for (int i = 0; i < info.others.size(); ++i) {
        if (info.others[i].status != DEAD && info.others[i].status != REAL_DEAD) {
            bool isFriend = false;
            for (int j = 0; j < teammates.size(); ++j) {
                if (info.others[i].player_ID == teammates[j]) {
                    isFriend = true;
                    break;
                }
            }
            if (!isFriend && info.others[i].polar_pos.distance < minDist) {
                enemy = info.others[i];
                minDist = enemy.polar_pos.distance;
            }
        }
    }

    // 发现敌人
    if (enemy.player_ID != -1) {
        if (info.self.attack_cd != 0) {
			if (aiPrevAct[0].act == Attack && aiWeaponCase[0].type != FIST) {
				aiBehavior = { Trek, enemy.polar_pos.angle, enemy.polar_pos.angle, 0, 0 };
			}
            // Debug Info
			if (MODE) f << "Attack cd!\n";
        } else {
            vFilterWeapon(enemy.polar_pos.distance);
			if (aiFilterWeaponFlag && aiWeaponCase[0].type != FIST) {
				aiBehavior = { Attack, enemy.polar_pos.angle, 0.0, 0, 0 };
				return true;
			}
        }
    }

    return (aiBehavior.act != Undecided);
}

/*
bool vEncounterEnemy() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    float valueTH = -100.0;     // enemy value thres
    float threatTH = 100.0;     // enemy threat thres

    double angleDT = 35.0;      // change in direction
    double trekTH = 50.0;

    // 发现敌人，并且敌人value大于阈值，则Attack
    if (aiEnemy.size() != 0 && aiKV[aiEnemy[0]].value > valueTH) {
        if (info.self.attack_cd != 0) {
            if (aiPrevAct[0].act == Attack && aiWeaponCase[0].type != FIST) {
                aiBehavior = { Trek, aiKV[aiEnemy[0]].rel_polar_pos.angle, aiKV[aiEnemy[0]].rel_polar_pos.angle, 0, 0 };
            }
            // Debug Info
            if (MODE) f << "Attack cd!\n";
        }
        else {
            if (aiWeaponCase[0].type != FIST) {
                aiBehavior = { Attack, aiKV[aiEnemy[0]].rel_polar_pos.angle, 0.0, 0, 0 };
                return true;
            }
        }
    }

    // 发现敌人，并且敌人value小于阈值或threat大于阈值，则Retreat
    if (aiEnemy.size() != 0 && aiKV[aiEnemy[0]].value <= valueTH) {
        aiBehavior = { Retreat, aiKV[aiEnemy[0]].rel_polar_pos.angle + angleDT + 180.0, aiKV[aiEnemy[0]].rel_polar_pos.angle + angleDT, 0, 0 };
        return true;
    }
    if (aiEnemy.size() != 0 && aiKV[aiEnemy[0]].threat > threatTH) {
        aiBehavior = { Retreat, aiKV[aiEnemy[0]].rel_polar_pos.angle + angleDT + 180.0, aiKV[aiEnemy[0]].rel_polar_pos.angle + angleDT, 0, 0 };
        return true;
    }

    return (aiBehavior.act != Undecided);
}
*/

// 当掉血时（现在这里时当血量过低时）
bool vLoseHp() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    double healthTH = info.self.hp_limit * 0.6; // health threshold

    if (info.self.hp < healthTH) {
        if (aiMedCase.size() == 0 || info.self.attack_cd != 0) {
            return false;
        } else {
            aiBehavior = { MedSelf, 0, 0, 0, 0 };
            return true;
        }

        /*
        if (aiEnemy.size() != 0) {
            vFilterMed(FIRST_AID_CASE);
            aiBehavior = { MedSelf, 0, 0, 0, 0 };
            return true;
        } else {
            // 有药就吃，BANDAGE足够奶满优先用BANDAGE（默认）
            if (aiMedCase.size() == 1) {
                aiBehavior = { MedSelf, 0, 0, 0, 0 };
                return true;
            }
            else if (aiMedCase.size() == 2) {
                double sup = -ITEM_DATA[aiMedCase[1].type].param * aiMedCase[1].durability;
                double req = healthTH - info.self.hp;
                if (req > sup)
                    vFilterMed(FIRST_AID_CASE);
                aiBehavior = { MedSelf, 0, 0, 0, 0 };
                return true;
            }
        }
        */
    }

    return (aiBehavior.act != Undecided);
}

bool vDeadTeammate() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

	if (aiMedCase.size() != 0 && aiMedCase[0].type == FIRST_AID_CASE) {
		for (int i = 0; i < aiFriend.size(); ++i) {
			if (aiKV[aiFriend[i]].status == DEAD && aiKV[aiFriend[i]].rel_polar_pos.distance < 100) {
				if (aiKV[aiFriend[i]].rel_polar_pos.distance < PICKUP_DISTANCE) {
					aiBehavior = { MedTeam, 0.0, 0.0, aiFriend[i], 0 };
				} else {
					double angle = GetMoveAngle(info.self.xy_pos, aiKV[aiFriend[i]].abs_xy_pos) - info.self.view_angle;
					aiBehavior = { Trek, angle, angle + TURN, 0, 0 };
				}
			}
		}
	}

    return (aiBehavior.act != Undecided);
}

// 需要跑毒时（不是特别紧急的那种，但是优先级比较高）
bool vRunPoison() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    bool isPoison = fabs(info.poison.next_center.x - 0.0) > 0.1 
        && fabs(info.poison.next_center.y - 0.0) > 0.1;

    if (isPoison) {
		double alpha = (frame > 300 && frame < 1000) ? 0.5 : 0.7;
		double r = info.poison.next_radius * alpha;
		if (vCalcDist(info.self.xy_pos, info.poison.next_center) >= r * r) {
			double mAngle = GetMoveAngle(info.self.xy_pos, info.poison.next_center) - info.self.view_angle;
			double vAngle = mAngle + TURN;
            aiBehavior = { Trek, mAngle, vAngle, 0, 0 };
            if (MAP[GetAreaId(info.self.xy_pos, true)] != CITY) {
                aiBehavior.view_angle = aiBehavior.move_angle;
                aiBehavior.msg = -1;
            }
            // Debug Info
            if (MODE) f << "Run Poison.\n";
            return true;
		}
	}

    return (aiBehavior.act != Undecided);
}

bool vPickItem() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    if (info.items.size() != 0) {
        // 先找到优先级最高的，没武器优先捡武器
        Item target = { 0, FIST, {0, 0}, 0 };
        double max = 0.0;
        int priTH = vAllPriority[BONDAGE];
		double distTH = 16.0;
		bool pFlag = false;
		bool lFlag = (aiPrevAct[0].act == Trek || aiPrevAct[0].act == Turn) && aiPrevAct[0].target_ID != 0;
        bool noweaponFlag = aiWeaponCase[0].type == FIST;
        bool lowweaponFlag = vGetWeaponDurabilitySum() < 20;
        bool existweaponFlag = false;
		for (int i = 0; i < info.items.size(); ++i) {
			Item thisI = info.items[i];

            if (thisI.type == CODE_CASE && AI_VOCATION != HACK)
                continue;

			// 直接捡最近的东西
			if (thisI.polar_pos.distance < 1.0) {
				target = thisI;
				pFlag = true;
				break;
			}

			// 如果上一步有目标就继续执行
			if (lFlag) {
				if (thisI.item_ID == aiPrevAct[0].target_ID) {
					target = thisI;
					pFlag = true;
					break;
				} else {
					continue;
				}
			}

            // 不能捡过远的东西
            if (thisI.polar_pos.distance > distTH) continue;

            // 没枪优先捡枪，枪少优先捡枪
			if (isWeapon(thisI.type) && thisI.type != TIGER_BILLOW_HAMMER) {
                existweaponFlag = true;
			}

            if (existweaponFlag) {
                if (noweaponFlag) {
                    if (fabs(max - 0.0) < 0.01)
                        max = 1000;
                    if (isWeapon(thisI.type) && thisI.polar_pos.distance < max) {
                        max = thisI.polar_pos.distance;
                        target = thisI;
                        continue;
                    }
                } else if (lowweaponFlag) {
                    if (vAllPriority[thisI.type] > max) {
                        max = vAllPriority[thisI.type];
                        target = thisI;
                        continue;
                    }
                }
            }

            if (vAllPriority[thisI.type] > max) {
                max = vAllPriority[thisI.type];
                target = thisI;
            }
        }

        // 判断优先级最高的要不要捡
        if (
            !pFlag && (max >= priTH ||
            isWeapon(target.type) && target.type != FIST ||
            isArmor(target.type) || isMed(target.type))
        ) pFlag = true;

        // 如果要捡，不在范围内则Trek，在范围内则Pick
        if (pFlag) {
            // Debug Info
			if (MODE) f << "Target item: " << target.polar_pos.distance << " " << target.polar_pos.angle << std::endl;
            if (target.polar_pos.distance <= PICKUP_DISTANCE) {
                aiBehavior = { Pick, 0, 0, target.item_ID, 0 };
            } else {
                double angle = target.polar_pos.angle;
                aiBehavior = { Trek, angle, angle, target.item_ID, 0 };
                if (target.polar_pos.distance > 2.0) aiBehavior.msg = -1;
            }
            return true;
        } else {
            return false;
        }
    }

    return (aiBehavior.act != Undecided);
}

bool vPickNearItem() {
	aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

	if (info.items.size() != 0) {
		for (int i = 0; i < info.items.size(); ++i) {
			Item thisI = info.items[i];

            if (thisI.type == CODE_CASE && AI_VOCATION != HACK)
                continue;

			if (thisI.polar_pos.distance < 1.0) {
				double angle = thisI.polar_pos.angle;
				if (thisI.polar_pos.distance < PICKUP_DISTANCE)
					aiBehavior = { Pick, 0, 0, thisI.item_ID, 0 };
				else 
					aiBehavior = { Trek, angle, angle, thisI.item_ID, 0 };
				return true;
			}
		}
	}

	return (aiBehavior.act != Undecided);
}

// 在周围随便走走
bool vWalkAround() {
    // Debug Info
    if (MODE) f << "Walk Around.\n";
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    XYPosition center = info.poison.next_center;
    if (fabs(center.x - 0.0) < 0.1 && fabs(center.y - 0.0) < 0.1) {
        center = { 550.0, 550.0 };
    }

    double alpha = 0.08 + 0.04 * (rand() % 101) / 101.0;
    double angleDT = 100.0;
    double r = info.poison.next_radius * alpha;
    double mAngle = GetMoveAngle(info.self.xy_pos, center) - info.self.view_angle;
    double vAngle = mAngle + TURN;
    if (vCalcDist(info.self.xy_pos, center) >= r * r) {
        // Debug Info
        if (MODE) f << "Center: " << center.x << " " << center.y << std::endl;
        aiBehavior = { Trek, mAngle, vAngle, 0, 0 };
        if (MAP[GetAreaId(info.self.xy_pos, true)] != CITY) {
            aiBehavior.view_angle = aiBehavior.move_angle;
            aiBehavior.msg = -1;
        }
        return true;
    } else {
        // Debug Info
        if (MODE) f << "Close to center.\n";
        /*
        aiBehavior = { Trek, mAngle - angleDT, vAngle - angleDT, 0, 0 };
        if (MAP[GetAreaId(info.self.xy_pos, true)] != CITY) aiBehavior.msg = -1;
        */
        if (vGetWeaponDurabilitySum() > 25 - frame * 4 / 100) {
            aiBehavior = { Turn, 0.0, info.self.view_angle + TURN, 0, 0 };
        } else {
            center.x += (center.x > info.self.xy_pos.x) ? 80 : -80;
            center.y += (center.y > info.self.xy_pos.y) ? 80 : -80;
            mAngle = GetMoveAngle(info.self.xy_pos, center) - info.self.view_angle;
            vAngle = mAngle + TURN;
            aiBehavior = { Trek, mAngle, vAngle, 0, 0 };
            if (MAP[GetAreaId(info.self.xy_pos, true)] != CITY) {
                aiBehavior.view_angle = aiBehavior.move_angle;
                aiBehavior.msg = -1;
            }
        }
        return true;
    }

    return (aiBehavior.act != Undecided);
}

bool vStandBy() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };
    return (aiBehavior.act != Undecided);
}





/* ********** ai evaluation ********** */
/*
// Temporary
void vUpdateEvaluation() {
    double alpha = 0.10 / (0.5 / 50.0); // ratio / (speed / length)
    double beta = 0.6 * (alpha / 9.0);  // ratio * (alpha / blocks)

    double ori[8][12];
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 12; ++j) {
            ori[i][j] = aiThreatMap[i][j];
        }
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 12; ++j) {
			aiThreatMap[i][j] *= alpha;
            for (int m = ((i==0) ? i : i-1); m <= ((i==7) ? i : i+1); ++m) {
                for (int n = ((j==0) ? j : j-1); n <= ((j==11) ? j : j+1); ++n) {
                    if (m != i && m != j)
						aiThreatMap[i][j] += ori[m][n] * beta;
                }
            }
        }
    }

	double crtAngle = vCalcAngle(info.poison.current_center, info.self.xy_pos);
	double nxtAngle = vCalcAngle(info.poison.next_center, info.self.xy_pos);
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 12; ++j) {
			double R;
			double r = 25 + i * 50;
			double angle = j * 30;
			if (fabs(angle - crtAngle) < 25.0) {
				R = sqrt(vCalcDist(info.self.xy_pos, info.poison.next_center)) - r * r;
				R *= R;
			} else if (fabs(angle - crtAngle) > 65.0) {
				R = sqrt(vCalcDist(info.self.xy_pos, info.poison.next_center)) + r;
				R *= R;
			} else {
				R = vCalcDist(info.self.xy_pos, info.poison.next_center) + r * r;
			}
			if (R >= info.poison.current_radius * info.poison.current_radius) {
				aiPoisonMap[i][j] = 100.0;
			} else if (R >= info.poison.current_radius * info.poison.current_radius - 100.0) {
				aiPoisonMap[i][j] = 50.0;
			} else {
				aiPoisonMap[i][j] = 0.0;
			}
		}
	}

	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 12; ++j) {
			aiEntireMap[i][j] = aiThreatMap[i][j] + aiPoisonMap[i][j];
		}
	}
}
*/





/* ********** ai audio ********** */
// Unused
void vAudioHandler() {
    for (int i = 0; i < info.sounds.size(); ++i) {
        if (info.sounds[i].sender == -1) {
            /*
            Sound sound = info.sounds[i];
            int fdist = SOUND_DATA[sound.type].speed * sound.delay;
            int index = fdist / 50 - 1;
            for (int j = 0; j < 12; ++j)
                aiThreatMap[index][j] = 100.0;
            */
            ;
        } else {
            vRadioHandler(info.sounds[i]);
        }
    }
}

void vRadioHandler(Sound radio) {
	;
}





/* ********** ai sight ********** */
void vSightHandler() {
    // process player info
    for (int i = 0; i < info.others.size(); ++i) {
        vDetectPlayer(i);
    }
	vLostEnemy();

	// Debug Info
    if (MODE) {
		if (info.others.size() != 0) {
			f << "AI in sight: \n";
			for (int i = 0; i < info.others.size(); ++i) {
				OtherInfo thisAi = info.others[i];
				if (i == 0) f << "\t";
				else f << "; ";
				f << "(id: " << thisAi.player_ID << ", "
					<< "status: " << thisAi.status << ") ";
			}
			f << std::endl;
		}
        if (aiEnemy.size() != 0) {
            f << "Enemy Queue: \n";
            for (int i = 0; i < aiEnemy.size(); ++i) {
                int id = aiEnemy[i];
                f << "\tEnemy (id: " << id << "): "
                    << "frame (" << aiKV[id].lastUpdateFrame << "), "
                    << "priority (" << aiKV[id].priority << "), "
                    << "rel_angle (" << aiKV[id].rel_polar_pos.angle << "), "
                    << "rel_dist (" << aiKV[id].rel_polar_pos.distance << ")\n"; 
            }
        }
    }
    // Debug Info End
}

// update real-time location, insert into kv table, update enemy queue
void vDetectPlayer(int index) {
    OtherInfo tInfo = info.others[index];
    XYPosition xy_pos;
    float threat = 0, value = 0, priority = 0;

    xy_pos = vPolarToXY(tInfo.polar_pos);

    vAiInfo vInfo = {
        frame, tInfo.player_ID, value, threat, priority,
		tInfo.vocation, tInfo.status,
        tInfo.move_angle, tInfo.view_angle, tInfo.move_speed,
        tInfo.polar_pos, xy_pos
    };

    vCalcEnemyPriority(vInfo);

    bool isFriend = false;
    for (int i = 0; i < teammates.size(); ++i) {
        if (tInfo.player_ID == teammates[i]) {
            isFriend = true;
            break;
        }
    }

    if (aiKV.find(tInfo.player_ID) != aiKV.end()) {
        aiKV[tInfo.player_ID] = vInfo;
    } else {
        aiKV.insert({tInfo.player_ID, vInfo});
        if (isFriend)
            aiFriend.push_back(tInfo.player_ID);
    }
	if (!isFriend)
        vUpdateEnemy(tInfo.player_ID);
}

// update enemy queue according to priority
void vUpdateEnemy(int id) {
	int pri = aiKV[id].priority;
	bool flag = false;
	for (int i = 0; i < aiEnemy.size(); ++i) {
		if (aiEnemy[i] == id) {
			aiEnemy.erase(aiEnemy.begin() + i);
            break;
		}
	}
	for (int i = 0; i < aiEnemy.size(); ++i) {
		if (pri >= aiKV[aiEnemy[i]].priority) {
			flag = true;
			aiEnemy.insert(aiEnemy.begin() + i, id);
			break;
		}
	}
	if (!flag) {
		aiEnemy.push_back(id);
	}
}

// check if enemy lost in sight or dead, update its priority
void vLostEnemy() {
	for (int i = 0; i < aiEnemy.size(); ++i) {
		int id = aiEnemy[i];
		if (aiKV[id].lastUpdateFrame != frame) {
			// Temporary
			// set priority = 0, update enemy queue
			aiKV[id].priority = 0;
			aiKV[id].value = -1000000;
			aiEnemy.erase(aiEnemy.begin() + i);

			// Debug Info
			if (MODE) f << "Enemy (id: " << id << ") lost in sight.\n";
		}
	}
    for (int i = 0; i < aiEnemy.size(); ++i) {
        int id = aiEnemy[i];
        if (aiKV[id].status == DEAD || aiKV[id].status == REAL_DEAD) {
            aiEnemy.erase(aiEnemy.begin() + i);

            // Debug Info
            if (MODE) f << "Enemy (id: " << id << ") is dead.\n";
        }
    }
}

void vCalcEnemyPriority(vAiInfo & ai) {
	if (ai.status == DEAD || ai.status == REAL_DEAD) {
		ai.threat = -1000000.0;
		ai.value = -1000000.0;
	} else {
		if (aiKV.find(ai.player_ID) != aiKV.end() && aiKV[ai.player_ID].lastUpdateFrame == frame - 1) {
			ai.threat = aiKV[ai.player_ID].threat + 2.5;
			ai.value = aiKV[ai.player_ID].value + 2.5;
		} else if (ai.vocation == SNIPER) {
			ai.threat = 50.0;
			ai.value = 25.0;
		} else if (ai.vocation == MEDIC) {
			ai.threat = 10.0;
			ai.value = 100.0;
		} else if (ai.vocation == SIGNALMAN) {
			ai.threat = 25.0;
			ai.value = 60.0;
		} else {
			ai.threat = 25.0;
			ai.value = 0.0;
		}
	}
	ai.priority = ai.value + ai.threat;
}



// finding path begin (zms)

int GetAreaId(XYPosition CurPosition) {
    int _x = CurPosition.x + (fabs(info.self.move_angle) > 90 ? 1.6 : -1.6);
	int _y = CurPosition.y + (info.self.move_angle < 0 ? 1.6 : -1.6);
	int q_x = _x / 100 , r_x = _x % 100;
	int q_y = _y / 100 , r_y = _y % 100;
	int id = q_y * 10 + q_x;
	return id;
}

int GetAreaId(XYPosition CurPosition, bool nothing) {
    int _x = CurPosition.x;
    int _y = CurPosition.y;
    int q_x = _x / 100, r_x = _x % 100;
    int q_y = _y / 100, r_y = _y % 100;
    int id = q_y * 10 + q_x;
    return id;
}

double GetMoveAngle(XYPosition CurPosition, XYPosition TargetPosition) {
	// directly
	if(false) {
		return vCalcAngle(TargetPosition,CurPosition);
	}
	int cur_id = GetAreaId(CurPosition);
	int tar_id = GetAreaId(TargetPosition, true);
    if (cur_id == tar_id)
        return GetMoveAngle_small(CurPosition, TargetPosition, cur_id);
    else
        return GetMoveAngle_big(CurPosition, TargetPosition, cur_id, tar_id);

}

double zms_dis(double x1, double y1, double x2, double y2) {
	x1-=x2;
	y1-=y2;
	return sqrt(x1*x1+y1*y1);
}

int get_BeachNodeId(double cur_x, double cur_y) {
	double min_dis = 1e9; //inf
	int id = -1;
	for (int i = 0; i < BeachNode; ++i) {
        double dis = zms_dis(cur_x, cur_y, BeachLoc[i][0], BeachLoc[i][1]);
		if (dis < min_dis) id = i, min_dis = dis;
	}
	return id;
}

int get_CityNodeId(double cur_x, double cur_y) {
	double min_dis = 1e9; //inf
	int id = -1;
	bool flag = false;
	if ((cur_x < 5.5 || cur_x > 94.5) || (cur_y < 5.5 || cur_y > 94.5)) flag = true;
	for (int i = 0; i < CityNode; ++i) {
		if (flag && (i >= 7 && i != 23)) continue;
		if (!flag && (i == 0 || i == 2 || i == 4 || i == 23)) continue;
        double dis = zms_dis(cur_x, cur_y, CityLoc[i][0], CityLoc[i][1]);
		if (dis < min_dis) {
			id = i;
			min_dis = dis;
		}
	}
	return id;
}

int get_HillNodeId(double cur_x,double cur_y) {
	double min_dis = 1e9; //inf
	int id = -1;
	for (int i = 0; i < HillNode; ++i) {
        double dis = zms_dis(cur_x, cur_y, HillLoc[i][0], HillLoc[i][1]);
        if (i == 2 && dis > 12.0) continue;
		if (dis < min_dis) id = i, min_dis = dis;
	}
	return id;
}

int get_FarmlandNodeId(double cur_x, double cur_y) {
	if (cur_y <= 50)
		return 0;
	else
		return 1;
}

int get_PoolNodeId(double cur_x, double cur_y) {
    double min_dis = 1e9; //inf
    int id = -1;
    for (int i = 0; i < PoolNode; ++i) {
        double dis = zms_dis(cur_x, cur_y, PoolLoc[i][0], PoolLoc[i][1]);
        if (cur_x <= 30 && i % 3 != 0) continue;
        if (cur_x > 30 && cur_x < 75 && i % 3 != 1) continue;
        if (cur_x >= 75 && i % 3 != 2) continue;
        if (dis < min_dis) id = i, min_dis = dis;
    }
    return id;
}

double GetMoveAngle_small(XYPosition CurPosition, XYPosition TargetPosition, int id) {
	int delta_x = id % 10 * 100;
	int delta_y = id / 10 * 100;
	
	double cur_x = CurPosition.x - delta_x;
	double cur_y = CurPosition.y - delta_y;
	XYPosition Cur;
	Cur.x = cur_x;
	Cur.y = cur_y;

	double tar_x = TargetPosition.x - delta_x;
	double tar_y = TargetPosition.y - delta_y;
	XYPosition Tar;
	Tar.x = tar_x;
	Tar.y = tar_y;

	AREA type = MAP[id];

	if (type == GRASS || type == ROADA || type == ROADB || type == FOREST) {
		return vCalcAngle(Tar, Cur);
	}

	else if (type == BEACH) {
		double dis = sqrt(vCalcDist(Tar, Cur));
		if (dis < 2) return vCalcAngle(Tar, Cur);
		int cur_BeachNodeId = get_BeachNodeId(cur_x,cur_y);
		int tar_BeachNodeId = get_BeachNodeId(tar_x,tar_y);

		XYPosition node[BeachNode];
		for(int i = 0; i < BeachNode; ++i)
			node[i].x = BeachLoc[i][0], node[i].y = BeachLoc[i][1];
		XYPosition s = node[cur_BeachNodeId];
		XYPosition t;
		if (cur_BeachNodeId == tar_BeachNodeId) {
			t = Tar;
		}
		else {
			if (BeachAdj[cur_BeachNodeId][tar_BeachNodeId] != -1) {

				t = node[tar_BeachNodeId];
			}
			else {
				t = node[BeachNext[cur_BeachNodeId][tar_BeachNodeId]];
			}
		}
		double tmp = vCalcAngle(Cur, t) - vCalcAngle(s, t);
		vAngleScale(tmp);
		double angle = fabs(tmp);

		if (angle < 2.5 || angle > 357.5) return vCalcAngle(t, Cur);
		else return vCalcAngle(s, Cur);
		
	}

	else if (type == CITY) {
		double dis = sqrt(vCalcDist(Tar, Cur));
		//if (dis < 2) return vCalcAngle(Tar, Cur);
		int cur_CityNodeId = get_CityNodeId(cur_x, cur_y);
		int tar_CityNodeId = get_CityNodeId(tar_x, tar_y);
        // Debug Info
		if (MODE) f << "(City) curNode: "<< cur_CityNodeId << " tarNode: " << tar_CityNodeId << std::endl;

		XYPosition node[CityNode];
		for (int i = 0; i < CityNode; ++i)
			node[i].x = CityLoc[i][0], node[i].y = CityLoc[i][1];
		XYPosition s = node[cur_CityNodeId];
		XYPosition t;
		if (cur_CityNodeId == tar_CityNodeId) {
			t = Tar;
		}
		else {
			if (CityAdj[cur_CityNodeId][tar_CityNodeId] != -1) {

				t = node[tar_CityNodeId];
			}
			else {
				t = node[CityNext[cur_CityNodeId][tar_CityNodeId]];
			}
		}
		double tmp = vCalcAngle(Cur, t) - vCalcAngle(s, t);
		vAngleScale(tmp);
		double angle = fabs(tmp);

		if (angle < 1.6 || angle > 358.4) return vCalcAngle(t, Cur);
		else return vCalcAngle(s, Cur);
	}

	else if (type == HILL) {
		double dis = sqrt(vCalcDist(Tar, Cur));
		if (dis < 2) return vCalcAngle(Tar, Cur);
		int cur_HillNodeId = get_HillNodeId(cur_x, cur_y);
		int tar_HillNodeId = get_HillNodeId(tar_x, tar_y);
        // Debug Info
        if (MODE) f << "(Hill) curNode: " << cur_HillNodeId << " tarNode: " << tar_HillNodeId << std::endl;

		XYPosition node[HillNode];
		for (int i = 0; i < HillNode; ++i)
			node[i].x = HillLoc[i][0], node[i].y = HillLoc[i][1];
		XYPosition s = node[cur_HillNodeId];
		XYPosition t;
		if (cur_HillNodeId == tar_HillNodeId) {
			t = Tar;
		}
		else {
			if (HillAdj[cur_HillNodeId][tar_HillNodeId] != -1) {

				t = node[tar_HillNodeId];
			}
			else {
				t = node[HillNext[cur_HillNodeId][tar_HillNodeId]];
			}
		}
		double tmp = vCalcAngle(Cur, t) - vCalcAngle(s, t);
		vAngleScale(tmp);
		double angle = fabs(tmp);

		if (angle < 2.5 || angle > 357.5) return vCalcAngle(t, Cur);
		else return vCalcAngle(s, Cur);
	}

	else if (type == FARMLAND) {
		double dis = sqrt(vCalcDist(Tar, Cur));
		if (dis < 2 || (Tar.x <= 75 && Cur.x <= 75)) return vCalcAngle(Tar, Cur);
		int cur_FarmlandNodeId = get_FarmlandNodeId(cur_x, cur_y);
		int tar_FarmlandNodeId = get_FarmlandNodeId(tar_x, tar_y);
        // Debug Info
        if (MODE) f << "(Farmland) curNode: " << cur_FarmlandNodeId << " tarNode: " << tar_FarmlandNodeId << std::endl;

		XYPosition node[FarmlandNode];
		for (int i = 0; i < FarmlandNode; ++i)
			node[i].x = FarmlandLoc[i][0], node[i].y = FarmlandLoc[i][1];
		XYPosition s = node[cur_FarmlandNodeId];
		XYPosition t;
		if (cur_FarmlandNodeId == tar_FarmlandNodeId) {
			t = Tar;
		}
		else {
			if (FarmlandAdj[cur_FarmlandNodeId][tar_FarmlandNodeId] != -1) {

				t = node[tar_FarmlandNodeId];
			}
			else {
				t = node[FarmlandNext[cur_FarmlandNodeId][tar_FarmlandNodeId]];
			}
		}
		double tmp = vCalcAngle(Cur, t) - vCalcAngle(s, t);
		vAngleScale(tmp);
		double angle = fabs(tmp);

		if (angle < 2.5 || angle > 357.5) return vCalcAngle(t, Cur);
		else return vCalcAngle(s, Cur);
	}

    else if (type == POOL) {
        double dis = sqrt(vCalcDist(Tar, Cur));
        if (dis < 2) return vCalcAngle(Tar, Cur);
        int cur_PoolNodeId = get_PoolNodeId(cur_x, cur_y);
        int tar_PoolNodeId = get_PoolNodeId(tar_x, tar_y);

        if (MODE) f << cur_PoolNodeId << " " << tar_PoolNodeId << std::endl;

        XYPosition node[PoolNode];
        for (int i = 0; i < PoolNode; ++i)
            node[i].x = PoolLoc[i][0], node[i].y = PoolLoc[i][1];
        XYPosition s = node[cur_PoolNodeId];
        XYPosition t;
        if (cur_PoolNodeId == tar_PoolNodeId) {
            t = Tar;
        }
        else {
            if (PoolAdj[cur_PoolNodeId][tar_PoolNodeId] != -1) {

                t = node[tar_PoolNodeId];
            }
            else {
                t = node[PoolNext[cur_PoolNodeId][tar_PoolNodeId]];
            }
        }
        double tmp = vCalcAngle(Cur, t) - vCalcAngle(s, t);
        vAngleScale(tmp);
        double angle = fabs(tmp);

        if (angle < 2.5 || angle > 357.5) return vCalcAngle(t, Cur);
        else return vCalcAngle(s, Cur);

    }

    else {
        return vCalcAngle(Tar, Cur);
    }
}

int getAreaLevel(int id) {
	if (id < 0 || id >= 100) return -1;
	if( MAP[id] == GRASS) return 1;
	if( MAP[id] == ROADA) return 3;
	if( MAP[id] == ROADB) return 3;
	if( MAP[id] == FOREST) return 1;
	if( MAP[id] == FARMLAND) return 1;
	if( MAP[id] == POOL) return 0;
	if( MAP[id] == BEACH) return 2;
	if( MAP[id] == CITY) return 0;
	if( MAP[id] == HILL) return 3;
	return -1;
}

int max(int x, int y) {
	return x > y ? x : y;
}

double GetMoveAngle_big(XYPosition CurPosition, XYPosition TargetPosition, int cur_id, int tar_id) {
	int cur_x = cur_id % 10;
	int cur_y = cur_id / 10;
	int tar_x = tar_id % 10;
	int tar_y = tar_id / 10;
	int len = abs(cur_x - tar_x) + abs(cur_y - tar_y);
	XYPosition O;
	O = CurPosition;
	int xx2 = floor(CurPosition.x);
	int yy2 = floor(CurPosition.y);
    double xx = CurPosition.x - (xx2 / 100) * 100;
    double yy = CurPosition.y - (yy2 / 100) * 100;

    XYPosition Right = CurPosition;
    Right.x += 102 - xx;
    XYPosition Left = CurPosition;
    Left.x -= xx + 2;
    XYPosition Up = CurPosition;
    Up.y += 102 - yy;
    XYPosition Down = CurPosition;
    Down.y -= yy + 2;
	XYPosition Up_Right = CurPosition;
	Up_Right.x += 102 - xx;
	Up_Right.y += 102 - yy;
    XYPosition Up_Left = CurPosition;
    Up_Left.x -= xx + 2;
    Up_Left.y += 102 - yy;
	XYPosition Down_Right = CurPosition;
	Down_Right.x += 102 - xx;
	Down_Right.y -= yy + 2;
	XYPosition Down_Left = CurPosition;
	Down_Left.x -= xx + 2;
	Down_Left.y -= yy + 2;

    // Debug Info
    if (MODE) f << "(big) " << cur_id << " -> " << tar_id << std::endl;

	if (len == 1) {
        if (cur_id == tar_id + 1) {
            if (MAP[cur_id - 1] == FARMLAND) {
                if (vCalcDist(info.self.xy_pos, { 0, 65 }) < vCalcDist(info.self.xy_pos, { 0, 35 })) {
                    return GetMoveAngle_small(O, { -2, 65 }, cur_id);
                } else {
                    return GetMoveAngle_small(O, { -2, 35 }, cur_id);
                }
            }
            return GetMoveAngle_small(O, Left, cur_id);
        }
        if (cur_id == tar_id - 1) return GetMoveAngle_small(O, Right, cur_id);
        if (cur_id == tar_id + 10) return GetMoveAngle_small(O, Down, cur_id);
        if (cur_id == tar_id - 10) return GetMoveAngle_small(O, Up, cur_id);
	}
    if (cur_x <= tar_x && cur_y == tar_y) {
		int cur_right = cur_id + 1;
		int cur_up_right = cur_id + 11;
		int cur_down_right = cur_id - 9;
		int v_right = getAreaLevel(cur_right);
		int v_up_right = getAreaLevel(cur_up_right);
		int v_down_right = getAreaLevel(cur_down_right);

		int mx = max(v_right, max(v_up_right, v_down_right));
		if(v_right == mx) return GetMoveAngle_small(O,Right,cur_id);
		if(v_up_right == mx) return GetMoveAngle_small(O,Up_Right,cur_id);
		if(v_down_right == mx) return GetMoveAngle_small(O, Down_Right, cur_id);

	}
	
    if (cur_x >= tar_x && cur_y == tar_y) {
		int cur_left = cur_id - 1;
		int cur_up_left = cur_id + 9;
		int cur_down_left = cur_id - 11;
		int v_left = getAreaLevel(cur_left);
		int v_up_left = getAreaLevel(cur_up_left);
		int v_down_left = getAreaLevel(cur_down_left);

		if (MAP[cur_id - 1] == FARMLAND) v_left = -100;

		int mx = max(v_left, max(v_up_left, v_down_left));
		if(v_left == mx) return GetMoveAngle_small(O, Left, cur_id);
		if(v_up_left == mx) return GetMoveAngle_small(O, Up_Left, cur_id);
		if(v_down_left == mx) return GetMoveAngle_small(O, Down_Left, cur_id);

	}
	
	if (cur_x == tar_x && cur_y <= tar_y) {
		int cur_up = cur_id + 10;
		int cur_up_right = cur_id + 11;
		int cur_up_left = cur_id + 9;
		int v_up = getAreaLevel(cur_up);
		int v_up_right = getAreaLevel(cur_up_right);
		int v_up_left = getAreaLevel(cur_up_left);

		int mx = max(v_up, max(v_up_right, v_up_left));
		if(v_up == mx) return GetMoveAngle_small(O, Up, cur_id);
		if(v_up_right == mx) return GetMoveAngle_small(O, Up_Right, cur_id);
		if(v_up_left == mx) return GetMoveAngle_small(O, Up_Left, cur_id);

	}
	
	if (cur_x == tar_x && cur_y >= tar_y) {
		int cur_down = cur_id - 10;
		int cur_down_left = cur_id - 11;
		int cur_down_right = cur_id - 9;
		int v_down = getAreaLevel(cur_down);
		int v_down_left = getAreaLevel(cur_down_left);
		int v_down_right = getAreaLevel(cur_down_right);

		int mx = max(v_down, max(v_down_left, v_down_right));
		if(v_down == mx) return GetMoveAngle_small(O, Down, cur_id);
		if(v_down_left == mx) return GetMoveAngle_small(O, Down_Left, cur_id);
		if(v_down_right == mx) return GetMoveAngle_small(O, Down_Right, cur_id);

	}
	
	if (cur_x >= tar_x && cur_y <= tar_y) {
		int cur_left = cur_id - 1;
		int cur_up_left = cur_id + 9;
		int cur_up = cur_id + 10;
		int v_left = getAreaLevel(cur_left);
		int v_up_left = getAreaLevel(cur_up_left) + getAreaLevel(cur_up) + getAreaLevel(cur_left);
		int v_up = getAreaLevel(cur_up);

		if (MAP[cur_id - 1] == FARMLAND) v_left = -100;

		int mx = max(v_left, max(v_up_left, v_up));
		if(v_left == mx) return GetMoveAngle_small(O, Left, cur_id);
		if(v_up_left == mx) return GetMoveAngle_small(O, Up_Left, cur_id);
		if(v_up == mx) return GetMoveAngle_small(O, Up, cur_id);

	}
	
	if (cur_x >= tar_x && cur_y >= tar_y) {
		int cur_left = cur_id - 1;
		int cur_down_left = cur_id - 11;
		int cur_down = cur_id - 10;
		int v_left = getAreaLevel(cur_left);
		int v_down_left = getAreaLevel(cur_down_left) + getAreaLevel(cur_down) + getAreaLevel(cur_left);
		int v_down = getAreaLevel(cur_down);

		if (MAP[cur_id - 1] == FARMLAND) v_left = -100;

		int mx = max(v_left, max(v_down_left, v_down));
		if(v_left == mx) return GetMoveAngle_small(O, Left, cur_id);
		if(v_down_left == mx) return GetMoveAngle_small(O, Down_Left, cur_id);
		if(v_down == mx) return GetMoveAngle_small(O, Down, cur_id);

	}
	
	if (cur_x <= tar_x && cur_y <= tar_y) {
		int cur_right = cur_id + 1;
		int cur_up_right = cur_id + 11;
		int cur_up = cur_id + 10;
		int v_right = getAreaLevel(cur_right);
		int v_up_right = getAreaLevel(cur_up_right) + getAreaLevel(cur_up) + getAreaLevel(cur_right);
		int v_up = getAreaLevel(cur_up);

		int mx = max(v_right, max(v_up_right, v_up));
		if(v_right == mx) return GetMoveAngle_small(O, Right, cur_id);
		if(v_up_right == mx) return GetMoveAngle_small(O, Up_Right, cur_id);
		if(v_up == mx) return GetMoveAngle_small(O, Up, cur_id);

	}
	
	if (cur_x <= tar_x && cur_y >= tar_y) {
		int cur_right = cur_id + 1;
		int cur_down = cur_id - 10;
		int cur_down_right = cur_id - 9;
		int v_right = getAreaLevel(cur_right);
		int v_down = getAreaLevel(cur_down);
		int v_down_right = getAreaLevel(cur_down_right) + getAreaLevel(cur_down) + getAreaLevel(cur_right);

		int mx = max(v_right, max(v_down, v_down_right));
		if(v_right == mx) return GetMoveAngle_small(O, Right, cur_id);
		if(v_down == mx) return GetMoveAngle_small(O, Down, cur_id);
		if(v_down_right == mx) return GetMoveAngle_small(O, Down_Right, cur_id);

	}

	return vCalcAngle(TargetPosition,CurPosition);
}

// finding path end (zms)
