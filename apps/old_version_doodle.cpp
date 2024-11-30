#include "random.h"

#define RECORD 1
#define headline WINDOWH*0.4		//中线高度。
#define DS 1
#define BULLETNUM 15
#define LANDNUM 25 					//不是一个界面中的地面数量，后台绘制游戏窗口和窗口上方一个窗口高度的地面
#define STRINGNUM 6 				//后台一共有多少个弹簧
#define MONSTERNUM 2				//建议一个
#define WINDOW_BOTTOM ( WINDOWH -jump_sum )
#define WINDOW_TOP (-jump_sum)
#define LANDS_SPAN (2 * WINDOWH)							//后台所有的地面所在的一个范围内，绝对值，正数。
#define LANDS_SPAN_BOTTOM (WINDOW_BOTTOM + 0.2 * WINDOWH)	//后台所有的地面从哪里开始回收
#define INTERVAL_LAND (LANDS_SPAN / LANDNUM)				//每一个地面的间隔

#define FALSE 0
#define TRUE 1


#define WINDOWH 1024				
#define WINDOWW 1280				
#define time_for_a_jump 80 				//使用多少帧完成一次完整跳跃
#define V 16 							//普通起跳初速度，四分之一的总用帧乘以V即一次起跳最大上升高度/像素
#define V_X 6							//玩家横向移动的极限速度。
#define STRING_V 30 					//弹簧起跳的初速度
#define JUMP_HEIGHT (V*time_for_a_jump/4) 		//一次起跳最大上升高度/像素
#define BLUELAND_DS 7							//蓝色砖块的最大移动速度,别小于2！
#define FRAGILELAND_DS 8						//易碎砖块的下降速度
#define PI (3.14159)
#define T 120 							//怪物震荡周期(帧)。
#define BULLET_CD 30 					//子弹冷却CD。300
#define BULLET_DS 29
#define FLYING_T 180


//下落加速度的一半,注意那个（float）两个整数作除，不加那个就得0了！
const float a_half = (float)V / time_for_a_jump;

enum player_status
{
	right, 
    right_tojump, 
    left, 
    left_tojump, 
    shooting, 
    shooting_tojump,
	dizzy
};

///子弹类
class bulletclass
{
public:
	friend void contact_monster();
	friend bool create_a_bullet();
	friend void draw_all_bullets(int sum);
	void show(int sum = 0);
private:
	int pos_x;
	int pos_y;
	int random_number;
	int CD=0;//冷却时间，进入live==falsh后开始倒数，小于0即可再次发生。
	bool live=FALSE;
};

///敌人基类
class monster
{
public:
	virtual void show(int sum = 0);
	int base_x;
	int base_y;
	int pos_x;
	int pos_y;
	bool live = FALSE;
	virtual bool is_contact(int x,int y);
	virtual bool be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	int HP;
	int last_t = 0;     //存在了多少帧。
};

///黑洞
class blackhole :public monster
{
public:
	virtual void show(int sum = 0);
	virtual bool is_contact(int x, int y);
};

///飞行小怪
class flying_monster :public monster
{
public:
	virtual void show(int sum = 0);
	virtual bool be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	virtual bool is_contact(int x, int y);
};

///绿色大怪
class green_monster :public monster
{
public:
	virtual void show(int sum = 0);
	virtual bool be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	virtual bool is_contact(int x, int y);
};

///蓝色小怪
class blue_monster :public monster
{
public:
	virtual void show(int sum = 0);
	virtual bool be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	virtual bool is_contact(int x, int y);
};

///红色小怪
class red_monster :public monster
{
public:
	virtual void show(int sum = 0);
	virtual bool be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	virtual bool is_contact(int x, int y);
};

//mini怪
class mini_monster :public monster
{
public:
	virtual void show(int sum = 0);
	virtual bool be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	virtual bool is_contact(int x, int y);
};

///玩家类
class playerclass
{
public:
	///设置玩家的新x，y坐标，以及人物状态
	void set(int nx, int ny, player_status ns);
	///增减玩家x，y坐标的函数
	void move(int dx, int dy);
	///检测到空格输入,记录为上一次尝试发射的时间，距离上一次尝试发射的时间在一定范围内会让玩家保持抬头状态。
	void shot();
	///在图中绘制一次角色。
	void show(int sum = 0);
	///每调用一次为所有时间变量加一，即时间增加一帧
	playerclass& operator++();
	///修改玩家的起跳状态，重置起跳位置为当前值，重置起跳时间，不修改y值
	void adjust_jumping_status(int jump_strength = 0);//跳跃的力度，如果0那就不跳跃，随重力下落，为1则普通跳跃，为2为弹簧跳
	///每帧根据状态调整高度。
	void adjust_y_by_jumping_status();
	///叠加惯性。往移动的方向叠加惯性，如果移动为0那么消减惯性。
	void accumulate_inertia_v(int dx);
	///与怪兽相碰撞。
	void contact_with_monster();

	int pos_x;
	int pos_y;
	//上一次的起跳情况，1是普通跳，2是弹簧跳,3是正在飞行！
	int jump_status;
	///jump_from_y记录上次玩家起跳的位置，这个变量用于规划玩家的跳跃运动轨迹。
	int jump_from_y;
	///bottom_x是判断玩家是否与地面接触的底部x点坐标
	int bottom_x();
	///bottom_y是判断玩家是否与地面接触的底部y点坐标
	int bottom_y();
	///contact_x是判断玩家是否与物品接触的x点坐标
	int contact_x();
	///contact_y是判断玩家是否与物品接触的y点坐标
	int contact_y();
	player_status status;
	player_status status_before_falling;
	int flying_t = -1;//这个是时间变量，记录开始飞行了多少帧。如果是-1说明没有起飞。
	int t;//这个是时间变量，用于记录当前距离上一次起跳经过了多少帧
	int shooting_t;//这个是时间变量，用于记录当前距离上一次发射经过了多少帧。
	int dizzy_t = -1;//从开始被撞晕的过了多少帧。如果是负一说明没有被撞晕。
	double inertia_v;//惯性速度，移动可以叠加惯性，停下会减少惯性。惯性会带来而外的速度，注意是带方向的！
};

