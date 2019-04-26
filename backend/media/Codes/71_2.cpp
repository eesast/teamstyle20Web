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

int k=0;

void play_game()
{
	/* Your code in this function */
	/* sample AI */

	void moveto(XYPosition aim);
	void medic();
	//////////////////////////////////////////////////�˽�ɫΪҽ�Ʊ�
	update_info();
	std::cout << "��ҵ�֡: " << frame << "\nHP: " << info.self.hp << std::endl;
	std::cout << "��λ����: " <<"X: "<< info.self.xy_pos.x << "  Y: " << info.self.xy_pos.y << std::endl;
	if (info.self.bag.size() > 1)
		std::cout << "ʰȡ�ɹ�" << std::endl;
	if (frame == 0)
	{
		srand(time(nullptr) + teammates[0]);
		XYPosition landing_point = {50/* 150 + rand() % 100*/, 50/*150 + rand() % 100*/ }; //����ѡ���õط���
		parachute(MEDIC, landing_point);  //��ɫ��ҽ�Ʊ�
		return;
	}
	else
	{
		srand(time(nullptr) + info.player_ID*frame);
	}
	if (info.self.status == ON_PLANE || info.self.status == JUMPING)
	{
		std::cout << "�ڷɻ���/��ɡ�С���" << std::endl;
		return;
	}
	if (info.others.empty())
	{
		std::cout << "δ����������" << std::endl;
		if (k < 4)
		{
			radio(teammates[k], info.player_ID);
			k++;
		}
		if (info.items.empty())
		{
			//see nothing
			if (info.self.status != MOVING)
			{
				double move_angle = 0;
				double view_angle = move_angle;
				move(move_angle, view_angle);
				std::cout << "�ƶ�" << move_angle << std::endl;
			}
		}
		else
		{
			Item closest_item;
			closest_item.polar_pos.distance = 100000;
			for (int i = 0; i < info.items.size(); ++i)
			{
				if (info.items[i].polar_pos.distance < closest_item.polar_pos.distance)
				{
					closest_item = info.items[i];
				}
			}
			std::cout << "status" << info.self.status << std::endl;
			std::cout << "**closest item angle" << closest_item.polar_pos.angle << "distance" << closest_item.polar_pos.distance << "**" << std::endl;
			if (closest_item.polar_pos.distance < 1)
			{
				pickup(closest_item.item_ID);
				std::cout << "����ʰȡ" << closest_item.item_ID << std::endl;
			}
			else if (info.self.status != MOVING)
			{
				move(closest_item.polar_pos.angle, closest_item.polar_pos.angle);
				std::cout << "�ƶ�" << closest_item.polar_pos.angle << std::endl;
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

					//////////// ////////////// ////////////// ////////////// ////////////// ////////////// ////////////// //////////////�˴���ʼ��ҩ�����㷨?
					medic();

					break;
				}
			}
			if (!is_friend && info.others[i].polar_pos.distance < closest_enemy.polar_pos.distance)
			{
				closest_enemy = info.others[i];
				has_enemy = true;
			}
		}
		if (has_enemy)  //���ֵ��ˣ����й�����
		{
			ITEM weapon = FIST;
			for (int i = 0; i < info.self.bag.size(); ++i)
			{
				if (ITEM_DATA[info.self.bag[i].type].type == WEAPON && info.self.bag[i].durability > 0)
				{
					weapon = info.self.bag[i].type;
					break;
				}
			}
			//����ѡ�������㷨
			if (closest_enemy.polar_pos.distance > ITEM_DATA[weapon].range)
			{
				move(closest_enemy.polar_pos.angle, closest_enemy.polar_pos.angle);
			}
			else
			{
				shoot(weapon, closest_enemy.polar_pos.angle); //�������ƫ�ƣ�
			}
		}
	}
	return;
}

void moveto(XYPosition aim)
{
	double dx = aim.x - info.self.xy_pos.x;
	double dy = aim.y - info.self.xy_pos.y;
	double dis = sqrt(dx * dx + dy * dy);

	double aim_angle = atan2(dy, dx) * 180.0 / 3.14159265358979;
	if (aim_angle < 0)
		  aim_angle = 360 + aim_angle;

	double set_angle = aim_angle - info.self.view_angle;
	if (set_angle < 0)
		  set_angle = 360 + set_angle;

	//std::cout << aim_angle << " " << dis << " " << dx << "." << dy << " " << aim.x <<"."<< aim.y << std::endl;
	if (dis < 1)
	{
		move(set_angle, set_angle, NOMOVE);
	}
	else
	{
		move(set_angle, set_angle);
	}
}


