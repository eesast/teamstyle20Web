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

#define M_PI 3.14159265358979

// 0 - MEDIC, 1 - SIGNALMAN, 2 - HACK, 3 - SNIPER
#define AI_VOCATION MEDIC 

#define REFRESH_PERIOD 5
#define REFRESH_PHASE 4

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

// Debug Info
/*
char * fname;
sprintf("./playback/%d.txt", fname, AI_VOCATION)
std::fstream f(fname, std::ios::out);
*/



// general functions 
XYPosition vPolarToXY(PolarPosition);
void vAngleScale(double &);
inline double vCalcDist(XYPosition, XYPosition);
inline double vCalcAngle(XYPosition, XYPosition);



// item queue
// caution: 'Item' is the struct of an item, 'ITEM' is the type
std::vector<Item> aiWeaponCase;
std::vector<Item> aiFilterWeaponCase;   // temporary usage
std::vector<Item> aiMedCase;
std::vector<Item> aiFilterMedCase;      // temporary usage
int aiArmor = VEST_1 - 1;
std::unordered_map<ITEM, int> vWeaponPriority;	// pre-decided
std::unordered_map<ITEM, int> vMedPriority;		// pre-decided
std::unordered_map<ITEM, int> vAllPriority;		// pre-decided
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
void vRadioEncode();
void vRadioDecode();



// ai sight: provide around evaluation ( and entire evaluation )
struct vAiInfo {
    int lastUpdateFrame;
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

    // Debug Info
	/*
	f << "frame (" << frame << ") hp (" << info.self.hp
		<< ") view_angle (" << info.self.view_angle << ") move_angle (" << vCalcAngle(info.self.xy_pos, aiPrevSelf.xy_pos)
		<< ") \npos (" << info.self.xy_pos.x << " " << info.self.xy_pos.y
		<< ") poison (" << info.poison.next_center.x << " " << info.poison.next_center.y 
		<< ") poison_dist (" << sqrt(vCalcDist(info.self.xy_pos, info.poison.next_center)) << ") " << std::endl;
	*/

    // check weapon and med queue
    if (frame % REFRESH_PERIOD == REFRESH_PHASE) {
        vCheckItemStatus();

		// Debug Info
		/*
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
			f << "None\n";
		} else {
			for (int i = 0; i < aiMedCase.size(); ++i) {
				Item m = aiMedCase[i];
				f << "\tMed (type: " << m.type << "): "
					<< "durability (" << m.durability << ")\n";
			}
		}
		*/
    }
    
    // landing
	if (frame == 0) {
		srand(time(nullptr) + teammates[0]);
		/*
		XYPosition landing_point = {
			start_pos.x * 0.35 + over_pos.x * 0.65 + rand() % 20, 
			start_pos.y * 0.35 + over_pos.y * 0.65 + rand() % 20
		};
		*/
		XYPosition landing_point = {
			350 + rand() % 20, 350 + rand() % 20
		};
		parachute(AI_VOCATION, landing_point);
        
        vInitWeaponPriority();
        vInitMedPriority();
        vInitAllPriority();

		aiWeaponCase.push_back({ 0, FIST, {-1, -1}, 9999 });

		aiPrevAct.push_back({ Undecided, 0, 0, 0, 0 });

		return;
	}

	// Debug Info
	/*
	if (info.self.status == ON_PLANE || info.self.status == JUMPING) {
		f << "jumping" << std::endl;
		return;
	}
	*/



    // stage 0: evaluate battle field status
//	vUpdateEvaluation();
    vAudioHandler();
    vSightHandler();



	// stage 1: decide ai behavior & parameters
    // Temporary
    bool decided = false;

    if (!decided && AI_VOCATION == MEDIC)
        decided = vDeadTeammate();
    if (!decided)
        decided = vEncounterEnemy();
    if (!decided)
        decided = vLoseHp();
    if (!decided)
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
	/*
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
	*/



    // stage 2: execute ai actions
    VSTATUS act = aiBehavior.act;

	// No movement check
	if (aiBehavior.act == Trek && aiPrevAct[0].act == Trek && isNoMove()) {
		// Debug Info
//		f << "No Movement!\n";
		aiBehavior = aiPrevAct[0];
		aiBehavior.view_angle = -43.21;
		aiBehavior.move_angle = -43.21;
	}

    // Check angle parameters
	vAngleScale(aiBehavior.move_angle);
	vAngleScale(aiBehavior.view_angle);

