#define outputinfo 0
#include "api.h"
#include "base.h"
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <queue>
#include <set>
#define profession MEDIC
#define it1 multiset < Node*, QueueCompare >::iterator

using namespace ts20;
using namespace std;
extern XYPosition start_pos, over_pos;//���ߵ�������յ��XY����
extern std::vector<int> teammates;//����ID
extern int frame;//��ǰ֡������0��ʼ������
extern PlayerInfo info;//������Ϣ�ľۺ�
const double pi = 3.14159265358979;
const double boundMax = 50.0;
const double boundMin = 20.0;
/*const int unitValue[ITEM_SZ] = {//ITEM_SZ = 21,
	1,//FIST = 0,
	5,//HAND_GUN = 1,
	16,//SUBMACHINE_GUN = 2,
	14,//SEMI_AUTOMATIC_RILE = 3,
	40,//ASSAULT_RIFLE = 4,
	40,//MACHINE_GUN = 5,
	300,//SNIPER_RILFE = 6,
	960,//SNIPER_BARRETT = 7,
	3,//TIGER_BILLOW_HAMMER = 8,
	12,//CROSSBOW = 9,

	20000,//VEST_1 = 10,
	40000,//VEST_2 = 11,
	60000,//VEST_3 = 12,
	30000,//INSULATED_CLOTHING = 13,

	10000,//MUFFLER = 14,
	20000,//BONDAGE = 15,
	45000,//FIRST_AID_CASE = 16,
	-1,//CODE_CASE = 17,ҽ����Ҫ����
	5000,//SCOPE_2 = 18,
	10000,//SCOPE_4 = 19,
	30000//SCOPE_8 = 20,
};*/
vector <XYPosition> path;
int landform[1000][1000];//landform[x][y]
enum STATUS nowStatus;//��ǰ״̬������info.self.status��
int demandPercent[ITEM_SZ];//Demand����ͨ��
XYPosition destination, shrink;//Ŀ�ĵأ���Ȧ���ģ�
int follow, delay, bfsdelay;
int inside;//0��curOut��nextOut��1��curIn��nextOut��2��curIn��nextIn
int seeEnemy, collectGarbage;//����������ֵ
bool wantMove, wantShoot;