void medic()
{
	struct received_info
	{
		int hp_sort;
		XYPosition loc_sort;
		VOCATION voc_sort;
	};
	//�������ߵ���Ϣ����hpֵ������������������
	int i = 0, j=0;
	int32_t temp[3];
	int sender_ID[3];

	for (; j < info.sounds.size(); ++j)
	{
		if (info.sounds[j].type == RADIO_VOICE) //������ɸѡ�����ߵ��źš����ߵ��ź�ֻ�������Զ��ѡ�
		{
			temp[i] = info.sounds[j].parameter;
			sender_ID[i] = info.sounds[j].sender;
			i++;
		}
	}

	received_info medic_teammates[3];

	for (i = 0; i < 3; i++)
	{
		 int32_t mid=temp[i];
		 medic_teammates[i].loc_sort.y = mid % 1000;
		 mid = (mid - medic_teammates[i].loc_sort.y)/1000;
		 medic_teammates[i].loc_sort.x = mid % 1000;
		 mid = (mid - medic_teammates[i].loc_sort.x)/1000;
		 medic_teammates[i].voc_sort = (VOCATION)(mid % 10);
		 mid = (mid - (mid % 10) )/ 10;
		 medic_teammates[i].hp_sort = (int)mid;
	}
	//��Ҫ�����hp���ص��Լ�ְҵ�����ǽ�hp�ֶΣ�ÿ�ζ�Ӧ��ͬ�����֣�
	//��������translate��������ɴ�����

	//������������ɫ����hp�ӵ͵�������
	int mid_hp_var;
	XYPosition mid_loc_var;
	VOCATION mid_voc_var;
	received_info mid;
	int sender_mid;

	for (j=0; j<2; j++)                                 //��hp�ӵ͵������򣬰��սṹ������medic_teammates[3]����
		for (i = 0; i < 2 - j; i++)
		{
			if (medic_teammates[i].hp_sort > medic_teammates[i+1].hp_sort)
			{
				mid = medic_teammates[i];
				sender_mid=sender_ID[i];
				medic_teammates[i] = medic_teammates[i + 1];
				sender_ID[i]=sender_ID[i+1];
				medic_teammates[i + 1] = mid;
				sender_ID[i + 1] = sender_mid;
			}
		}

   
	XYPosition min_alive_loc;
	VOCATION min_alive_hp_voc;
	int min_alive_hp_ID;
	double med_aid_angle=0;  //ʹ��ҽ�ư汾shoot����ʱ�õ��ĽǶȡ�ҽ�Ʊ������ξ���Ϊ1.0������ʹ��90��ı�������������˵�Ѿ���������ID������Ҫ����ָ���Ƕȣ�
	ITEM med_aid;
	if (info.self.hp <= 15 && info.self.hp >=0)
	{
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 16 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, info.player_ID);
	}

	if (info.self.hp>= 15 && info.self.hp <= 70)
	{
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 15 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, info.player_ID);
	}

	if (medic_teammates[0].hp_sort <= 15 && medic_teammates[0].hp_sort >= 0 /*&&���벻��̫Զ��&&����Ϊ����&&��Ȧ��*/ ) //Ѫ������15�����ڵ���0������Ϊ�������Ҿ���С��ĳ��ֵ���þ����䡣
	{
		for (i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 16 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, sender_ID[0]);
	}

	if (medic_teammates[0].hp_sort >= 15 && medic_teammates[0].hp_sort <= 70 /*&&���벻��̫Զ��&&��Ȧ��*/) //Ѫ������15������70���Ҿ���С��ĳ��ֵ���ñ�����
	{
		for (int i = 0; i < info.self.bag.size(); ++i)
		{
			if (ITEM_DATA[info.self.bag[i].type].number == 15 && info.self.bag[i].durability > 0)
			{
				med_aid = info.self.bag[i].type;
				break;
			}
		}
		shoot(med_aid, 0, sender_ID[0]);
	}
}
