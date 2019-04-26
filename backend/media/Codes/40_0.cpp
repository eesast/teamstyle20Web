#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

using namespace ts20;

extern XYPosition start_pos, over_pos;
extern std::vector<int> teammates;
extern int frame;
extern PlayerInfo info;



double move_angle(PlayerInfo info);

bool Notincircle(PlayerInfo info,XYPosition centerxy,XYPosition myxy);


void play_game(){
	update_info();
	if (frame == 0){
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = { 350, 650 };//跳在固定点
		parachute(HACK, landing_point);
		return;
	}else{
		srand(time(nullptr) + info.player_ID*frame);
	}
	double move_ang = move_angle(info)-info.self.view_angle;
	if(move_ang<0)
		move_ang += 360;
	if(info.self.status != MOVING){
		move(move_ang, 0);
	}
	return;
}


double move_angle(PlayerInfo info) {
	XYPosition centerxy;
	if(info.poison.move_flag == 1){
		centerxy = info.poison.next_center;
	}else{
		centerxy = info.poison.current_center;
	}
	XYPosition myxy = info.self.xy_pos;
	double center_angle = 0;
	if(1 || (frame>1000 || Notincircle(info,centerxy,myxy))){
		if((centerxy.x-myxy.x) == 0){
			return 0;
		}else{
			center_angle = atan2((centerxy.y-myxy.y),(centerxy.x-myxy.x))/(2*3.141)*360;
			if(center_angle<0){
				center_angle+=360;
			}
			return center_angle;
		}
	}else{//不动
		return -1;
	}
}

bool Notincircle(PlayerInfo info,XYPosition centerxy,XYPosition myxy){//判断是否在（下一次的）圈内
	double r = info.poison.next_radius;
	if((myxy.x-centerxy.x)*(myxy.x-centerxy.x)+(myxy.y-centerxy.y)*(myxy.y-centerxy.y)>r*r)
		return true;
	else
		return false;
}