///一个地面的虚基类
class landclass
{
public:
	///虚显示函数,就算啥也不干也必须要写，因为这个land类不是纯虚函数！
	virtual void show(int sum = 0);
	///当判断出发生碰撞时调用。
	virtual void contact();
	///这个函数返回是否与地面发生碰撞。
	virtual bool is_contact(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	int pos_x = 0;
	int pos_y = 0;
	bool live = FALSE;
	static int background_width;
	static int background_height;
};

///绿色地面类
class greenlandclass: public landclass
{
public:
	//这一层的virtual可写可不写，因为是最上的一层，如果还有其他类继承Greenlandclass，还是得写上的
	///显示一块绿地面。
	void show(int sum = 0) override;
};

///蓝色地面类
class bluelandclass : public landclass
{
public:
	void show(int sum = 0) override;
	//每个蓝砖生成时产生一个方向变量，1或者-1。rand()%2得0,1再乘二得0,2，减一得-1或1。
	int direction = 2 * (rand() % 2) - 1;
	int speed = rand() % (BLUELAND_DS - 1) + 2;
};

///脆弱地面类
class fragilelandclass:public landclass
{
public:
	void show(int sum = 0) override;
	void contact() override;
	bool broken = FALSE;
	int broken_t = 0;//距离破碎的时间
};

///弹簧类
class stringlandclass:public landclass
{
public:
	void show(int sum = 0) override;
	void contact() override;
	bool triggerd = FALSE;//是否已经被触发
	bool is_contact(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	landclass* base=nullptr;
	int relative_x = 0;
};

///火箭道具,由于他是基于玩家类和地面类的，要放在最下面！
class rocketclass
{
public:
	void show(int sum);
	bool is_contact(int x, int y);
	int pos_x;
	int pos_y;
	int triggerd_t = -1;
	int base_y;
	int base_x;
	int falling_t = -1;
	landclass* base = nullptr;
	playerclass* base_player = nullptr;
	bool live = FALSE;
};
//-------------------------------------------------------------------------------
// 计算exp
double exp_approx(double x) 
{
    const int n = 10;
    double result = 1.0;
    double term = 1.0;
    for (int i = 1; i <= n; ++i) 
	{
        term *= x / i;
        result += term;
    }
    return result;
}

///每调用一次为所有时间变量加一，即时间增加一帧
playerclass& playerclass::operator++()
{
	t++;
	shooting_t++;
	//距离上一次发射的帧数超过40时则取消发射姿态
	if (shooting_t == 40 && status !=dizzy)
	{
		status = right;
	}
	if (dizzy_t >= 0)
	{
		dizzy_t++;
	}
	if (flying_t >= 0)
	{
		flying_t++;
	}
	return *this;
}
///bottom_x是判断玩家是否与地面接触的底部x点坐标
int  playerclass::bottom_x()
{
	return pos_x + 52;
}
///bottom_y是判断玩家是否与地面接触的底部y点坐标
int  playerclass::bottom_y()
{
	return pos_y + 116;
}
///contact_x是判断玩家是否与物品接触的x点坐标
int  playerclass::contact_x()
{
	return pos_x + 65;
}
///contact_y是判断玩家是否与物品接触的y点坐标
int  playerclass::contact_y()
{
	return pos_y + 60;
}



///与怪兽相碰撞。
void playerclass::contact_with_monster()
{
	if (status ==dizzy)//如果是特殊状态计算与怪物碰撞了也不反应。
	{
		return;
	}
	if (status == right || status == right_tojump)
	{
		status_before_falling = right;
	}
	else if (status == left || status == left_tojump)
	{
		status_before_falling = left;
	}
	else
	{
		status_before_falling = right;
	}
	status = dizzy;
	dizzy_t = 0;
	return;
}
///初始设置设置玩家的新x，y坐标，以及人物状态
void playerclass::set(int nx, int ny, player_status ns)
{
	pos_x = nx;
	pos_y = ny;
	status = ns;
	jump_from_y = pos_y;
	return;
}
///每帧根据状态调整高度
///是根据上一其他位置来修改pos_y的！跟所处的pos_y无关！
void playerclass::adjust_y_by_jumping_status()
{
	int ds;
	//根据上一次的起跳情况调节高度变化。
	switch (jump_status)
	{
	case 1:
		//这里有个坑！坐标要减去跳跃量!画布上的是y反的！
		pos_y = jump_from_y - ((V * t) - floor(a_half * t * t));
		break;
	case 2:
		pos_y = jump_from_y - ((STRING_V * t) - floor(a_half * t * t));
		break;
	case 3:
		ds = 25 * (1 - exp_approx(flying_t - FLYING_T));
		if (ds > V)
		{
			pos_y -= ds;
		}
		else
		{
			this->adjust_jumping_status(1);
			flying_t = -1;
		}
	default:
		break;
	}
	return;
}
///起跳函数，每次调用根据t和是否输入true来修改玩家当前的y值，注意！
void playerclass::adjust_jumping_status(int jump_strength)
{
	if (status == dizzy)//如果眩晕了不允许改变状态
	{
		return;
	}
	if (jump_strength==1)//普通跳
	{
		t = 0;
		jump_from_y = pos_y;
		jump_status = 1;
	}
	else if (jump_strength == 2)
	{
		t = 0;
		jump_from_y = pos_y;
		jump_status = 2;
	}
	else if (jump_strength == 3)
	{
		flying_t = 0;
		jump_from_y = pos_y;
		jump_status = 3;
	}
	return;
}
///叠加惯性。往移动的方向叠加惯性，如果移动为0那么消减惯性
void playerclass::accumulate_inertia_v(int dx)
{
	if (dx == 0)
	{
		if (fabs(inertia_v) < 1E-6)
		{
			return;
		}
		else
		{
			inertia_v -= 0.3*inertia_v / fabs(inertia_v);//这个0.3决定了惯性的衰减速度！可以修改，小于每帧增加的惯性即可
		}
	}
	else
	{
		if (status == dizzy)//如果眩晕了不允许叠加惯性
		{
			return;
		}
		if (fabs(inertia_v) < V_X)//极限速度
		{
			inertia_v += 2* dx / abs(dx);//移动时每帧增加惯性2个，可以修改
		}
	}
	return;
}
///增减玩家x，y坐标的函数
void playerclass::move(int dx, int dy) 
{
	if (status == dizzy)
	{
		dx = 0;//这样就保证玩家如果眩晕了会一直眩晕，无法操作
	}
	if (dx < 0)
	{
		status = left;
	}
	else if (dx > 0)
	{
		status = right;
	}
	pos_x += dx + round(inertia_v);
	pos_y += dy;
	//如果进入了屏幕两侧要可以从另外一侧出现
	if (pos_x < -80)
	{
		pos_x += WINDOWW + 60;
		return;
	}
	if (pos_x > WINDOWW-20)
	{
		pos_x -= WINDOWW + 60;
		return;
	}
	return;
}
///在图中绘制一次角色
void playerclass::show(int sum)
{
	//距离上一次尝试发射的时间在一定范围内会让玩家保持抬头状态
	if (shooting_t < 40 && status !=dizzy)status = shooting;
	if (t < time_for_a_jump / 4 && status != dizzy)status = (player_status)(status + 1);
	//这个的目的是，如果t<10说明刚刚起跳,应该显示起跳动作，
	//正好起跳状态是正常状态的下一位，所以这里加一是把状态转换为当前状态对应的起跳状态
	switch (status)
	{
	case right:
		putpng(pos_x, pos_y+sum, 124, 120, &rimage, 372, 0);
		//line(0, bottom_y() + sum, WINDOWW, bottom_y() + sum);//测试定位线
		//line(bottom_x(), 0, bottom_x(), WINDOWH);
		break;
	case right_tojump:
		putpng(pos_x, pos_y+sum, 124, 120, &rimage, 248, 0);
		//line(0, bottom_y() + sum, WINDOWW, bottom_y() + sum);
		//line(bottom_x(), 0, bottom_x(), WINDOWH);
		break;
	case left:
		putpng(pos_x, pos_y+sum, 124, 120, &limage, 0, 0);
		//line(0, bottom_y() + sum, WINDOWW, bottom_y() + sum);
		//line(bottom_x(), 0, bottom_x(), WINDOWH);
		break;
	case left_tojump:
		putpng(pos_x, pos_y+sum, 124, 120, &limage, 124, 0);
		//line(0, bottom_y() + sum, WINDOWW, bottom_y() + sum);
		//line(bottom_x(), 0, bottom_x(), WINDOWH);
		break;
	case shooting:
		//先打印嘴巴，再打印身体。
		putpng(pos_x + 52, pos_y+sum, 24, 38, &atlas, 894, 770);
		putpng(pos_x, pos_y+sum, 124, 120, &rimage, 124, 0);
		break;
	case shooting_tojump:
		putpng(pos_x + 52, pos_y+sum, 24, 38, &atlas, 894, 770);
		putpng(pos_x, pos_y+sum, 124, 120, &rimage, 0, 0);
		break;
	case dizzy:
		if (status_before_falling == right)
		{
			putpng(pos_x, pos_y + sum, 124, 120, &rimage, 372, 0);
			if (dizzy_t % 9 < 3)
			{
				putpng(pos_x+12, pos_y + sum, 100, 69, &atlas, 870, 0);
			}
			else if (dizzy_t % 9 < 6)
			{
				putpng(pos_x+12, pos_y + sum, 100, 69, &atlas, 870, 138);
			}
			else
			{
				putpng(pos_x+12, pos_y + sum, 100, 69, &atlas, 870, 69);
			}
		}
		else
		{
			putpng(pos_x, pos_y + sum, 124, 120, &limage, 0, 0);
			if (dizzy_t % 9 < 3)
			{
				putpng(pos_x+12, pos_y + sum, 100, 69, &atlas, 870, 0);
			}
			else if (dizzy_t % 9 < 6)
			{
				putpng(pos_x+12, pos_y + sum, 100, 69, &atlas, 870, 138);
			}
			else
			{
				putpng(pos_x+12, pos_y + sum, 100, 69, &atlas, 870, 69);
			}
		}
		break;
	default:
		break;
	}
	if (t < time_for_a_jump/4 && status != dizzy)status = (player_status)(status - 1);
	//记得要变回来！
	return;
}
///重置距离上一次发射的帧数为0，距离上一次尝试发射的时间在一定范围内会让玩家保持抬头状态。
void playerclass::shot()
{
	shooting_t = 0;
	return;
}

///当判断出发生碰撞时调用。
void landclass::contact() 
{
	//虚基类的碰撞啥事没有
}
///这个函数返回是否与地面发生碰撞。
bool landclass::is_contact(int last_t_bottom_y,int player_bottom_x,int player_bottom_y)
{
	//-20,+130,为了让玩家摸到一点砖也能跳
	if ((last_t_bottom_y < pos_y+2) && (player_bottom_y >= pos_y+2)
		&& (player_bottom_x >= pos_x - 20) && (player_bottom_x <= pos_x + 130))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
///虚显示函数,必须要写，因为land类不是纯虚函数！
void landclass::show(int sum)
{
	//啥都不干
}

///子弹的发射函数,子弹在atlas的939，291处，长21，高22。
void bulletclass::show(int sum)
{
	//一开始希望利用伪随机数的一个特性,我们把子弹的系数作为随机数的种子，生成一个伪随机数
	//那么对于同一个子弹，他的伪随机数是相同的，所以他每次绘制的x轴偏移量都是相同的
	//但是这样不好，干扰了随机数种子只生成一次的原则！
	//srand(i);
	//time（0）为种子的srand一个程序最好只调用一次！否则会导致每次调用之后第一次的rand（）产生的随机数非常相近！
	//而且就算你用同样的数去重设他，也会导致rand（）从新回到第一个数开始报！
	//尤其如果每帧都重新设置随机数种子，那么rand（）每次都只报第一个数！，每次报的第一个数是是否相近！
	putpng(pos_x, pos_y+sum, 21, 22, &atlas, 939, 291);
	pos_x -= (random_number & 7) - 3;
	pos_y -= BULLET_DS;
	return;
}

///显示一块绿地面
void greenlandclass::show(int sum)
{
	putpng(pos_x, pos_y+sum, 115, 31, &atlas, 316, 893);
	//line(0, pos_y+2 + sum, WINDOWW, pos_y+2 + sum);//定位线，测试时用
	return;
}

///显示一块蓝地面。每次显示自己移动自己的位置！
void bluelandclass::show(int sum)
{
	pos_x += direction * speed;
	//如果砖块撞到了屏幕两侧，0和WINDOWW-115则调换方向
	if (pos_x < 0 || pos_x>WINDOWW-115)
	{
		direction = -direction;
	}
	putpng(pos_x, pos_y + sum, 115, 31, &atlas, 316, 924);
	return;
}

///当脆弱砖块发生碰撞时
void fragilelandclass::contact()
{
	broken = TRUE;
	return;
}
///显示脆弱砖块
void fragilelandclass::show(int sum)
{
	//如果没坏
	if (!broken)
	{
		putpng(pos_x, pos_y + sum, 121, 31, &atlas, 324, 826);
	}
	//如果坏了
	else
	{
		if (broken_t < 5)
		{
			putpng(pos_x, pos_y + sum, 121, 38, &atlas, 826, 379);
		}
		else if (broken_t < 10)
		{
			putpng(pos_x, pos_y + sum, 121, 58, &atlas, 652, 562);
		}
		else
		{
			putpng(pos_x, pos_y + sum, 121, 65, &atlas, 826, 444);
		}
		pos_y += FRAGILELAND_DS;
		broken_t++;
	}
	return;
}

///显示弹簧
void stringlandclass::show(int sum)
{
	//跟随基地面移动
	pos_x = base->pos_x + 18 + relative_x;
	pos_y = base->pos_y - 20;
	//如果弹簧被触发了
	if (triggerd)
	{
		putpng(pos_x, pos_y + sum - 33, 34, 54, &atlas, 523, 858);
	}
	else
	{
		putpng(pos_x, pos_y + sum, 34, 23, &atlas, 482, 782);
	}
	return;
}
//当弹簧发生碰撞时（被触发时）
void stringlandclass::contact()
{
	triggerd = TRUE;
}
//重载与弹簧接触的条件
bool stringlandclass::is_contact(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	//-20,+,为了让玩家摸到一点弹簧也能跳
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 43))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

///敌人基类的空实现
void monster::show(int sum)
{
	//nothing
}
bool monster::is_contact(int x, int y)
{
	return FALSE;
}
bool monster::be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	return FALSE;
}

///红色小怪的重载
void red_monster::show(int sum)
{
	if (HP < 1) 
	{
   		if (HP--==0)//意思是这个if里面是死亡之后的坐标初始化，死亡后只调用一次
		{
			base_x = pos_x;
			base_y = pos_y;
			last_t=0;
		}
		pos_y = base_y - ((8 * last_t) - floor(a_half * last_t * last_t));
		pos_x = pos_x + 3*((base_x % 2) * 2 - 1);//((base_x % 2) * 2 - 1)这个是根据base_x产生的随机数，1或-1
		putpng(pos_x, pos_y + sum, 97, 70, &atlas, 873, 219);
		if (last_t % 9 < 3)
		{
			putpng(pos_x -5, pos_y -35+ sum, 100, 69, &atlas, 870, 0);
		}
		else if (last_t % 9 < 6)
		{
			putpng(pos_x -5, pos_y -35+ sum, 100, 69, &atlas, 870, 138);
		}
		else
		{
			putpng(pos_x -5, pos_y -35+ sum, 100, 69, &atlas, 870, 69);
		}
		last_t++;
		return;
	}
	last_t++;
	pos_x = base_x + round(30 * sin(last_t * 6 * PI / T));
	pos_y = base_y + round(15 * sin(last_t * PI / T));
	putpng(pos_x, pos_y + sum, 97, 70, &atlas, 873, 219);
	//line(pos_x-20, pos_y + sum, pos_x + 97+15, pos_y + sum);
	//line(pos_x-20, pos_y + 70+20 + sum, pos_x + 97+15, pos_y + 70+20 + sum);
	//line(pos_x-20, pos_y + sum, pos_x-20, pos_y + 70+20 + sum);
	//line(pos_x + 97+15, pos_y + sum, pos_x + 97+15, pos_y + 70+20 + sum);
	return;
}
bool red_monster::is_contact(int x, int y)
{
	if (x >= pos_x-20 && x< pos_x + 97+15 && y>pos_y && y < pos_y + 70+20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
bool red_monster::be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 70 + 10))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

///绿色大怪的重载
void green_monster::show(int sum)
{
	if (HP < 1)
	{
		if (HP-- == 0)//意思是这个if里面是死亡之后的坐标初始化，死亡后只调用一次
		{
			base_x = pos_x;
			base_y = pos_y;
			last_t = 0;
		}
		pos_y = base_y - ((8 * last_t) - floor(a_half * last_t * last_t));
		pos_x = pos_x + 3 * ((base_x % 2) * 2 - 1);//((base_x % 2) * 2 - 1)这个是根据base_x产生的随机数，1或-1
		putpng(pos_x, pos_y + sum, 163, 120, &atlas, 0, 601);
		if (last_t % 9 < 3)
		{
			putpng(pos_x + 25, pos_y - 35 + sum, 100, 69, &atlas, 870, 0);
		}
		else if (last_t % 9 < 6)
		{
			putpng(pos_x + 25, pos_y - 35 + sum, 100, 69, &atlas, 870, 138);
		}
		else
		{
			putpng(pos_x + 25, pos_y - 35 + sum, 100, 69, &atlas, 870, 69);
		}
		last_t++;
		return;
	}
	last_t++;
	pos_x = base_x + round(10 * sin(last_t * 2 * PI / T));
	pos_y = base_y + round(6 * sin(last_t * PI / T));
	if (HP > 8)
	{
		putpng(pos_x, pos_y + sum, 163, 103, &atlas, 0, 721);
	}
	else if (HP > 4)
	{
		putpng(pos_x, pos_y + sum, 163, 105, &atlas, 0, 496);
	}
	else
	{
		putpng(pos_x, pos_y + sum, 163, 120, &atlas, 0, 601);
	}
	//line(pos_x - 20, pos_y + sum, pos_x + 163 + 15, pos_y + sum);
	//line(pos_x - 20, pos_y + 103 + 20 + sum, pos_x + 163 + 15, pos_y + 103 + 20 + sum);
	//line(pos_x - 20, pos_y + sum, pos_x - 20, pos_y + 103 + 20 + sum);
	//line(pos_x + 163 + 15, pos_y + sum, pos_x + 163 + 15, pos_y + 103 + 20 + sum);
	return;
}
bool green_monster::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x< pos_x + 163 + 15 && y>pos_y && y < pos_y + 103 + 20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
bool green_monster::be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 103 + 10))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