int LimitBound(int x)
{
	if (x < 0)
		return 0;
	else if (x > 999)
		return 999;
	else
		return x;
}
double LimitBound(double x)
{
	if (x < 0.0)
		return 0.0;
	else if (x > 999.0)
		return 999.0;
	else
		return x;
}
template<class T>
T Max(T a, T b)
{
	return a > b ? a : b;
}
template<class T>
T Min(T a, T b)
{
	return a < b ? a : b;
}
double AngleLimit(double a)//0-360
{
	while (a < 0.0)
		a += 360.0;
	while (a > 360.0)
		a -= 360.0;
	return a;
}
int DoubleToInt(double a)
{
	if (a > 0)
		return (int)((a * 2 + 1) / 2);
	else
		return (int)((a * 2 - 1) / 2);
}
bool DoubleEqual(double a, double b)
{
	if (fabs(a - b) < 1e-3)
		return true;
	else
		return false;
}
PolarPosition XYToPolar(XYPosition finish)//ע�����ӽ��йأ����Ż�
{
	PolarPosition a;
	a.distance = sqrt((finish.x - info.self.xy_pos.x)*(finish.x - info.self.xy_pos.x) + (finish.y - info.self.xy_pos.y)*(finish.y - info.self.xy_pos.y));
	/*if (fabs(finish.x - info.self.xy_pos.x) < 0.01)
	{
		if (fabs(finish.y - info.self.xy_pos.y) > 0.08)
		{
			if (finish.y - info.self.xy_pos.y > 0.0)
				a.angle = 90.0;
			else
				a.angle = 270.0;
			return a;
		}
	}
	if (fabs(finish.y - info.self.xy_pos.y) < 0.01)
	{
		if (fabs(finish.x - info.self.xy_pos.x) > 0.08)
		{
			if (finish.x - info.self.xy_pos.x > 0.0)
				a.angle = 0.0;
			else
				a.angle = 180.0;
			return a;
		}
	}*/
	a.angle = AngleLimit(180.0 * atan2(finish.y - info.self.xy_pos.y, finish.x - info.self.xy_pos.x) / pi - info.self.view_angle);
	return a;
}
XYPosition PolarToXY(PolarPosition p)//ע�����ӽ��й�
{
	XYPosition a;
	a.x = LimitBound(info.self.xy_pos.x + p.distance * cos(((p.angle + info.self.view_angle) / 180.0) * pi));
	a.y = LimitBound(info.self.xy_pos.y + p.distance * sin(((p.angle + info.self.view_angle) / 180.0) * pi));
	return a;
}
enum Order
{
	meInfo = 0,
	itemInfo = 1,
	enemyInfo = 2,
	demandInfo = 3,
	wantInfo = 4,
	healInfo = 5,
};
struct RadioOrder
{
	Order order;
	int type;
	XYPosition pos;//����ΪҪ����
	int priority;
};
struct Node
{
	int x, y;
	int g; //��ʼ�㵽��ǰ��ʵ�ʴ���
	int h;//��ǰ�ڵ㵽Ŀ��ڵ����·���Ĺ��ƴ���
	int f;//����ֵ
	Node* father;
	Node()
	{
		this->x = 0;
		this->y = 0;
		this->g = 0;
		this->h = 0;
		this->f = 0;
		this->father = NULL;
	}
	Node(XYPosition p)
	{
		this->x = LimitBound(DoubleToInt(p.x));
		this->y = LimitBound(DoubleToInt(p.y));
		this->g = 0;
		this->h = 0;
		this->f = 0;
		this->father = NULL;
	}
	Node(int x, int y)
	{
		this->x = x;
		this->y = y;
		this->g = 0;
		this->h = 0;
		this->f = 0;
		this->father = NULL;
	}
	Node(int x, int y, Node* father)
	{
		this->x = x;
		this->y = y;
		this->g = 0;
		this->h = 0;
		this->f = 0;
		this->father = father;
	}
};
struct QueueCompare
{
	bool operator () (Node* n1, Node* n2)
	{
		return (n1->f < n2->f) || ((n1->f == n2->f) && (n1->y < n2->y)) || ((n1->f == n2->f) && (n1->y == n2->y) && (n1->x < n2->x));
	}
};
struct XYInt
{
	int x, y;
	XYInt() { x = 0, y = 0; }
	XYInt(XYPosition a)
	{
		x = DoubleToInt(a.x);
		y = DoubleToInt(a.y);
	}
	XYInt(int xi, int yi)
	{
		x = xi, y = yi;
	}
};
double Dist(XYPosition start, XYPosition finish)
{
	return sqrt((finish.x - start.x)*(finish.x - start.x) + (finish.y - start.y)*(finish.y - start.y));
}
double Dist(Node start, Node finish)
{
	return sqrt((finish.x - start.x)*(finish.x - start.x) + (finish.y - start.y)*(finish.y - start.y));
}
class BFS
{
public:
	BFS()
	{
		for (int i = 0; i < 1000; i++)
			for (int j = 0; j < 1000; j++)
				contain[i][j] = 0;
	}
	XYPosition SearchAccess(XYPosition s)//Ѱ���ܵ���ĵ�
	{
		org.x = DoubleToInt(s.x), org.y = DoubleToInt(s.y);
		dorg = s;
		XYInt tmp;
		XYPosition a = s;
		if (s.x > 999 || s.x < 0 || s.y > 999 || s.y < 0 || org.x < 0 || org.x > 999 || org.y < 0 || org.y > 999)//�����յ����	
		{
			a.x = LimitBound(s.x);
			a.y = LimitBound(s.y);
			return a;
		}
		if (landform[org.x][org.y] <= 2)//�յ�������
			return a;
		for (int i = 0; i < 1000; i++)
			for (int j = 0; j < 1000; j++)
				contain[i][j] = 0;
		while (visit.size() > 0)
			visit.pop();
		visit.push(org);
		contain[org.x][org.y] = 1;
		while (visit.size() > 0)
		{
			tmp = visit.front();
			if (landform[tmp.x][tmp.y] <= 2)
			{
				a.x = (double)tmp.x, a.y = (double)tmp.y;
				return a;
			}
			visit.pop();
			AddPoint(tmp.x - 1, tmp.y);
			AddPoint(tmp.x + 1, tmp.y);
			AddPoint(tmp.x, tmp.y - 1);
			AddPoint(tmp.x, tmp.y + 1);
		}
		return a;
	}
private:
	void AddPoint(int x, int y)
	{
		XYPosition tmp;
		tmp.x = x, tmp.y = y;
		if (contain[x][y] == 1 || x < 0 || x >= 1000 || y < 0 || y >= 1000 || Dist(dorg, tmp) > 80.0)//����Ǵ󺣣�50����2=71	
			return;
		else
		{
			visit.push(XYInt(x, y));
			contain[x][y] = 1;
		}
	}
	bool contain[1000][1000];
	XYInt org;
	XYPosition dorg;
	queue <XYInt> visit;
};//�õ�ʱ���ֿ�
class Astar {
public:
	bool Search(Node* startPos, Node* endPos);
	void CheckPoint(int x, int y, it1 father, int g);
	void NextStep(it1 currentPoint);
	void CountGHF(Node* sNode, Node* eNode, int g);
	static bool Compare(Node* n1, Node* n2);
	bool UnWalk(int x, int y);
	void RecordPath(Node* current);
	multiset < Node*, QueueCompare > openList;
	Node *startPos;
	Node *endPos;
	bool openContain[1000][1000];
	bool closeContain[1000][1000];
	it1 it[1000][1000];
}astar;
bool Astar::Search(Node* startPos, Node* endPos)//�п��ܿ�ʼ�ĵط��ǲ����ߵģ���ǽ���ˣ����Ż�
{
	if (startPos->x < 0 || startPos->x >= 1000 || startPos->y < 0 || startPos->y >= 1000 ||
		endPos->x < 0 || endPos->x >= 1000 || endPos->y < 0 || endPos->y >= 1000)
		return 0;
	if (landform[endPos->x][endPos->y] > 2 || landform[startPos->x][startPos->y] > 2)
		return 0;
	for (int i = 0; i < 1000; i++)
		for (int j = 0; j < 1000; j++)
			openContain[i][j] = closeContain[i][j] = 0;
	openList.clear();
	path.clear();
	it1 current;
	this->startPos = startPos;
	this->endPos = endPos;
	it[startPos->x][startPos->y] = openList.insert(startPos);
	openContain[startPos->x][startPos->y] = 1;
	//��Ҫ����飬�ѿ�ʼ�Ľڵ����openlist��ʼ�����Աߵ�8���ڵ㣬������곬����Χ����closelist��return ����Ѿ�����openlist�ͶԱȵ�ǰ�ڵ㵽���������Ǹ��ڵ��Gֵ�͵�ǰ�ڵ㵽ԭ�����ڵ��Gֵ ���ԭ����Gֵ�Ƚϴ� ���ù� �������¸�ֵGֵ ���ڵ� ��f ������½ڵ� ���뵽openlist ֱ��opellistΪ�ջ��ҵ��յ�
	while (openList.size() > 0)
	{
		current = openList.begin();
		if ((*current)->x == endPos->x && (*current)->y == endPos->y)
		{
			RecordPath(*current);
			return 1;
		}
		NextStep(current);
		closeContain[(*current)->x][(*current)->y] = 1;
		openContain[(*current)->x][(*current)->y] = 0;
		openList.erase(current);
	}
	return 0;
}
void Astar::CheckPoint(int x, int y, it1 father, int g)
{
	if (x < 0 || x >= 1000 || y < 0 || y >= 1000)
		return;
	if (this->UnWalk(x, y))
		return;
	if (closeContain[x][y] == 1)
		return;
	if (openContain[x][y] == 1)
	{
		Node* point = *it[x][y];
		if (point->g > (*father)->g + g)
		{
			openList.erase(it[x][y]);
			point->father = *father;
			point->g = (*father)->g + g;
			point->f = point->g + point->h;
			it[x][y] = openList.insert(point);
			int a = 1;
		}
	}
	else
	{
		Node *point = new Node(x, y, *father);
		CountGHF(point, endPos, g);
		it[point->x][point->y] = openList.insert(point);
		openContain[point->x][point->y] = 1;
	}
}
void Astar::NextStep(const it1 current)
{
	int w = 10, wh = 14;//����б������
	switch (landform[(*current)->x][(*current)->y])//�������ͼ�Rect��Circ����
	{
	case 6://DEEP_WATER
		w = 20; wh = 28;
		break;
	case 4://RECTANGLE_BUILDING, CIRCLE_BUILDING
		w = 15; wh = 21;
		break;
	case 2://SHALLOW_WATER
		w = 13; wh = 18;
		break;
	case 5://WALL
		w = 15; wh = 21;
		break;
	case 3://TREE
		w = 15; wh = 21;
		break;
	case 1://RECTANGLE_GRASS, CIRCLE_GRASS
		w = 8; wh = 11;
		break;
	default:
		w = 10; wh = 14;
		break;
	}
	CheckPoint((*current)->x - 1, (*current)->y, (current), w);//��
	CheckPoint((*current)->x + 1, (*current)->y, (current), w);//��
	CheckPoint((*current)->x, (*current)->y + 1, (current), w);//��
	CheckPoint((*current)->x, (*current)->y - 1, (current), w);//��
	//CheckPoint((*current)->x - 1, (*current)->y + 1, (current), wh);//����
	//CheckPoint((*current)->x - 1, (*current)->y - 1, (current), wh);//����
	//CheckPoint((*current)->x + 1, (*current)->y - 1, (current), wh);//����
	//CheckPoint((*current)->x + 1, (*current)->y + 1, (current), wh);//����
}
void Astar::CountGHF(Node* sNode, Node* eNode, int g)
{
	int h = abs(sNode->x - eNode->x) + abs(sNode->y - eNode->y) * 10;//ֱ������
	int currentg = sNode->father->g + g;
	int f = currentg + h;
	sNode->f = f;
	sNode->h = h;
	sNode->g = currentg;
}
bool Astar::Compare(Node* n1, Node* n2)
{
	return n1->f < n2->f;
}
bool Astar::UnWalk(int x, int y)
{
	if (landform[x][y] > 2)
		return true;
	return false;
}
void Astar::RecordPath(Node* current)
{
	if ((current)->father != NULL)
	{
		XYPosition p;
		RecordPath((current)->father);
		p.x = (current)->x, p.y = (current)->y;
		path.push_back(p);
	}
}
bool Reachable(XYPosition p)
{
	if (p.x < 0 || p.x > 999.0 || p.y < 0 || p.y > 999.0)
		return 0;
	return (landform[DoubleToInt(p.x)][DoubleToInt(p.y)] <= 2);
}
bool IsFriend(int id)
{
	for (int i = 0; i < teammates.size(); i++)
		if (id == teammates[i])
			return true;
	return false;
}
bool HaveItem(ITEM_TYPE a)
{
	for (int i = 0; i < info.self.bag.size(); i++)
	{
		if (info.self.bag[i].type == a && info.self.bag[i].durability > 0)
			return 1;
	}
	return 0;
}
int HaveWeapon(double searchDist = 3.0)//searchdist=3����fist�ͻ��δ��������ܴ��е��˺���ߵ�ǹ֧��ţ�����ǹ֧�ͻ��δ�����û��ǹ����-1
{
	int num = -1, damageMax = 0, demandMin = 100;
	for (int i = 0; i <= 9; i++)
	{
		if ((searchDist <= ITEM_DATA[i].range) && HaveItem((ITEM_TYPE)i) && damageMax <= ITEM_DATA[i].damage / ITEM_DATA[i].cd)
		{
			num = i;
			damageMax = ITEM_DATA[i].damage;
		}
	}
	return Max(-1, num);
}
void Rect(BLOCK_TYPE t, int x1, int y1, int x2, int y2)
{
	int xmin = LimitBound(Min(x1, x2));
	int xmax = LimitBound(Max(x1, x2));
	int ymin = LimitBound(Min(y1, y2));
	int ymax = LimitBound(Max(y1, y2));
	for (int j = ymin; j <= ymax; j++)
		for (int i = xmin; i <= xmax; i++)
			switch (t)
			{
			case SHALLOW_WATER://����ͨ�е������
				landform[i][j] = 2;
				break;
			case RECTANGLE_GRASS:case CIRCLE_GRASS://����ͨ�п�������
				landform[i][j] = 1;
				break;
			case DEEP_WATER://�޷�ͨ���޷����ӵ�������
				landform[i][j] = 6;
				break;
			case WALL://�޷�ͨ���޷����ӵ�����С
				landform[i][j] = 5;
				break;
			case RECTANGLE_BUILDING:case CIRCLE_BUILDING://�޷�ͨ���ܵ��ӵ�����һ�
				landform[i][j] = 4;
				break;
			case TREE://�޷�ͨ���ܵ��ӵ���С�һ�
				landform[i][j] = 3;
				break;
			default://����ͨ��
				break;
			}
}
void Circ(BLOCK_TYPE t, int x, int y, int r)
{
	XYPosition a, b;
	int xmin = LimitBound(x - r);
	int xmax = LimitBound(x + r);
	int ymin = LimitBound(y - r);
	int ymax = LimitBound(y + r);
	for (int j = ymin; j <= ymax; j++)
		for (int i = xmin; i <= xmax; i++)
		{
			a.x = i, a.y = j;
			b.x = x, b.y = y;
			if (Dist(a, b) <= (double)r + 1e-3)
			{
				switch (t)
				{
				case SHALLOW_WATER://����ͨ�е������
					landform[i][j] = 2;
					break;
				case RECTANGLE_GRASS:case CIRCLE_GRASS://����ͨ�п�������
					landform[i][j] = 1;
					break;
				case DEEP_WATER://�޷�ͨ���޷����ӵ�������
					landform[i][j] = 6;
					break;
				case WALL://�޷�ͨ���޷����ӵ�����С
					landform[i][j] = 5;
					break;
				case RECTANGLE_BUILDING:case CIRCLE_BUILDING://�޷�ͨ���ܵ��ӵ�����һ�
					landform[i][j] = 4;
					break;
				case TREE://�޷�ͨ���ܵ��ӵ���С�һ�
					landform[i][j] = 3;
					break;
				default://����ͨ��
					break;
				}
			}
		}
}
void MapProcess()
{
	for (int i = 0; i < 1000; i++)
		for (int j = 0; j < 1000; j++)
			landform[i][j] = 0;
	for (int y = 9; y >= 0; y--)
	{
		for (int x = 0; x < 10; x++)
		{
			AREA num = MAP[y * 10 + x];
			int offsetx = x * 100, offsety = y * 100;
			for (int i = 0; i < AREA_DATA[num].size(); i++)
			{
				if (AREA_DATA[num][i].shape == RECTANGLE)
					Rect(AREA_DATA[num][i].type, offsetx + AREA_DATA[num][i].x0, offsety + AREA_DATA[num][i].y0, offsetx + AREA_DATA[num][i].x1, offsety + AREA_DATA[num][i].y1);
				else if (AREA_DATA[num][i].shape == CIRCLE)
					Circ(AREA_DATA[num][i].type, offsetx + AREA_DATA[num][i].x0, offsety + AREA_DATA[num][i].y0, AREA_DATA[num][i].r);
			}
		}
	}
}
void Initial()//������
{
	srand(time(NULL));
	follow = 2;
	nowStatus = info.self.status;
	wantMove = wantShoot = 0;
	shrink.x = shrink.y = 0;
	if (frame == 0)
	{
		parachute(profession, XYPosition{ (start_pos.x + over_pos.x) / 2 + rand() % 100, (start_pos.y + over_pos.y) / 2 + rand() % 100 });
		MapProcess();
	}
}
bool Shoot(ITEM item_type, double shoot_angle, int parameter = -1)//������ʹ�õĵ���/ǹ��ö�٣���ԽǶȣ����������ҽ�Ʊ�ʹ��ҩƷ�Ķ���ID��
{
	wantShoot = 1;
	switch (nowStatus)
	{
	case RELAX:
		shoot(item_type, shoot_angle, parameter);
		nowStatus = SHOOTING;
		return true;
	case MOVING:
		shoot(item_type, shoot_angle, parameter);
		nowStatus = MOVING_SHOOTING;
		return true;
	default:
		return false;
	}
}
bool Move(double move_angle, double view_angle, int parameter = -1)//������ǰ���������ӽǵ���ԽǶȣ�����ڵ�ǰ�ӽǣ���parameter == NOMOVE(0)ʱ���ƶ���ֻ�����Ƕ�
{
	wantMove = 1;
	if (parameter != NOMOVE)
	{
		switch (nowStatus)
		{
		case RELAX:case MOVING:
			move(move_angle, view_angle);
			nowStatus = MOVING;
			return true;
		case SHOOTING:
			move(move_angle, view_angle);
			nowStatus = MOVING_SHOOTING;
			return true;
		default:
			return false;
		}
	}
	else
	{
		move(move_angle, view_angle, NOMOVE);
		return true;
	}
}
Node Shrink(XYPosition des)//���Ż�
{
	XYPosition close;
	PolarPosition polar;
	const double r = 25.0;
	double delta;
	close.x = des.x, close.y = des.y;
	polar = XYToPolar(des);
	delta = Max(fabs(sin(polar.angle * pi / 180.0)), fabs(cos(polar.angle * pi / 180.0)));
	delta = 1 / delta;
	if (Reachable(des) && polar.distance < boundMax)
		return Node(des);
	polar.distance = boundMax;
	while (polar.distance > boundMin)
	{
		polar.distance -= delta;
		close = PolarToXY(polar);
		if (Reachable(close))
			return Node(close);
	}
	BFS finder;
	int time = clock();
	polar.distance = boundMax;
	close = PolarToXY(polar);
	close = finder.SearchAccess(close);
	bfsdelay = clock() - time;
	return Node(close);
}
bool MoveToDes(int parameter = -1)
{
	if (info.self.status == ON_PLANE || info.self.status == JUMPING || frame < 5)
		return 0;
	if (parameter == NOMOVE)
		return Move(0, VOCATION_DATA[info.self.vocation].angle - 1.0, NOMOVE);
	if (Dist(info.self.xy_pos, destination) < 3.0)
		return Move(XYToPolar(destination).angle, XYToPolar(destination).angle);
	Node a = Shrink(destination);
	shrink.x = a.x, shrink.y = a.y;
	astar.Search(&Node(info.self.xy_pos), &a);
	if (!path.empty())
	{
		if (follow == 2)
			return Move(XYToPolar(path[0]).angle, XYToPolar(destination).angle, parameter);
		else if (follow == 1)
			return Move(XYToPolar(path[0]).angle, XYToPolar(path[0]).angle, parameter);
		else
			return Move(XYToPolar(path[0]).angle, VOCATION_DATA[info.self.vocation].angle - 1.0, parameter);
		//if (ObstacleDetect(5))
		/*int sz = Min(2, (int)path.size());//ȡǰ������ƽ��
		XYPosition average;
		average.x = 0, average.y = 0;
		for (int i = 0; i < sz; i++)
		{
			average.x += path[i].x;
			average.y += path[i].y;
		}
		average.x /= (double)sz;
		average.y /= (double)sz;
		if (follow == 2)
			return Move(XYToPolar(average).angle, XYToPolar(destination).angle, parameter);
		else if (follow == 1)
			return Move(XYToPolar(average).angle, XYToPolar(average).angle, parameter);
		else
			return Move(XYToPolar(average).angle, VOCATION_DATA[info.self.vocation].angle - 1.0, parameter);*/
	}
	else
		return 0;
}
bool Pickup(int target_ID, bool strong = false, int parameter = -1)//���������ϵ���ƷID����Ҫ��һ����Χ�ڲ��ܳɹ���PICKUP_DISTANCE������ΪstrongΪtrue���Ϲ���������ֻ����ƶ�
{
	switch (nowStatus)
	{
	case RELAX:case MOVING:
		pickup(target_ID, parameter);
		nowStatus = PICKUP;
		return true;
	case SHOOTING:case MOVING_SHOOTING:
		if (strong)
		{
			pickup(target_ID, parameter);
			nowStatus = PICKUP;
			return true;
		}
		return false;
	default:
		return false;
	}
}
bool Radio(int target_ID, int msg)//ÿ�غ�ֻ�ܷ���ȥ��һ����������������ID����Ϣ��ֻ��ʹ�õ�29λ��
{
	static int flag = 0;
	if (flag + frame != 0)
	{
		flag = -frame;
		radio(target_ID, msg);
		return true;
	}
	return false;
}
void Demand()
{
	for (int i = 0; i < ITEM_SZ; i++)
		demandPercent[i] = 100;
	demandPercent[0] = -1;//fist
	demandPercent[17] = -1;//code case
	demandPercent[15] = demandPercent[16] = 120;//bandage,first aid case
	const int m = 10;
	for (int i = 0; i < info.self.bag.size(); i++)
	{
		switch (info.self.bag[i].type)
		{
		case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
			demandPercent[info.self.bag[i].type] -= m;
			break;
		case 10:case 11:case 12:case 13:
			demandPercent[info.self.bag[i].type] = 100 - 100 * info.self.bag[i].durability / ITEM_DATA[info.self.bag[i].type].durability;
			if (info.self.bag[i].type == 11)//���и��߼���������Ҫ�ͼ�������������һ�����Ϸ�������Ҫ�����
				demandPercent[10] = demandPercent[13] = 0;
			else if (info.self.bag[i].type == 12)
				demandPercent[10] = demandPercent[11] = demandPercent[13] = 0;
			break;
		case 14:
			demandPercent[info.self.bag[i].type] = -1;
			break;
		case 15:case 16:
			demandPercent[info.self.bag[i].type] = DoubleToInt((demandPercent[info.self.bag[i].type] * 0.8));//�߼ʵݼ�
			break;
		case 18:
			demandPercent[18] = -1;
			break;
		case 19:
			demandPercent[18] = demandPercent[19] = -1;
			break;
		case 20:
			demandPercent[18] = demandPercent[19] = demandPercent[20] = -1;
			break;
		default://0,17
			break;
		}
	}
}
bool Neibour(XYPosition a, XYPosition b)
{
	return (Dist(a, b) <= 1.0);
}
int SeeEnemy()//�������˳ɹ����˷���3������������cd���˻��߿���������ǹ�򲻵�����2������ûǹ���˷���1��û��������0
{
	int num, enemyNum = -1, flag = 0;//flag�����Ƿ�׷��
	double minDist = 1000000.0;
	if (info.others.empty())
		return 0;
	for (int i = 0; i < info.others.size(); i++)
	{
		if ((!IsFriend(info.others[i].player_ID)) && info.others[i].polar_pos.distance < minDist)
		{
			enemyNum = i;
			minDist = info.others[i].polar_pos.distance;
		}
	}
	if (enemyNum < 0)
		return 0;
	if (HaveWeapon() <= 0)//ûǹ����
		return 1;
	num = HaveWeapon(info.others[enemyNum].polar_pos.distance);
	if (num >= 0)
	{
		if (Shoot((ITEM)num, info.others[enemyNum].polar_pos.angle))
			return 3;
		else
			return 2;
	}
	else
	{
		destination = PolarToXY(info.others[enemyNum].polar_pos);
		return 2;
	}
	return 2;//��ǹ���򲻵��Ͳ�׷�ˣ���ϵ����
}
int CollectGarbage()//�������˷���3�����Լ�û����������2�������˹���������1��û�������ܶ�����Ҫ����0�����Ż�����id
{
	double minDist = 100000.0;
	int num = -1;
	if (inside == 0 || inside == 1)
		return 0;
	if (!Neibour(info.self.xy_pos, destination))//��û��dest
	{
		for (int i = 0; i < info.items.size(); i++)
		{
			if (Neibour(destination, PolarToXY(info.items[i].polar_pos)) && demandPercent[info.items[i].type] > 0)//��Ҫ��dest������
				return 1;//����
		}//��ѭ��˵��destination���Ƕ������߶�����������
		for (int i = 0; i < info.items.size(); i++)
		{
			if (info.items[i].polar_pos.distance < minDist && demandPercent[info.items[i].type] > 0)
			{
				num = i;
				minDist = info.items[i].polar_pos.distance;
			}
		}//��һ���������Ҫ�Ķ������
		if (num >= 0)
		{
			destination = PolarToXY(info.items[num].polar_pos);
			return 1;
		}
		else
			return 0;
	}
	else//�Ѿ���des��
	{
		for (int i = 0; i < info.items.size(); i++)
		{
			if (Neibour(info.self.xy_pos, PolarToXY(info.items[i].polar_pos)) && demandPercent[info.items[i].type] > 0)//��Ҫ�ҹ��õ�
			{
				if (Pickup(info.items[i].item_ID, false))
					return 3;
				else
					return 2;
			}
		}//��ѭ��˵�����������߹��ˣ���Ӧ�ó��֣�
		for (int i = 0; i < info.items.size(); i++)
		{
			if (info.items[i].polar_pos.distance < minDist && demandPercent[info.items[i].type] > 0)
			{
				num = i;
				minDist = info.items[i].polar_pos.distance;
			}
		}//��һ���������Ҫ�Ķ������
		if (num >= 0)
		{
			destination = PolarToXY(info.items[num].polar_pos);
			return 1;
		}
		else
			return 0;
	}
}
void YangYongXin()//�ܶ�����������㷨���Ż��������е㱩������д�����˱�����ܵ���ĵ�
{
	if (info.self.status == ON_PLANE || info.self.status == JUMPING || frame < 5)
		return;
	if (frame <= 200)
	{//����һ��return
		inside = 2;
		seeEnemy = SeeEnemy();
		if (seeEnemy <= 1)
			collectGarbage = CollectGarbage();
		if (collectGarbage <= 0 && seeEnemy <= 1)//����ȥ����/����
		{
			const double radius = 100;
			XYPosition org;
			org.x = org.y = 500;
			if (!(Dist(destination, org) < radius && Dist(destination, org) > 0.6 * radius && Dist(destination, info.self.xy_pos) > 15.0 && Reachable(destination)))//des�����㣨desλ�ڵ�ͼ���ĵİ뾶С��200�Ҵ���120��Բ�����Ҿ��Լ��ľ������15�����ߣ�
			{
				BFS finder;
				double dx = (double)(rand() % (int)radius), dy = (double)(rand() % (int)radius);
				destination.x = LimitBound(org.x + dx);
				destination.y = LimitBound(org.y + dy);
				destination = finder.SearchAccess(destination);
			}
		}
		return;
	}//����Ϊframe>200
	BFS finder;
	inside = 0;
	if (frame <= 900)
	{
		if (Dist(info.self.xy_pos, info.poison.current_center) <= info.poison.current_radius * 0.7 + info.poison.next_radius * 0.3)
			inside++;
		if (Dist(info.self.xy_pos, info.poison.next_center) <= info.poison.next_radius * 0.95)
			inside++;
	}
	else
	{
		if (Dist(info.self.xy_pos, info.poison.current_center) <= info.poison.current_radius * 0.6 + info.poison.next_radius * 0.4)
			inside++;
		if (Dist(info.self.xy_pos, info.poison.next_center) <= info.poison.next_radius * 0.8)
			inside++;
	}//����Ϊ����inside
	if (inside == 0)
	{
		int time = clock();
		destination = finder.SearchAccess(info.poison.current_center);//��ҪBFS
		bfsdelay = clock() - time;
		return;
	}
	else if (inside == 1)
	{
		int time = clock();
		destination = finder.SearchAccess(info.poison.next_center);//��ҪBFS
		bfsdelay = clock() - time;
		return;
	}
	else//����Ϊinside == 2
	{//����һ��return
		seeEnemy = SeeEnemy();
		collectGarbage = CollectGarbage();
		if (collectGarbage <= 0)//����ȥ����/����
		{
			if (info.poison.next_radius > 120)//Ȧ���ϴ�
			{
				if (!(Dist(destination, info.poison.next_center) < 0.8 * info.poison.next_radius && Dist(destination, info.poison.next_center) > 0.1 * info.poison.next_radius && Dist(destination, info.self.xy_pos) < 10.0 && Reachable(destination)))//des�����㣨desλ�ڵ�ͼ���ĵİ뾶С��200�Ҵ���120��Բ�����Ҿ��Լ��ľ������15�����ߣ�
				{
					BFS finder;
					double dx = (double)(rand() % (int)(0.8 * info.poison.next_radius)), dy = (double)(rand() % (int)(0.8 * info.poison.next_radius));
					destination.x = LimitBound(info.poison.next_center.x + dx);
					destination.y = LimitBound(info.poison.next_center.y + dy);
					destination = finder.SearchAccess(destination);
				}
				return;
			}
			else
			{
				int time = clock();
				destination = finder.SearchAccess(info.poison.next_center);//��ҪBFS
				bfsdelay = clock() - time;
				return;
			}
		}
		else
			return;
	}
}
void Heal()
{
	if (fabs(info.self.hp_limit - info.self.hp > 5.0))
	{
		for (int i = 0; i < info.self.bag.size(); i++)
		{
			if (info.self.bag[i].type == 16)
			{
				if (frame % 8 == 0)
				{
					if (info.self.vocation == MEDIC)
						Shoot(info.self.bag[i].type, 0.0, info.player_ID);
					else
						Shoot(info.self.bag[i].type, 0.0);
					return;
				}
			}
			else if (info.self.bag[i].type == 15)
			{
				if (frame % 5 == 0)
				{
					if (info.self.vocation == MEDIC)
						Shoot(info.self.bag[i].type, 0.0, info.player_ID);
					else
						Shoot(info.self.bag[i].type, 0.0);
					return;
				}
			}
		}
	}
}
void File()
{
	if (frame <= 1)
		return;
	FILE *fp;
	char round[100];
	switch (info.self.vocation)
	{
	case MEDIC:
		sprintf(round, "C:\\Users\\123\\Desktop\\medic\\%d.txt", frame);
		break;
	case HACK:
		sprintf(round, "C:\\Users\\123\\Desktop\\hack\\%d.txt", frame);
		break;
	case SIGNALMAN:
		sprintf(round, "C:\\Users\\123\\Desktop\\signalman\\%d.txt", frame);
		break;
	case SNIPER:
		sprintf(round, "C:\\Users\\123\\Desktop\\sniper\\%d.txt", frame);
		break;
	}
	fp = fopen(round, "w");
	fprintf(fp, "frame:  %5d,  %3d,  %2d,  %2d,  %3d\n", frame, delay, seeEnemy, collectGarbage, path.size());
	fprintf(fp, "Heal:%6.1f /%6.1f  Pos :%6.1f ,%6.1f\n", info.self.hp, info.self.hp_limit, info.self.xy_pos.x, info.self.xy_pos.y);
	fprintf(fp, "Cen :%6.1f ,%6.1f  Next:%6.1f ,%6.1f\n", info.poison.current_center.x, info.poison.current_center.y, info.poison.next_center.x, info.poison.next_center.y);
	fprintf(fp, "Rad :%6.1f ,%6.1f  Rest:%6d ", info.poison.current_radius, info.poison.next_radius, info.poison.rest_frames);
	if (info.poison.move_flag == 3)
		fprintf(fp, "to point \n");
	if (info.poison.move_flag == 2)
		fprintf(fp, "to move  \n");
	else if (info.poison.move_flag == 1)
		fprintf(fp, "to finish\n");
	else
		fprintf(fp, "to start \n");
	fprintf(fp, "Ang :%6.1f ,%6.1f  Stat: %d  ", info.self.move_angle, info.self.view_angle, inside);
	switch (info.self.status)
	{
	case RELAX:
		fprintf(fp, "Relax"); break;
	case ON_PLANE:
		fprintf(fp, "Fly  "); break;
	case JUMPING:
		fprintf(fp, "Jump "); break;
	case MOVING:
		fprintf(fp, "Move "); break;
	case SHOOTING:
		fprintf(fp, "Shoot"); break;
	case MOVING_SHOOTING:
		fprintf(fp, "Mv&St"); break;
	case DEAD:
		fprintf(fp, "Dying"); break;
	case REAL_DEAD:
		fprintf(fp, "Dead "); break;
	}
	fprintf(fp, "\nToCr:%6.1f ,%6.1f  Want:", Dist(info.self.xy_pos, info.poison.current_center) - info.poison.current_radius, Dist(info.self.xy_pos, info.poison.next_center) - info.poison.next_radius);
	if (wantMove)
		fprintf(fp, "Move  ");
	if (wantShoot)
		fprintf(fp, "Shoot ");
	fprintf(fp, "\nDest:%6.1f ,%6.1f  Dist:%6.1f\n", destination.x, destination.y, Dist(info.self.xy_pos, destination));
	fprintf(fp, "Shrk:%6.1f ,%6.1f  ", shrink.x, shrink.y);
	if (path.size() > 0)
		fprintf(fp, "Pace:%6.3lf ,%6.3lf", path[0].x - info.self.xy_pos.x, path[0].y - info.self.xy_pos.y);
	for (int i = 1; i < ITEM_SZ; i++)//Demand
	{
		if ((i - 1) % 5 == 0)
			fprintf(fp, "\n");
		if (demandPercent[i] > 0)
			fprintf(fp, "%3d%%   ", demandPercent[i]);
		else
			fprintf(fp, "---%%   ");
	}
	int xx = DoubleToInt(info.self.xy_pos.x), yy = DoubleToInt(info.self.xy_pos.y), r = 1;
	int xmin = LimitBound(xx - r), xmax = LimitBound(xx + r), ymin = LimitBound(yy - r), ymax = LimitBound(yy + r);
	for (int j = ymax; j >= ymin; j--)
	{
		fprintf(fp, "\n");
		for (int i = xmin; i <= xmax; i++)
			fprintf(fp, "%d  ", landform[i][j]);
	}
	fprintf(fp, "\n\n\n");
	fclose(fp);
}
void play_game()
{
	bfsdelay = 0;
	int a = clock();
	update_info();
	Initial();
	Demand();
	YangYongXin();
	MoveToDes();
	Heal();
	delay = clock() - a;
#if outputinfo
	File();
#endif
}