    if (act == Attack) {
        Item weapon = aiFilterWeaponFlag ? aiFilterWeaponCase[0] : aiWeaponCase[0];
        shoot(weapon.type, aiBehavior.move_angle);
        weapon.durability = -1;
        vUpdateWeapon(weapon);
		// Debug Info
//		f << "Attack: " << weapon.type << " " << aiBehavior.move_angle << std::endl;
    } else if (act == MedSelf) {
        Item med = aiFilterMedFlag ? aiFilterMedCase[0] : aiMedCase[0];
        shoot(med.type, 0);
        med.durability = -1;
        vUpdateMed(med);
		// Debug Info
//		f << "MedSelf: " << med.type << std::endl;
    } else if (act == MedTeam) {
        Item med = aiFilterMedFlag ? aiFilterMedCase[0] : aiMedCase[0];
        shoot(med.type, 0, aiBehavior.target_ID);
        med.durability = -1;
        vUpdateMed(med);
		// Debug Info
//		f << "MedTeam: " << aiBehavior.target_ID << std::endl;
    } else if (act == Radio) {
        radio(aiBehavior.target_ID, aiBehavior.msg);
		// Debug Info
//		f << "Radio: " << aiBehavior.msg << std::endl;
    } else if (act == Retreat) {
        // Temporary 
        move(aiBehavior.view_angle + 180.0, aiBehavior.view_angle);
    } else if (act == Pick) {
        pickup(aiBehavior.target_ID);
        for (int i = 0; i < info.items.size(); ++i) {
            if (info.items[i].item_ID == aiBehavior.target_ID) {
                if (isWeapon(info.items[i].type)) {
                    vUpdateWeapon(info.items[i]);
                } else if (isMed(info.items[i].type)) {
                    vUpdateMed(info.items[i]);
                } else if (isArmor(info.items[i].type)) {
                    ITEM a = info.items[i].type;
                    aiArmor = aiArmor > a ? aiArmor : a;
                }
                break;
            }
        }
    } else if (act == Turn) {
        move(0, aiBehavior.view_angle, NOMOVE);
		// Debug Info
//		f << "Turn: " << aiBehavior.view_angle << std::endl;
    } else if (act == Trek) {
        move(aiBehavior.move_angle, aiBehavior.view_angle);
		// Debug Info
//		f << "Trek: " << aiBehavior.move_angle << " " << aiBehavior.view_angle << std::endl;
    } else {
		;// do nothing
    }



	// after processing
	vClearFilter();
	vClearBehavior();
    aiPrevSelf = info.self;
	// Debug Info
//	f << "-\n--\n-\n";

	return;
}

/* ********** Main Function ********** */
/* *********************************** */