///mini小怪重载
void mini_monster::show(int sum)
{
	if (HP < 1)
	{
		if (HP-- == 0)//意思是这个if里面是死亡之后的坐标初始化，死亡后只调用一次
		{
			base_x = pos_x;
			base_y = pos_y;
			last_t = 0;
		}
		pos_y = base_y - ((8 * last_t) - floor(a_half * last_t * last_t));
		pos_x = pos_x + 3 * ((base_x % 2) * 2 - 1);//((base_x % 2) * 2 - 1)这个是根据base_x产生的随机数，1或-1
		putpng(pos_x, pos_y + sum, 110, 98, &atlas, 652, 630);
		if (last_t % 9 < 3)
		{
			putpng(pos_x + 5, pos_y - 35 + sum, 100, 69, &atlas, 870, 0);
		}
		else if (last_t % 9 < 6)
		{
			putpng(pos_x + 5, pos_y - 35 + sum, 100, 69, &atlas, 870, 138);
		}
		else
		{
			putpng(pos_x + 5, pos_y - 35 + sum, 100, 69, &atlas, 870, 69);
		}
		last_t++;
		return;
	}
	last_t++;
	pos_x = base_x + round(20 * sin(last_t * 10 * PI / T));
	pos_y = base_y + round(15 * sin(last_t * 2*PI / T));
	putpng(pos_x, pos_y + sum, 110, 98, &atlas, 652, 630);
	//line(pos_x-20, pos_y + sum, pos_x + 110+15, pos_y + sum);
	//line(pos_x-20, pos_y + 98+20 + sum, pos_x + 110+15, pos_y + 98+20 + sum);
	//line(pos_x-20, pos_y + sum, pos_x-20, pos_y + 98+20 + sum);
	//line(pos_x + 110+15, pos_y + sum, pos_x + 110+15, pos_y + 98+20 + sum);
	return;
}
bool mini_monster::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x< pos_x + 110 + 15 && y>pos_y && y < pos_y + 98 + 20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
bool mini_monster::be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 110 + 10))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//黑洞重载
void blackhole::show(int sum)
{
	last_t++;
	pos_x = base_x;
	pos_y = base_y;
	putpng(pos_x, pos_y + sum, 136, 131, &atlas, 495 ,650);
	//line(pos_x - 20, pos_y + sum, pos_x + 136 + 15, pos_y + sum);
	//line(pos_x - 20, pos_y + 131 + 15 + sum, pos_x + 136 + 15, pos_y + 131 + 15 + sum);
	//line(pos_x - 20, pos_y + sum, pos_x - 20, pos_y + 131 + 15 + sum);
	//line(pos_x + 136 + 15, pos_y + sum, pos_x + 136 + 15, pos_y + 131 + 15 + sum);
	return;
}
bool blackhole::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x< pos_x + 136 + 15 && y>pos_y && y < pos_y + 131 + 15)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