/*bool LookAround(XYPosition p)//ǰ�������ܿ�
{
	Gotoxy(40, 41);
	printf("warning\n");
	wantlook = 1;
	return MoveToDes(p, VOCATION_DATA[info.self.vocation].angle - 1.0);
}
bool LookAround()//��ǰ��
{
	Gotoxy(40, 41);
	printf("warning\n");
	wantlook = 1;
	return Move(0, VOCATION_DATA[info.self.vocation].angle - 1.0, NOMOVE);
}
int HearOthers()////δ���
{
	for (int i = 0; i < info.sounds.size(); i++)
	{
		if (info.sounds[i].sender == -1 && info.sounds[i].delay > 0)//���˷���������
		{
			return -1;
		}
		else if (info.sounds[i].sender != -1)//���ѵ����ߵ�
		{
			return 1;////RadioProcess();
		}
		else
			return 0;
	}
}
bool Stuck()
{
	static XYPosition p[4];
	static STATUS s[4];
	if (frame == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			p[i].x = p[i].y = 0;
			s[i] = RELAX;
		}
	}
	s[3] = s[2];
	s[2] = s[1];
	s[1] = s[0];
	s[0] = info.self.status;
	p[3] = p[2];
	p[2] = p[1];
	p[1] = p[0];
	p[0] = info.self.xy_pos;
	if (frame <= 5)
		return 0;
	if (Dist(destination, info.self.xy_pos) <= 1 || Dist(thing, info.self.xy_pos) <= 1)
		return 0;
	for (int i = 0; i < 4; i++)
	{
		if (s[i] == MOVING || s[i] == MOVING_SHOOTING)
		{
			;
		}
		else
			return 0;
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			if (i != j && Dist(p[i], p[j]) > 1.5)
				return 0;
	return 1;
}
void DeStuck()
{
	isStuck = Stuck();
	if (isStuck)
	{
		boundLimit -= 20;
		if (boundLimit <= 30)
			boundLimit = 30;
		if (!path.empty())
			Move(AngleLimit(-1 * XYToPolar(path[0]).angle), AngleLimit(-1 * XYToPolar(path[0]).angle));
		else
			Move(180, 180);
	}
	else
		boundLimit = 150;
}
bool ObstacleDetect(int r)//���Ż�Ϊǰ������
{
	int flag = 0;
	int xmin = LimitBound(DoubleToInt(info.self.xy_pos.x) - r), xmax = LimitBound(DoubleToInt(info.self.xy_pos.x) + r);
	int ymin = LimitBound(DoubleToInt(info.self.xy_pos.y) - r), ymax = LimitBound(DoubleToInt(info.self.xy_pos.y) + r);
	for (int j = ymin; j <= ymax; j++)
		for (int i = xmin; i < xmax; i++)
			if (landform[i][j] > 2)
				flag++;
	return (flag != 0);
}
void File()
{
	FILE *fp;
	char round[100];
	int landfile[1000][1000];
	sprintf(round, "a%d.txt", frame);
	fp = fopen(round, "w");
	memcpy(landfile, landform, sizeof(landfile));
	for (int i = 0; i < path.size(); i++)
		landfile[DoubleToInt(path[i].x)][DoubleToInt(path[i].y)] = 8;
	for (int j = 1000 - 1; j >= 0; j--)
	{
		for (int i = 0; i < 1000; i++)
		{
			fprintf(fp, "%d", landfile[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}*/