/* ********** general definations ********** */
XYPosition vPolarToXY(PolarPosition pos) {
	XYPosition xy_pos = {
		info.self.xy_pos.x + pos.distance * cos(pos.angle * M_PI / 180.0),
		info.self.xy_pos.y + pos.distance * sin(pos.angle * M_PI / 180.0)
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
			if (weapon.durability == 0) {
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
	/*
    f << "Weapon Update: type (" << w << "), place (" << p
        << "), durability (" << weapon.durability << ")\n";
	*/
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
	/*
    f << "Med Update: type (" << m << "), place (" << p
        << "), durability (" << med.durability << ")\n";
	*/
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
        FIRST_AID_CASE, BONDAGE
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
        SEMI_AUTOMATIC_RILE, BONDAGE, SUBMACHINE_GUN, FIRST_AID_CASE,
        SCOPE_2, VEST_1, SCOPE_4, VEST_2, INSULATED_CLOTHING,
        SNIPER_RILFE, ASSAULT_RIFLE, SNIPER_BARRETT, MACHINE_GUN,
        SCOPE_8, VEST_3
    };
    if (AI_VOCATION == HACK)
        temp.push_back(CODE_CASE);
    else
        temp.insert(temp.begin(), CODE_CASE);

    vAllPriority.clear();
    for (int i = 0; i < temp.size(); ++i) {
        vAllPriority.insert({temp[i], i});
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
				vUpdateWeapon(item);
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
				if (item.type == aiWeaponCase[j].type) {
					isFind = true;
					int realDur = item.durability;
					int curDur = aiWeaponCase[j].durability;
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
    // 优先跑毒
    double r = info.poison.next_radius * 0.9;
    if (vCalcDist(info.self.xy_pos, info.poison.next_center) >= r * r)
        return false;

    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    float valueTH = -10.0;      // enemy value thres
    float threatTH = 50.0;      // enemy threat thres
	double angleDT = 35.0;		// change in direction
    bool eFlag = false;         // if exist enemy in sight
    vAiInfo enemyVA, enemyDG;   // valuable, dangerous

    if (aiEnemy.size() != 0) {
        eFlag = true;
        enemyVA = aiKV[aiEnemy[0]];
        enemyDG = aiKV[aiEnemy[aiEnemy.size() - 1]];
    }

    // 发现敌人，并且敌人value大于阈值，则Attack
    if (eFlag && enemyVA.value > valueTH) {
        if (info.self.attack_cd != 0) {
            // Debug Info
//			f << "Attack cd!\n";
            ;
        } else {
            aiBehavior = { Attack, enemyVA.rel_polar_pos.angle, 0.0, 0, 0 };
            return true;
        }
    }

    // 发现敌人，并且敌人value小于阈值或threat大于阈值，则Retreat
    if (eFlag && enemyVA.value <= valueTH) {
        aiBehavior = { Retreat, 0.0, enemyVA.rel_polar_pos.angle + angleDT, 0, 0 };
        return true;
    }
    if (eFlag && enemyDG.threat > threatTH) {
        aiBehavior = { Retreat, 0.0, enemyDG.rel_polar_pos.angle + angleDT, 0, 0 };
        return true;
    }

    return (aiBehavior.act != Undecided);
}

// 当掉血时（现在这里时当血量过低时）
bool vLoseHp() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    double healthTH = 30.0 - (aiArmor - VEST_1) * 10.0; // low health threshold
    double healthTG = info.self.hp_limit * 0.85;        // full health threshold
	double angleDT = 35.0;		// change in direction

    if (info.self.hp < healthTH) {
        // 如果在作战状态，则Retreat
        // 如果在未知状态，则MedSelf
        if (aiPrevAct[0].act == Attack) {
            aiBehavior = { Retreat, 0.0, angleDT, 0, 0 };
            return true;
        } else if (aiPrevAct[0].act == Retreat) {
            aiBehavior = { Retreat, 0.0, 0.0, 0, 0 };
            return true;
        } else {
            if (aiMedCase.size() != 0) {
                vFilterMed(FIRST_AID_CASE);
                aiBehavior = { MedSelf, 0, 0, 0, 0 };
                return true;
            } else {
                ;
                // call for help maybe
            }
        }
    }

    // 如果之前状态为MedSelf，则继续MedSelf直到奶够为止
    if (aiPrevAct[0].act == MedSelf) {
        if (info.self.hp < healthTG) {
            // 有药就吃，BANDAGE足够奶满优先用BANDAGE（默认）
            if (aiMedCase.size() == 1) {
                aiBehavior = { MedSelf, 0, 0, 0, 0 };
                return true;
            } else if (aiMedCase.size() == 2) {
                double sup = -ITEM_DATA[aiMedCase[1].type].param * aiMedCase[1].durability;
                double req = healthTG - info.self.hp;
                if (req > sup)
                    vFilterMed(FIRST_AID_CASE);
                aiBehavior = { MedSelf, 0, 0, 0, 0 };
                return true;
            } else {
                ;
                // call for help maybe
            }
        }
    }

    return (aiBehavior.act != Undecided);
}

bool vDeadTeammate() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };
    // do something maybe
    return (aiBehavior.act != Undecided);
}

// 需要跑毒时（不是特别紧急的那种，但是优先级比较高）
bool vRunPoison() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    bool isPoison = fabs(info.poison.next_center.x - 0.0) > 0.1 
        && fabs(info.poison.next_center.y - 0.0) > 0.1;

    if (isPoison) {
		double alpha = (frame > 300 && frame < 1000) ? 0.3 : 0.6;
		double r = info.poison.next_radius * alpha;
		double mAngle = vCalcAngle(info.poison.next_center, info.self.xy_pos) - info.self.view_angle;
		double vAngle = mAngle;
		if (vCalcDist(info.self.xy_pos, info.poison.next_center) >= r * r) {
			aiBehavior = { Trek, mAngle, vAngle, 0, 0 };
            return true;
		}
	}

    return (aiBehavior.act != Undecided);
}