///蓝色小怪的重载
void blue_monster::show(int sum)
{
	if (HP < 1)
	{
		if (HP-- == 0)//意思是这个if里面是死亡之后的坐标初始化，死亡后只调用一次
		{
			base_x = pos_x;
			base_y = pos_y;
			last_t = 0;
		}
		pos_y = base_y - ((8 * last_t) - floor(a_half * last_t * last_t));
		pos_x = pos_x + 3 * ((base_x % 2) * 2 - 1);//((base_x % 2) * 2 - 1)这个是根据base_x产生的随机数，1或-1
		putpng(pos_x, pos_y + sum, 75, 98, &atlas, 447, 858);
		if (last_t % 9 < 3)
		{
			putpng(pos_x - 5, pos_y - 35 + sum, 100, 69, &atlas, 870, 0);
		}
		else if (last_t % 9 < 6)
		{
			putpng(pos_x - 5, pos_y - 35 + sum, 100, 69, &atlas, 870, 138);
		}
		else
		{
			putpng(pos_x - 5, pos_y - 35 + sum, 100, 69, &atlas, 870, 69);
		}
		last_t++;
		return;
	}
	last_t++;
	if (last_t == 1)
	{
		pos_x = base_x;
	}
	pos_x = pos_x - 3;
	if (pos_x < -80)
	{
		pos_x += WINDOWW + 60;
	}
	pos_y = base_y + round(15 * sin(last_t * PI / T));
	putpng(pos_x, pos_y + sum, 75, 98, &atlas, 447, 858);
	//line(pos_x-20, pos_y + sum, pos_x + 75+15, pos_y + sum);
	//line(pos_x-20, pos_y + 98+20 + sum, pos_x + 75+15, pos_y + 98+20 + sum);
	//line(pos_x-20, pos_y + sum, pos_x-20, pos_y + 98+20 + sum);
	//line(pos_x + 75+15, pos_y + sum, pos_x + 75+15, pos_y + 98+20 + sum);
	return;
}
bool blue_monster::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x< pos_x + 75 + 15 && y>pos_y && y < pos_y + 98 + 20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
bool blue_monster::be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 75 + 10))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void flying_monster::show(int sum)
{
	if (HP < 1)
	{
		if (HP-- == 0)//意思是这个if里面是死亡之后的坐标初始化，死亡后只调用一次
		{
			base_x = pos_x;
			base_y = pos_y;
			last_t = 0;
		}
		pos_y = base_y - ((8 * last_t) - floor(a_half * last_t * last_t));
		pos_x = pos_x + 3 * ((base_x % 2) * 2 - 1);//((base_x % 2) * 2 - 1)这个是根据base_x产生的随机数，1或-1
		if (last_t % 9 < 3)
		{
			putpng(pos_x, pos_y + sum, 150, 88, &atlas, 675, 378);
			putpng(pos_x + 25, pos_y - 35 + sum, 100, 69, &atlas, 870, 0);
		}
		else if (last_t % 9 < 6)
		{
			putpng(pos_x, pos_y + sum, 156, 87, &atlas, 495, 562);
			putpng(pos_x + 25, pos_y - 35 + sum, 100, 69, &atlas, 870, 138);
		}
		else
		{
			putpng(pos_x, pos_y + sum, 150, 83, &atlas, 675, 468);
			putpng(pos_x + 25, pos_y - 35 + sum, 100, 69, &atlas, 870, 69);
		}
		last_t++;
		return;
	}
	last_t++;
	pos_x = base_x + round(10 * sin(last_t * 2 * PI / T));
	pos_y = base_y + round(6 * sin(last_t * PI / T));
	if (last_t%9<=3)
	{
		putpng(pos_x, pos_y + sum, 150, 88, &atlas, 675, 378);
	}
	else if (last_t % 9 <= 6)
	{
		putpng(pos_x, pos_y + sum, 156, 88, &atlas, 495, 561);
	}
	else
	{
		//putpng(pos_x, pos_y + sum, 156, 87, &atlas, 495, 562);
		putpng(pos_x+2, pos_y + sum, 150, 85, &atlas, 675, 466);
	}
	//line(pos_x - 20, pos_y + sum, pos_x + 150 + 15, pos_y + sum);
	//line(pos_x - 20, pos_y + 88 + 20 + sum, pos_x + 150 + 15, pos_y + 88 + 20 + sum);
	//line(pos_x - 20, pos_y + sum, pos_x - 20, pos_y + 88 + 20 + sum);
	//line(pos_x + 150 + 15, pos_y + sum, pos_x + 150 + 15, pos_y + 88 + 20 + sum);
	return;
}
bool flying_monster::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x< pos_x + 150 + 15 && y>pos_y && y < pos_y + 88 + 20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
bool flying_monster::be_trample(int last_t_bottom_y, int player_bottom_x, int player_bottom_y)
{
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 88 + 10))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//火箭类的重载
void rocketclass::show(int sum)
{
	//如果对应的玩家起飞了
	if (base_player != nullptr && base_player->flying_t >= 0)
	{
		triggerd_t = base_player->flying_t;
		pos_x = base_player->pos_x;
		pos_y = base_player->pos_y;
		if (base_player->status == right || base_player->status == right_tojump || base_player->status == shooting || base_player->status == shooting_tojump)
		{
			setaspectratio(-1, 1);
			if (triggerd_t < 15 || (FLYING_T - triggerd_t) < 15)
			{
				putpng(-(pos_x + 48), pos_y + 35 + sum, 50, 125, &atlas, 906, 641);
			}
			else if (triggerd_t < 30 || (FLYING_T - triggerd_t) < 30)
			{
				putpng(-(pos_x + 48), pos_y + 35 + sum, 50, 125, &atlas, 846, 641);
			}
			else if ((triggerd_t % 6) < 3)
			{
				putpng(-(pos_x + 48), pos_y + 35 + sum, 50, 125, &atlas, 846, 512);
			}
			else
			{
				putpng(-(pos_x + 48), pos_y + 35 + sum, 50, 125, &atlas, 906, 512);
			}
			setaspectratio(1, 1);
		}
		else
		{
			if (triggerd_t < 15 || (FLYING_T - triggerd_t) < 15)
			{
				putpng(pos_x+75, pos_y+35 + sum, 50, 125, &atlas, 906, 641);
			}
			else if (triggerd_t < 30 || (FLYING_T - triggerd_t) < 30)
			{
				putpng(pos_x+75, pos_y+35 + sum, 50, 125, &atlas, 846, 641);
			}
			else if ((triggerd_t % 6) < 3)
			{
				putpng(pos_x+75, pos_y+35 + sum, 50, 125, &atlas, 846, 512);
			}
			else
			{
				putpng(pos_x+75, pos_y+35 + sum, 50, 125, &atlas, 906, 512);
			}
		}
		return;
	}
	if (triggerd_t > 0)
	{
		if (falling_t<0)
		{
			base_y = pos_y;
			base_x = pos_x;
			falling_t = 0;
		}
		pos_y = base_y - ((8 * falling_t) - floor(a_half * falling_t * falling_t));
		pos_x = pos_x + 3 * ((base_x % 2) * 2 - 1);
		putpng(pos_x, pos_y + sum, 50, 100, &atlas, 769, 812);
		falling_t++;
		return;
	}
	//跟随基地面移动
	pos_x = base->pos_x + 21 + 10;
	pos_y = base->pos_y - 70;
	putpng(pos_x, pos_y + sum, 50, 73, &atlas, 825, 835);
	//line(pos_x - 20, pos_y + sum, pos_x + 50 + 20, pos_y + sum);
	//line(pos_x - 20, pos_y + 73 + 20 + sum, pos_x + 50 + 20, pos_y + 73 + 20 + sum);
	//line(pos_x - 20, pos_y + sum, pos_x - 20, pos_y + 73 + 20 + sum);
	//line(pos_x + 50 + 20, pos_y + sum, pos_x + 50 + 20, pos_y + 73 + 20 + sum);
	return;
}
bool rocketclass::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x< pos_x + 50 + 20 && y>pos_y-40 && y < pos_y + 73 + 20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//-------------------------------------------------------------------------------