/*
#include <windows.h>
void Gotoxy(int x, int y)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hConsoleOut;
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
	csbiInfo.dwCursorPosition.X = 2 * x;
	csbiInfo.dwCursorPosition.Y = y;
	SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
}
void Print()
{
	if (info.self.vocation != MEDIC)
		return;
	static int offset = 48;
	int y = 0;
	if (frame % 160 == 0)
		system("cls");
	Gotoxy(offset, y++);
	switch (info.self.vocation)
	{
	case MEDIC:
		printf("Medic ");
		break;
	case HACK:
		printf("Hack  ");
		break;
	case SIGNALMAN:
		printf("Signal");
		break;
	case SNIPER:
		printf("Sniper");
		break;
	}
	printf(":  %5d,  %3d,  %2d,  %2d,  %3d", frame, delay, seeEnemy, collectGarbage, path.size());
	Gotoxy(offset, y++);
	printf("Heal:%6.1f /%6.1f  Pos :%6.1f ,%6.1f", info.self.hp, info.self.hp_limit, info.self.xy_pos.x, info.self.xy_pos.y);
	Gotoxy(offset, y++);
	printf("Cen :%6.1f ,%6.1f  Next:%6.1f ,%6.1f", info.poison.current_center.x, info.poison.current_center.y, info.poison.next_center.x, info.poison.next_center.y);
	Gotoxy(offset, y++);
	printf("Rad :%6.1f ,%6.1f  Rest:%6d ", info.poison.current_radius, info.poison.next_radius, info.poison.rest_frames);
	if (info.poison.move_flag == 3)
		printf("to point ");
	if (info.poison.move_flag == 2)
		printf("to move  ");
	else if (info.poison.move_flag == 1)
		printf("to finish");
	else
		printf("to start ");
	Gotoxy(offset, y++);
	printf("Ang :%6.1f ,%6.1f  Stat: %d  ", info.self.move_angle, info.self.view_angle, inside);
	switch (info.self.status)
	{
	case RELAX:
		printf("Relax"); break;
	case ON_PLANE:
		printf("Fly  "); break;
	case JUMPING:
		printf("Jump "); break;
	case MOVING:
		printf("Move "); break;
	case SHOOTING:
		printf("Shoot"); break;
	case MOVING_SHOOTING:
		printf("Mv&St"); break;
	case DEAD:
		printf("Dying"); break;
	case REAL_DEAD:
		printf("Dead "); break;
	}
	Gotoxy(offset, y++);
	printf("ToCr:%6.1f ,%6.1f  Want:", Dist(info.self.xy_pos, info.poison.current_center) - info.poison.current_radius, Dist(info.self.xy_pos, info.poison.next_center) - info.poison.next_radius);
	if (wantMove)
		printf("Move  ");
	else
		printf("      ");
	if (wantShoot)
		printf("Shoot ");
	else
		printf("      ");
	Gotoxy(offset, y++);
	printf("Dest:%6.1f ,%6.1f  Dist:%6.1f", destination.x, destination.y, Dist(info.self.xy_pos, destination));
	Gotoxy(offset, y++);
	printf("Shrk:%6.1f ,%6.1f  ", shrink.x, shrink.y);
	if (path.size() > 0)
		printf("Pace:%6.3lf ,%6.3lf", path[0].x - info.self.xy_pos.x, path[0].y - info.self.xy_pos.y);
	else
		printf("            ");
	for (int i = 1; i < ITEM_SZ; i++)//Demand
	{
		if ((i - 1) % 5 == 0)
			Gotoxy(offset, y++);
		if (demandPercent[i] > 0)
			printf("%3d%%   ", demandPercent[i]);
		else
			printf("---%%   ");
	}
	int xx = DoubleToInt(info.self.xy_pos.x), yy = DoubleToInt(info.self.xy_pos.y), r = 1;
	int xmin = LimitBound(xx - r), xmax = LimitBound(xx + r), ymin = LimitBound(yy - r), ymax = LimitBound(yy + r);
	for (int j = ymax; j >= ymin; j--)
	{
		Gotoxy(offset + 17, y - 4 + j - ymin);
		for (int i = xmin; i <= xmax; i++)
			printf("%d  ", landform[i][j]);
	}
	Gotoxy(0, 0);
}
*/