bool vPickItem() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    if (info.items.size() != 0) {
        // 先找到优先级最高的
        Item target = { 0, FIST, {0, 0}, 0 };
        int max = 0;
        int priTH = vAllPriority[FIRST_AID_CASE];
        bool pFlag = false;
        for (int i = 0; i < info.items.size(); ++i) {
            Item thisI = info.items[i];
            if (vAllPriority[thisI.type] > max) {
                max = vAllPriority[thisI.type];
                target = thisI;
            }
        }

        // 判断优先级最高的要不要捡
        if (
            max >= priTH ||
            isWeapon(target.type) && target.type != FIST && aiWeaponCase.size() <= 2 ||
            isArmor(target.type) && target.type >= aiArmor ||
            isMed(target.type) && (aiMedCase.size() == 0 || aiMedCase[0].durability <= 8)
        ) pFlag = true;

        // 如果要捡，不在范围内则Trek，在范围内则Pick
        if (pFlag) {
            // Debug Info
//			f << "Target item: " << target.polar_pos.distance << " " << target.polar_pos.angle << std::endl;
            if (target.polar_pos.distance <= PICKUP_DISTANCE) {
                aiBehavior = { Pick, 0, 0, target.item_ID, 0 };
            } else {
                double angle = target.polar_pos.angle;
                aiBehavior = { Trek, angle, angle, 0, 0 };
            }
            return true;
        } else {
            return false;
        }
    }

    return (aiBehavior.act != Undecided);
}

// 在周围随便走走
bool vWalkAround() {
    aiBehavior = { Undecided, 0.0, 0.0, 0, 0 };

    double alpha = 0.2;
    double angleDT = 100.0;
    double r = info.poison.next_radius * alpha;
    double mAngle = vCalcAngle(info.poison.next_center, info.self.xy_pos) - info.self.view_angle;
    double vAngle = mAngle;
    if (vCalcDist(info.self.xy_pos, info.poison.next_center) >= r * r) {
        aiBehavior = { Trek, mAngle, vAngle, 0, 0 };
        return true;
    } else {
        // 如果离毒中心太近就向外走一点
        aiBehavior = { Trek, mAngle - angleDT, vAngle - angleDT, 0, 0 };
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
	/*
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
	*/

    // process item info
}

// update real-time location, insert into kv table, update enemy queue
void vDetectPlayer(int index) {
    OtherInfo tInfo = info.others[index];
    XYPosition xy_pos;
    float threat = 0, value = 0, priority = 0;

    xy_pos = vPolarToXY(tInfo.polar_pos);

    vAiInfo vInfo = {
        frame, value, threat, priority,
		tInfo.vocation, tInfo.status,
        tInfo.move_angle, tInfo.view_angle, tInfo.move_speed,
        tInfo.polar_pos, xy_pos
    };

    vCalcEnemyPriority(vInfo);

    if (aiKV.find(tInfo.player_ID) != aiKV.end()) {
        aiKV[tInfo.player_ID] = vInfo;
    } else {
        aiKV.insert({tInfo.player_ID, vInfo});

        bool isFriend = false;
        for (int i = 0; i < teammates.size(); ++i) {
            if (tInfo.player_ID == teammates[i]) {
                isFriend = true;
                break;
            }
        }
        if (isFriend) aiFriend.push_back(tInfo.player_ID);
        else vUpdateEnemy(tInfo.player_ID);
    }
}

// update enemy queue according to priority
void vUpdateEnemy(int id) {
	int pri = aiKV[id].priority;
	bool flag = false;
	for (int i = 0; i < aiEnemy.size(); ++i) {
		if (aiEnemy[i] == id) {
			aiEnemy.erase(aiEnemy.begin() + i);
		}
	}
	for (int i = 0; i < aiEnemy.size(); ++i) {
		if (pri <= aiKV[aiEnemy[i]].priority) {
			flag = true;
			aiEnemy.insert(aiEnemy.begin() + i + 1, id);
			break;
		}
	}
	if (!flag) {
		aiEnemy.push_back(id);
	}
}

// check if enemy lost in sight, update its priority
void vLostEnemy() {
	for (int i = 0; i < aiEnemy.size(); ++i) {
		int id = aiEnemy[i];
		if (aiKV[id].lastUpdateFrame != frame) {
			// Temporary
			// set priority = 0, update enemy queue
			aiKV[id].priority = 0;
			aiEnemy.erase(aiEnemy.begin() + i);
			aiEnemy.push_back(id);

			// Debug Info
//			f << "Enemy (id: " << id << ") lost in sight.\n";
		}
	}
}

void vCalcEnemyPriority(vAiInfo & ai) {
	if (ai.vocation == SNIPER) {
		ai.threat = 0.0;
		ai.value = 0.0;
	} else if (ai.vocation == MEDIC) {
		ai.threat = 10.0;
		ai.value = 100.0;
	} else {
		ai.threat = 25.0;
		ai.value = 50.0;
	}
	ai.priority = ai.value - ai.threat;
}