const type_info& landclass_info = typeid(landclass);
int jump_sum;//累计超过中线的高度,是一个正数
IMAGE player_img;
IMAGE player_img2;
IMAGE atlas2;
IMAGE atlas;
IMAGE font;
rocketclass rocket;
playerclass player;
bulletclass *bullets[15];//子弹没有多态需求，也不十分耗内存，可以不使用指针，但这里统一格式，设置为与和其他的数组一致，更容易管理
stringlandclass* strings[STRINGNUM];//如果不对这个指针数组进行初始化，这些数组全部默认设置为空指针
landclass* lands[LANDNUM];
monster* monsters[MONSTERNUM];
IMAGE bulletclass::atlas;//这个是初始化bulletclass里的静态变量，由于规定，静态变量必须在主函数文件的外部初始化
IMAGE landclass::atlas;
IMAGE monster::atlas;
IMAGE rocketclass::atlas;
IMAGE printclass::font;
printclass myprint;
int landclass::background_width = WINDOWW;
int landclass::background_height = WINDOWH;
int the_top_land_index;
int the_highest_solid_land_index;//最后一个实体地面，绿色蓝色（不带弹簧的）地面的index，要保证任何两个实体方块间距离小于玩家可以跳的最大距离。
int the_bottom_land_index;
int last_t_bottom_y;//这个静态变量用于储存上一帧玩家的底部高度坐标，如果上一帧到下一帧穿过了地面的平面则视为发生了碰撞。 
int dead_time;
TCHAR instruction[] = _T("press SPACE to restart or press ESC to exit..............");
set<int,std::greater<int>> scores;//储存分数

///扫描每一个怪物是否有人物，子弹碰撞
void contact_monster()
{
	for (int i = 0; i < MONSTERNUM; i++)
	{
		//如果怪物不存在，或者已经被关掉显示了，或者已经被打了，则不需要在判读是否相碰撞
		if (monsters[i] == nullptr || monsters[i]->live == FALSE || monsters[i]->HP < 1)
		{
			continue;
		}
		for (int j = 0; j < BULLETNUM; j++)//如果子弹与怪物相碰
		{
			if ((bullets[j] != nullptr && bullets[j]->live == TRUE) && typeid(*monsters[i]) != typeid(blackhole))
			{
				if (monsters[i]->is_contact(bullets[j]->pos_x - +10, bullets[j]->pos_y + 10))
				{
					bullets[j]->live = FALSE;
					bullets[j]->CD = BULLET_CD;
					monsters[i]->HP--;
				}
			}
		}
		//如果有玩家相撞
		if (player.status != dizzy&& monsters[i]->is_contact(player.contact_x(), player.contact_y()))//注意防止玩家已经晕了重复碰撞
		{
			//如果玩家正在飞行，直接撞飞
			if (player.flying_t >= 0)
			{
				monsters[i]->HP=0;
			}
			else
			{
				player.contact_with_monster();
			}
		}
	}
	return;
}

void contact_rocket()
{
	if (rocket.live == FALSE)
	{
		return;
	}
	//判断玩家是否摸到了火箭，注意确保玩家没有晕，注意防止反复碰撞！
	if (player.flying_t < 0 &&rocket.triggerd_t<0&& player.status != dizzy && rocket.is_contact(player.contact_x(), player.contact_y()))
	{
		rocket.base_player = &player;
		rocket.live == FALSE;
		player.adjust_jumping_status(3);
	}
	return;
}

///扫描是否踩在了怪物头上。
void on_monster()
{
	for (int i = 0; i < MONSTERNUM; i++)
	{
		if (monsters[i] == nullptr || monsters[i]->live == FALSE || monsters[i]->HP < 1 || typeid(*monsters[i]) == typeid(blackhole))
		{
			continue;
		}
		if (player.status != dizzy && monsters[i]->be_trample(last_t_bottom_y, player.bottom_x(), player.bottom_y()))//如果玩家与怪物相碰,注意防止重复碰撞
		{
			player.pos_y += (monsters[i]->pos_y - player.bottom_y());//因为有可能上一帧在地面上，但是下一帧已经在地面下了，不能从地面下其他，要将人物移动到地面上
			player.adjust_jumping_status(1);
			monsters[i]->HP = 0;
			break;
		}
	}
}

///查找是否与任何一个地面或弹簧相碰撞
void on_string()
{
	//扫描弹簧
	for (int i = 0; i < STRINGNUM; i++)
	{
		if (strings[i] == nullptr || strings[i]->live == FALSE)
		{
			continue;
		}
		if (player.status != dizzy && (player.t >= time_for_a_jump / 2) && strings[i]->is_contact(last_t_bottom_y, player.bottom_x(), player.bottom_y()))
		{
			player.pos_y += (strings[i]->pos_y - player.bottom_y());//因为有可能上一帧在地面上，但是下一帧已经在地面下了，不能从地面下其他，要将人物移动到地面上
			player.adjust_jumping_status(2);
			strings[i]->contact();
			break;
		}
	}
	return;
}

///查找是否与任何一个地面相碰撞
void on_land()
{
	//扫描地面
	for (int i = 0; i < LANDNUM; i++)
	{
		//如果判断这个地面为基类地面，那么不需要进入碰撞判断
		// 下面的typeid(landclass)用landclass_info也可以
		if (lands[i]->live == FALSE || typeid(*lands[i]) == typeid(landclass))
		{
			continue;
		}
		//player.t >= time_for_a_jump / 2 的意思是玩家正在下落
		if (player.status != dizzy && (player.t >= time_for_a_jump / 2) && lands[i]->is_contact(last_t_bottom_y, player.bottom_x(), player.bottom_y()))
		{
			//如果碰撞的是蓝色地面
			if (typeid(*lands[i]) == typeid(greenlandclass))
			{
				player.pos_y += (lands[i]->pos_y - player.bottom_y());//因为有可能上一帧在地面上，但是下一帧已经在地面下了，不能从地面下其他，要将人物移动到地面
				player.adjust_jumping_status(1);
				break;
			}
			//如果碰撞的是蓝色地面
			if (typeid(*lands[i]) == typeid(bluelandclass))
			{
				player.pos_y += (lands[i]->pos_y - player.bottom_y());//因为有可能上一帧在地面上，但是下一帧已经在地面下了，不能从地面下其他，要将人物移动到地面
				player.adjust_jumping_status(1);
				break;
			}
			//如果碰撞的是白色地面
			if (typeid(*lands[i]) == typeid(whitelandclass))
			{
				player.pos_y += (lands[i]->pos_y - player.bottom_y());//因为有可能上一帧在地面上，但是下一帧已经在地面下了，不能从地面下其他，要将人物移动到地面
				player.adjust_jumping_status(1);
				lands[i]->contact();
				break;
			}
			//如果碰撞的是易碎地面
			if (typeid(*lands[i]) == typeid(fragilelandclass))
			{
				//不起跳!
				lands[i]->contact();
				break;
			}
		}
	}
	return;
}

///整合两个函数，查找是否与任何一个地面或者弹簧相碰撞,任何刷新上一帧的玩家底部位置。
void on_string_or_land_or_monster()
{
	on_string();
	on_land();
	on_monster();
	last_t_bottom_y = player.bottom_y();
}

///为一个地面生成一个弹簧。
void create_a_string(landclass* base_land)
{
	//先找出一个可以生成的空位。
	for (int i = 0; i < STRINGNUM; i++)
	{
		if ( nullptr == strings[i]  || strings[i]->live == FALSE)
		{
			//如果是nullptr == strings[i]进入的不可以参与判断strings[i]->live == FALSE
			if (nullptr != strings[i])
			{
				delete strings[i];
			}
			strings[i] = new stringlandclass;
			strings[i]->live = TRUE;
			strings[i]->triggerd = FALSE;
			strings[i]->base = base_land;
			strings[i]->relative_x = rand() % 58;
			strings[i]->pos_x = base_land->pos_x + 18 + strings[i]->relative_x;
			strings[i]->pos_y = base_land->pos_y - 20;
			return;
		}
	}
	return;
}

///尝试生成火箭。
void create_rocket(landclass* base_land)
{
	if (rocket.live == FALSE)
	{
		rocket.base = base_land;
		rocket.pos_x = base_land->pos_x;
		rocket.pos_y = base_land->pos_y;
		rocket.triggerd_t = -1;
		rocket.falling_t = -1;
		rocket.live = TRUE;
	}
	return;
}

///尝试生成一个敌人
void create_a_monster(int y)
{
	static int seed;
	for (int i = 0; i<MONSTERNUM; i++)
	{
		if (nullptr == monsters[i] || monsters[i]->live == FALSE)
		{
			if (monsters[i] != nullptr)
			{
				delete monsters[i];
			}
			seed = rand() % ((WINDOWW - 200) + 50);
			if (seed % 100 < 20)
			{
				monsters[i] = new flying_monster;
				monsters[i]->base_x = seed;
				monsters[i]->base_y = y;
				monsters[i]->live = TRUE;
				monsters[i]->last_t = 0;
				monsters[i]->HP = 5;
			}
			else if (seed % 100 < 40)
			{
				monsters[i] = new red_monster;
				monsters[i]->base_x = seed;
				monsters[i]->base_y = y;
				monsters[i]->live = TRUE;
				monsters[i]->last_t = 0;
				monsters[i]->HP = 5;
			}
			else if (seed % 100 < 50)
			{
				monsters[i] = new blackhole;
				monsters[i]->base_x = seed;
				monsters[i]->base_y = y;
				monsters[i]->live = TRUE;
				monsters[i]->last_t = 0;
				monsters[i]->HP = 1;
			}
			else if (seed % 100 < 60)
			{
				monsters[i] = new blue_monster;
				monsters[i]->base_x = seed;
				monsters[i]->base_y = y;
				monsters[i]->live = TRUE;
				monsters[i]->last_t = 0;
				monsters[i]->HP = 5;
			}
			else if (seed % 100 < 80)
			{
				monsters[i] = new mini_monster;
				monsters[i]->base_x = seed;
				monsters[i]->base_y = y;
				monsters[i]->live = TRUE;
				monsters[i]->last_t = 0;
				monsters[i]->HP = 3;
			}
			else
			{
				monsters[i] = new green_monster;
				monsters[i]->base_x = seed;
				monsters[i]->base_y = y;
				monsters[i]->live = TRUE;
				monsters[i]->last_t = 0;
				monsters[i]->HP = 12;
			}
			return;
		}
	}
	return;
}

///尝试生成一个子弹（弹夹还有空就可以生成），生成成功则返回真。
bool create_a_bullet()
{
	//找出一个未被调用（即为live==flase）的子弹，将之位置分配到玩家嘴边。
	for (int i = 0; i < BULLETNUM; i++)
	{
		if (bullets[i] == nullptr || (bullets[i]->live == FALSE && bullets[i]->CD <= 0))
		{
			if (bullets[i] != nullptr)
			{
				delete bullets[i];
			}
			bullets[i] = new bulletclass;
			bullets[i]->live = TRUE;
			bullets[i]->CD = 0;
			bullets[i]->random_number = rand();
			bullets[i]->pos_x = player.pos_x + 52;
			bullets[i]->pos_y = player.pos_y - 20;
			return TRUE;
		}
	}
	return FALSE;
}

///初始化角色数据。
void initplayer()
{
	player.set(WINDOWW * 0.2, WINDOWH * 0.8 - jump_sum, right, player_img2, player_img);
	player.atlas = atlas;
	player.atlas2 = atlas2;
	player.adjust_jumping_status(3);
	player.t = 0;
	player.inertia_v = 0;
	player.shooting_t = 1000;
	return;
}

///绘制初始界面的所有地面
void initlands()
{
	int seed = 0;
	int land_x, land_y;
	for (int i = 0; i < LANDNUM; i++)
	{
		seed = rand() % 3000;
		///land_x指当前这一块地随机生成的x坐标，505=620-115地图宽减去地砖长。
		land_x = seed % (WINDOWW-115);
		///land_y指当前这一块地随机生成的y坐标，初始化时地图高度分成地面数份逐份向上生成。
		land_y = LANDS_SPAN_BOTTOM - i * INTERVAL_LAND;

		///生成绿色地面。
		if (seed < 1800)
		{
			lands[i] = new greenlandclass;
			lands[i]->live = TRUE;
			lands[i]->pos_x = land_x;
			lands[i]->pos_y = land_y;
			if (seed % 5 == 1&&i>4)
			{
				create_a_string(lands[i]);
			}
			else//如果不创建弹簧的话他就是最高的那一个实体地面
			{
				//我们来判断是否存在卡关现象，如果当前这个实体方块离现有（也就是上一个）的最高的实体方块超过了玩家可以起跳的最高高度
				//需要为上一个最高的实体方块创建弹簧，防止出现卡关
				if (lands[the_highest_solid_land_index]->pos_y - land_y > JUMP_HEIGHT)
				{
					create_a_string(lands[the_highest_solid_land_index]);
				}
				else
				{
					//没有超过玩家起跳高度
				}
				//继续将之重置为上一个最高的实体方块
				the_highest_solid_land_index = i;
			}
			continue;
		}
		///生成脆弱地面
		else if (seed < 2500)
		{
			lands[i] = new fragilelandclass;
			lands[i]->live = TRUE;
			lands[i]->pos_x = land_x;
			lands[i]->pos_y = land_y;
			continue;
		}
		///生成空地面（地面基类）
		else
		{
			lands[i] = new landclass;
			lands[i]->live = TRUE;
			lands[i]->pos_x = land_x;
			lands[i]->pos_y = land_y;
			continue;
		}
	}
	//将最后生成的那一个定义为目前最高的地面
	the_top_land_index = LANDNUM - 1;
	return;
}

///初始化所有的全局变量
void initglobal_variable()
{
	int temp;
	jump_sum = 0;
	the_top_land_index = 0;
	the_highest_solid_land_index = 0;
	the_bottom_land_index=0;
	last_t_bottom_y = 0;
	dead_time = -1;
	if (RECORD == 1)
	{
		std::ifstream file;
		file.open("scores.txt");
		if (file.is_open()) {
			while (!file.eof())
			{
				file >> temp;
				scores.insert(temp);
			}
		}
		file.close();
	}
	return;
}

//初始化所有弹簧
void initstrings()
{
	for (int i = 0; i < STRINGNUM; i++)
	{
		strings[i] = nullptr;
	}
	return;
}

///初始化敌人数组
void initmonsters()
{
	for (int i = 0; i < MONSTERNUM; i++)
	{
		monsters[i] = nullptr;
	}
	return;
}

///初始化子弹数组
void initbullets()
{
	for (int i = 0; i < BULLETNUM; i++)
	{
		bullets[i] = nullptr;
	}
	return;
}

///初始化火箭
void initrocket()
{
	rocket.pos_x = 0;
	rocket.pos_y = 0;
	rocket.triggerd_t = -1;
	rocket.falling_t = -1;
	rocket.base_x = 0;
	rocket.base_y = 0;
	rocket.base = nullptr;
	rocket.base_player = nullptr;
	rocket.live = FALSE;
	rocket.base_player = &player;
	return;
}

///一局结束后删除所有的弹簧
void delete_all_strings()
{
	//注意删除掉指针指向的空间后，指针本身没有修改
	for (int i = 0; i < STRINGNUM; i++)
	{
		if (strings[i] != nullptr)
		{
			delete strings[i];
		}
		strings[i] = nullptr;
	}
	return;
}

///一局结束后删掉所有的地面
void delete_all_lands()
{
	for (int i = 0; i < LANDNUM; i++)
	{
		if (lands[i] != nullptr)
		{
			delete lands[i];
		}
		lands[i] = nullptr;
	}
	return;
}

///一局结束后删掉所有的敌人
void delete_all_monsters()
{
	for (int i = 0; i < MONSTERNUM; i++)
	{
		if (monsters[i] != nullptr)
		{
			delete monsters[i];
		}
		monsters[i] = nullptr;
	}
	return;
}

///一局结束后删掉所有的子弹
void delete_all_bullets()
{
	for (int i = 0; i < BULLETNUM; i++)
	{
		if (bullets[i] != nullptr)
		{
			delete bullets[i];
		}
		bullets[i] = nullptr;
	}
	return;
}

///更新当前所有的地图元素。即将已经在屏幕下方（live=FALSE）的地图元素，重新在上方生成
void refresh_all_elements()
{
	///这个blank_land_in_series是用来判断当前是否连续生成的多个空地面（就是没生成），如果连续两个没生成才有空间产生怪物
	static int blank_land_in_series = 0;
	int seed = 0;
	int land_x, land_y;
	//srand(unsigned(time(0)));//这个函数有一个巨坑！time返回的是秒！但是你是每帧都调用一次srand！所以一秒内每一帧接收的种子都一样！一秒内生成的随机数都一样！
	while (lands[the_top_land_index]->pos_y > LANDS_SPAN_BOTTOM - LANDS_SPAN)
	{
		//如果你重复调用srand，一秒内的rand产生的第一个随机数都一样，下一秒rand产生的第一个
		//数会只比上一秒产生的稍微大几而已，余了1000之后，导致seed几乎没有很大变化！
		seed = rand() % 3000;
		///land_x指当前这一块地随机生成的x坐标，505=620-115地图宽减去地砖长
		land_x = seed % (WINDOWW - 115);
		///land_y指当前这一块地随机生成的y坐标，初始化时地图高度分成地面数份逐份向上生成
		land_y = lands[the_top_land_index]->pos_y - INTERVAL_LAND;
		the_top_land_index = the_bottom_land_index;
		delete lands[the_bottom_land_index];
		///生成绿色地面
		if (seed % 100 < 30)
		{
			lands[the_bottom_land_index] = new greenlandclass;
			lands[the_bottom_land_index]->live = TRUE;
			lands[the_bottom_land_index]->pos_x = land_x;
			lands[the_bottom_land_index]->pos_y = land_y;
			if (seed % 10 == 0)//设置成几就算几分之一的概率产生弹簧
			{
				create_a_string(lands[the_bottom_land_index]);
			}
			else//如果不创建弹簧的话他就是最高的那一个实体地面
			{
				//我们来判断是否存在卡关现象，如果当前这个实体方块离现有（也就是上一个）的最高的实体方块超过了玩家可以起跳的最高高度，
				//需要为上一个最高的实体方块创建弹簧，防止出现卡关
				if (lands[the_highest_solid_land_index]->pos_y - land_y > JUMP_HEIGHT)
				{
					create_a_string(lands[the_highest_solid_land_index]);
				}
				else
				{
					if (seed % 10 < 2)
					{
						//如果没有超过玩家起跳高度那么说明上一个最高的实体方块肯定没有弹簧，可以生成火箭。
						create_rocket(lands[the_highest_solid_land_index]);
					}
				}
				//继续将之重置为上一个最高的实体方块。
				the_highest_solid_land_index = the_bottom_land_index;
			}
			the_bottom_land_index = (++the_bottom_land_index) % LANDNUM;
			blank_land_in_series = 0;
			continue;
		}
		//生成蓝色地面
		if (seed % 100 < 40)
		{
			lands[the_bottom_land_index] = new bluelandclass;
			lands[the_bottom_land_index]->live = TRUE;
			lands[the_bottom_land_index]->pos_x = land_x;
			lands[the_bottom_land_index]->pos_y = land_y;
			if (seed % 5 == 1)
			{
				create_a_string(lands[the_bottom_land_index]);
			}
			else//如果不创建弹簧的话他就是最高的那一个实体地面
			{
				//我们来判断是否存在卡关现象，如果当前这个实体方块离现有（也就是上一个）的最高的实体方块超过了玩家可以起跳的最高高度，
				//需要为上一个最高的实体方块创建弹簧，防止出现卡关
				if (lands[the_highest_solid_land_index]->pos_y - land_y > JUMP_HEIGHT)
				{
					create_a_string(lands[the_highest_solid_land_index]);
				}
				else
				{
					if (seed % 10 < 2)
					{
						//如果没有超过玩家起跳高度那么说明上一个最高的实体方块肯定没有弹簧，可以生成火箭
						create_rocket(lands[the_highest_solid_land_index]);
					}
				}
				//继续将之重置为上一个最高的实体方块
				the_highest_solid_land_index = the_bottom_land_index;
			}
			the_bottom_land_index = (++the_bottom_land_index) % LANDNUM;
			blank_land_in_series = 0;
			continue;
		}
		//生成白色地面
		if (seed % 100 < 50)
		{
			lands[the_bottom_land_index] = new whitelandclass;
			lands[the_bottom_land_index]->live = TRUE;
			lands[the_bottom_land_index]->pos_x = land_x;
			lands[the_bottom_land_index]->pos_y = land_y;
			//因为白色砖块不可以设置弹簧，所以就算实际是可以踩的也不设置为实体方块
			the_bottom_land_index = (++the_bottom_land_index) % LANDNUM;
			blank_land_in_series = 0;
			continue;
		}
		//生成易碎地面
		if (seed % 100 < 65)
		{
			lands[the_bottom_land_index] = new fragilelandclass;
			lands[the_bottom_land_index]->live = TRUE;
			lands[the_bottom_land_index]->pos_x = land_x;
			lands[the_bottom_land_index]->pos_y = land_y;
			the_bottom_land_index = (++the_bottom_land_index) % LANDNUM;
			blank_land_in_series = 0;
			continue;
		}
		///生成空地面（地面基类）
		else
		{
			blank_land_in_series++;
			///这个blank_land_in_series是用来判断当前是否连续生成的多个空地面（就是没生成），如果连续没生成才有空间产生怪物
			if (blank_land_in_series >= 3)//这里填即大于几就算空几个就生成。可以修改
			{
				if (seed % 1 == 0)
				{
					create_a_monster(land_y - LANDS_SPAN / (2 * LANDNUM));
					blank_land_in_series = 0;
				}
			}
			lands[the_bottom_land_index] = new landclass;
			lands[the_bottom_land_index]->live = TRUE;
			lands[the_bottom_land_index]->pos_x = land_x;
			lands[the_bottom_land_index]->pos_y = land_y;
			the_bottom_land_index = (++the_bottom_land_index) % LANDNUM;
			continue;
		}
	}
	return;
}

///更新当前的跳跃总高度
void refresh_jump_sum()
{
	if (player.pos_y < headline)
	{
		///jump_sum记为玩家y坐标小于400的最大差值
		jump_sum = max(jump_sum, headline - player.pos_y);
	}
	return;
}

///绘制所有的弹簧，并且将在地图下的标记为FALSE
void draw_all_monsters(int sum)
{	
	for (int i = 0; i < MONSTERNUM; i++)
	{
		//别对还没生成的弹簧操作
		if (monsters[i] == nullptr)
		{
			continue;
		}
		if (monsters[i]->pos_y > WINDOW_BOTTOM )
		{
			monsters[i]->live = FALSE;
		}
		//已经离开窗口的一定不能显示！仍在显示可能导致地面已经被回收弹簧仍在读取地面地面数据！
		if (monsters[i]->live == TRUE)
		{
			draw_a_monster(monsters[i],jump_sum);
		}
	}
	return;
}

///绘制所有的弹簧，并且将在地图下的标记为FALSE
void draw_all_strings(int sum)
{
	for (int i = 0; i < STRINGNUM; i++)
	{
		//别对还没生成的弹簧操作
		if (strings[i] == nullptr)
		{
			continue;
		}
		if (strings[i]->pos_y > WINDOW_BOTTOM)
		{
			strings[i]->live = FALSE;
		}
		//已经离开窗口的一定不能显示！仍在显示可能导致地面已经被回收弹簧仍在读取地面地面数据！
		if (strings[i]->live == TRUE)
		{
			draw_a_land(strings[i], jump_sum);
		}
	}
	return;
}

///绘制当前界面所有的地面。并且将在地图下的标记为FALSE
void draw_all_lands(int sum)
{
	//static landclass* testptr = new stringlandclass;
	//testptr->pos_x = 400;
	//testptr->pos_y = 500;
	//draw_a_land(testptr, 0);
	//显示在窗口范围内的地砖live=TRUE,
	for (int i = 0; i < LANDNUM; i++)
	{
		//如果地砖已经在地图底下的话那就关掉live，不显示了，也不判断碰撞
		if(lands[i]->pos_y > WINDOW_BOTTOM)
		{
			lands[i]->live = FALSE;
		}
		//只输出所有TRUE，即应该显示的砖块
		if (lands[i]->live==TRUE)
		{
			draw_a_land(lands[i], jump_sum);
		}
	}
	return;
}

///绘制出所有的子弹,并且自动移动
void draw_all_bullets(int sum = 0)
{
	for (int i = 0; i < BULLETNUM; i++)
	{
		if (bullets[i] == nullptr)
		{
			continue;
		}
		if (bullets[i]->pos_y < LANDS_SPAN_BOTTOM - LANDS_SPAN)
		{
			bullets[i]->CD = BULLET_CD;
			bullets[i]->live = FALSE;
		}
		if (bullets[i]->live == TRUE)
		{
			bullets[i]->show(sum);
		}
		bullets[i]->CD--;
	}
}

//绘制火箭
void draw_rocket(int sum)
{
	if (rocket.pos_y > WINDOW_BOTTOM)
	{
		rocket.live = FALSE;
	}
	if (player.flying_t >= 0)//如果玩家正在飞行，那么直接复活火箭，让它飞到玩家身上
	{
		rocket.live = TRUE;
	}
	if (rocket.live == TRUE)
	{
		rocket.show(sum);
	}
	return;
}

//绘制所有底层UI，如背景
void draw_background_UI(int sum = 0)
{
	//先绘制背景图。
	//atlas2 的0,0点是背景图片
	setaspectratio(WINDOWW * XASP / 620.0, WINDOWH * YASP / 820.0);
	putimage(0, 0, 620, 820, &atlas2, 0, 0);
	setaspectratio(XASP, YASP);
	return;
}

//绘制所有上层UI，如计分。
void draw_upper_UI(int sum = 0)
{
	//这个函数可以绘制半透明物体
	// 透明状态栏可以打开
	//drawAlpha(0, 0, 640 * XASP, 105 * YASP, &atlas, 0, 132, WINDOWW * XASP, WINDOWH * YASP, 0.5);
	//drawAlpha(640 * XASP, 0, 640 * XASP, 105 * YASP, &atlas, 0, 132, WINDOWW * XASP, WINDOWH * YASP, 0.5);
	myprint(to_string(jump_sum/10), 30, 30);
	myprint(to_string(show_FPS()),500,30);
	return;
}

///用于读取用户的键盘与鼠标输入。还有子弹的生成也在其中。
void player_control()
{	
	if (_kbhit()) //如果有键盘按下则进入语块
	{
		//别用_getch()！这个函数相较于直接调用windowapi的效率低的多的多的多！会导致非常卡！
		//那是因为_getch()是阻塞式的，而windowapi这个是非阻塞式的！
		//注意！下面只写’A‘是可以同时识别大小写的，但是
		if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A'))// 按下A键或左方向键
		{
			player.move(-DS, 0);
			player.accumulate_inertia_v(-DS);//只检测输入函数的正负，决定惯性增加的方向。
		}
		if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D'))// 按下D键或右方向键
		{
			player.move(+DS, 0);
			player.accumulate_inertia_v(+DS);
		}
		static int last_shoot = 0, now = 10000;//last_shoot是上一次发射时绘制的时间，now是这一次绘制的时间。
		if ((GetAsyncKeyState(VK_SPACE)|| GetAsyncKeyState(VK_UP)) && now - last_shoot >  40)//按下空格或者上键发射，并且两次发射时间需要大于一定的间隔。
		{
			if (player.status !=dizzy)
			{
				//重置距离上一次发射的帧数为0。（这个帧数用来维持角色姿态）
				player.shot();
				//如果生成子弹成功，重置距离上一次发射的时间为0。（这个时间是用来计算两次成功发射直接的间隔的，判断要不要接受键盘数据）
				if (create_a_bullet())
				{
					last_shoot = now;
				}
			}
		}
		now = GetTickCount();
		//如果没有按下左右方向键
		if (!(GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A')) && !(GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D')))
		{
			player.move(0, 0);
		}
		//每帧都要惯性消减（输入0就是消减）
		player.accumulate_inertia_v(0);
	}
	//这个kbhit的特点是检测缓冲区有无字符，但是并不会读取它！所以你一旦有过键盘输入他返回的一直是1！要调用getch（）不断吸收键盘区的字符！但是这样还会带来卡顿！
	return;
}

///这个是初始化游戏变量的函数，如初始化角色
void initgame()
{
	delete_all_bullets();
	delete_all_lands();
	delete_all_monsters();
	delete_all_strings();
	initglobal_variable();
	//初始化角色的位置，加载角色对象内的图片文件，初始化角色对象内的各各时间变量
	initplayer();
	//初始化一些类当中的静态图片
	//将素材赋给静态变量中的图片
	bulletclass::atlas = atlas;
	landclass::atlas = atlas;
	monster::atlas = atlas;
	rocketclass::atlas = atlas;
	printclass::font = font;
	//初始化所有的地面。即绘制第一帧中的地面
	initrocket();
	initbullets();
	initstrings();
	initlands();
	initmonsters();
}

///这个是自定义的函数，为所有图片全局变量初始化
void loadimg()
{ 
	//loadimage(&atlas2, _T("./shapes/atlas2.png"));
	//loadimage(&player_img, _T("./shapes/playerSheet.png"));
	//loadimage(&player_img2, _T("./shapes/playerSheet2.png"));
	loadimage(&atlas, _T("IMAGE"), _T("atlas1"));
	loadimage(&atlas2, _T("IMAGE"), _T("atlas2"));
	loadimage(&player_img, _T("IMAGE"), _T("playerSheet1"));
	loadimage(&player_img2, _T("IMAGE"), _T("playerSheet3"));
	//loadimage(&player_img2, _T("IMAGE"), _T("playerSheet2"));playerSheet2转移之后得到的文件，这个文件似乎依然不能用，仅仅只是在IMAGE类下面内显示而已，要将之修改为外部文件
	loadimage(&font, _T("IMAGE"),_T("bitmapFont_1"));
	//loadimage(&atlas, _T("./shapes/atlas.png"));
	//loadimage(&font, _T("./shapes/bitmapFont_0.png"));
};

///调用后判断玩家是不是挂了（掉出屏幕），然后再指向对应操作
void if_player_dead()
{
	int base_x;
	int base_y;
	base_x = WINDOWW / 2 - 212;
	base_y = WINDOWH * 0.2 + round(WINDOWH * 0.8 * pow(0.95, dead_time));
	if (dead_time >= 0 )
	{
		dead_time++;
		putpng(base_x,base_y, 424, 150, &atlas, 0, 245);
		myprint("score", base_x, base_y+180);
		myprint(to_string(jump_sum / 10), base_x + 300, base_y +265);
		myprint("highest score", base_x, base_y + 380);
		myprint(to_string(*(scores.begin())), base_x + 300, base_y + 465);
		setbkmode(TRANSPARENT);
		settextcolor(RGB(100, 100, 100)); 
		settextstyle(25, 0, _T("黑体")); // 设置文字大小，字体,第一个参数是文字高度，第二个是平均宽度，设为0则根据高度自适应
		outtextxy(0, WINDOWH-25, instruction); 
	}
	if (player.bottom_y() > WINDOW_BOTTOM)
	{
		//initplayer();//直接重置位置，纯纯开发人员专属无敌版。
		if (dead_time == -1)
		{
			dead_time = 0;
			scores.insert(jump_sum/10);
		}
		if (GetAsyncKeyState(VK_SPACE))
		{
			initgame();
		}
		else if (GetAsyncKeyState(VK_ESCAPE))
		{
			delete_all_bullets();
			delete_all_lands();
			delete_all_monsters();
			delete_all_strings();
			if (RECORD == 1)
			{
				std::ofstream file;
				file.open("scores.txt", std::ios::out);
				for (auto it = scores.begin(); it != scores.end(); it++)
				{
					file << *it << "\n";
				}
				file.close();
			}
			exit(0);//记得用exit要带上括号！不然相当于return
		}
	}
	return;
}

///这个函数的目的是绘制一帧当前的游戏界面
void draw_game_window() 
{
	//绘制UI
	draw_background_UI();
	//玩家对象又运行了一帧，让其时间自增
	++player;
	//检测是否有键盘输入，修改玩家的坐标位置，以及判断是否要发生子弹
	player_control();
	//调整y
	player.adjust_y_by_jumping_status();
	//判断是否有怪物发生碰撞
	contact_monster();
	//判断是否与火箭碰撞。
	contact_rocket();
	//判断是否与弹簧或地面相碰撞。
	on_string_or_land_or_monster();
	//player的高度y坐标只在jumping调用后修改，故y修改后我们开始刷新跳跃总高度
	refresh_jump_sum();
	//更新地面
	refresh_all_elements();
	//分别绘制不同对象
	draw_all_lands(jump_sum);
	draw_all_strings(jump_sum);
	draw_all_bullets(jump_sum);
	draw_all_monsters(jump_sum);
	draw_rocket(jump_sum);
	player.show(jump_sum);
	draw_upper_UI(jump_sum);
	//判断玩家是否死亡
	if_player_dead();
}

int main() 
{
	int mytime = 1;
	int last_frame = 1;
	srand(unsigned(time(0)));
	//initgraph(WINDOWW* XASP,WINDOWH* YASP,SHOWCONSOLE);//这个会显示画布的同时还显示控制台
	initgraph(WINDOWW, WINDOWH);
	//这个函数是启动显示窗口用的，重复调用会关掉已经存在的，再重新启动
	setaspectratio(XASP, YASP);
	//初始化线条颜色
	setlinecolor(RGB(0, 0, 0));
	loadimg();
	start_again:
	initgame();
	//打开一个显示缓冲区，即在遇到EndBatchDraw()之前所有绘制操作绘制在一个缓冲区中
	//这个缓冲区不会被显示，是隐藏在内存中的
	BeginBatchDraw();
	while (1)
	{
		draw_game_window();
		FlushBatchDraw();
		//使用上面这个函数会使得一次性将缓冲区内的图像显示在输出窗口中
		//如果不是事先输出在缓存窗口中会导致显示器中绘制的图像看着是先后渲染的
		mytime = GetTickCount();
		while (mytime-last_frame<12)//这个数字决定帧率上限
		{
			Sleep(0.5);//这里设置成1会使画面卡，设置成0.5
			mytime = GetTickCount();
		}
		last_frame = GetTickCount();
		//每帧间隔17毫秒，使得每秒最多插60帧，但是程序渲染每一帧都要画个几毫秒，所以这里设个小于17的数
	}
	EndBatchDraw();
	//getchar();也可以用来定住画框，但这里有while多余了
	return 0;
}