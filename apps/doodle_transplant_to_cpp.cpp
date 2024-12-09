extern "C"{
	#include "basicio.h"
	#include "math.c"
	#include "debug.h"
}


#define headline 408  //中线高度
#define DS 1		  //每帧下降的速度
#define LANDNUM 16    //不是一个界面中的地面数量，后台绘制游戏窗口和窗口上方一个窗口高度的地面
#define STRINGNUM 4   //后台一共有多少个弹簧
#define WINDOW_BOTTOM ( WINDOWH - jump_sum )
#define WINDOW_TOP (-jump_sum)
#define LANDS_SPAN (2 * WINDOWH)                      //后台所有的地面所在的一个范围内，绝对值，正数。
#define LANDS_SPAN_BOTTOM (WINDOW_BOTTOM + 205)       //后台所有的地面从哪里开始回收 205==WIDOWH/5
#define INTERVAL_LAND (LANDS_SPAN / LANDNUM)          //每一个地面的间隔

#define WINDOWH 1024
#define WINDOWW 1280

#define DOODLE_REGS_BASE (0x300000)
#define KEYBOARD_ADDR (0xbad00000)

#define FALSE 0
#define TRUE 1

#define time_for_a_jump 80      //使用多少帧完成一次完整跳跃
#define V 80                    //普通起跳初速度，四分之一的总用帧乘以V即一次起跳最大上升高度/像素
#define STRING_V 150             //弹簧起跳的初速度
#define JUMP_HEIGHT (V * 20)           //一次起跳最大上升高度/像素
#define BLUELAND_DS 7           					//蓝色砖块的最大移动速度,别小于2！
#define FRAGILELAND_DS 8        					//易碎砖块的下降速度
#define FLYING_T 200

#define BALL_W 15
#define BALL_H 15
#define BROWN_PLATFORM_BREAKING_1_W 90
#define BROWN_PLATFORM_BREAKING_1_H 23
#define BROWN_PLATFORM_BREAKING_2_W 90
#define BROWN_PLATFORM_BREAKING_2_H 42
#define DOODLE_W 100
#define DOODLE_H 90
#define DOODLE_UP_W 66
#define DOODLE_UP_H 111
#define GREEN_PLATFORM_W 90
#define GREEN_PLATFORM_H 23
#define BLUE_PLATFORM_W 90
#define BLUE_PLATFORM_H 23
#define PURPLE_MONSTER_W 82
#define PURPLE_MONSTER_H 73
#define RED_MONSTER_W 75
#define RED_MONSTER_H 59
#define SPRING_COMPRESSED_W 28
#define SPRING_COMPRESSED_H 24
#define SPRING_FULL_W 25
#define SPRING_FULL_H 36
#define PROPELLER_W 40
#define PROPELLER_H 27
#define PROPELLER_RUNNING_W 40
#define PROPELLER_RUNNING_H 31

static int cur = 114514;
static const int A = 1103515245; // parameter from glibc
static const int C = 12345; // parameter from glibc

int rand(){
    cur = (A * cur + C) & 0x7fffffff;
    return cur;
}

void seed(int n){
    cur = n;
}

//玩家状态
enum PLAYER_STATUS
{
	RIGHT, 
    LEFT, 
};
//地面类型
enum LAND_TYPE
{
    INVALID,
    GREENLAND,
    BLUELAND,
    FRAGILELAND
};
//物品类型 决定寄存器摆放位置
enum PicElemKind
{
    PLATFORM,
    BREAK_PLATFORM,
    MONSTER,
    BALL,
    TOOL,
    ROLE
};
//图片编号
enum PlatformKind
{
    GREEN_PLATFORM = 1,
    BLUE_PLATFORM
};
enum BreakPlatformKind
{
	NOT_BREAK = 1,
	BREAK
};
enum MonsterKind
{
    PURPLE_MONSTER = 1,
    RED_MONSTER
};
enum ToolKind
{
    SPRING_COMPRESSED = 1,
    SPRING_FULL,
    PROPELLER,
    PROPELLER_RUNNING
};
enum RoleKind
{
    DOODLE_LEFT = 1,
    DOODLE_RIGHT,
    DOODLE_UP
};

//玩家类
struct playerclass
{
	//设置玩家的新x，y坐标，以及人物状态
	void set(int nx, int ny, PLAYER_STATUS ns);
	//增减玩家x，y坐标的函数
	void move(int dx, int dy);
	//在图中绘制一次角色。
	void show(int sum = 0);
	//每调用一次为所有时间变量加一，即时间增加一帧
	playerclass& operator++();
	//修改玩家的起跳状态，重置起跳位置为当前值，重置起跳时间，不修改y值
    //跳跃的力度，如果0那就不跳跃，随重力下落，为1则普通跳跃，为2为弹簧跳
	void adjust_jumping_status(int jump_strength = 0);
	///每帧根据状态调整高度。
	void adjust_y_by_jumping_status();

	int pos_x;
	int pos_y;
	//上一次的起跳情况，1是普通跳，2是弹簧跳，3是正在飞行！
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

	PLAYER_STATUS status;
	PLAYER_STATUS status_before_falling;
	int flying_t = -1;	//这个是时间变量，记录开始飞行了多少帧。如果是-1说明没有起飞。
	int t;				//这个是时间变量，用于记录当前距离上一次起跳经过了多少帧
};

//基础地面类
struct landclass
{
	int pos_x = 0;
	int pos_y = 0;
	bool live = FALSE;
    LAND_TYPE type = INVALID;
    //显示地面
    void show(int sum = 0, int index = 0);
    //判断是否与玩家发生碰撞
    bool is_contact(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
    //当判断出发生碰撞时调用
    void contact();
    // greenlandclass();
    // 无其他变量
    // bluelandclass();
    // 每个蓝砖生成时产生一个方向变量 1或者-1
	int direction = __mulsi3(2, (__modsi3(rand(), 2))) - 1;
	int speed = __modsi3(rand(), BLUELAND_DS-1) + 2;
    // fragilelandclass();
	bool broken = FALSE;
	int broken_t = 0; //距离破碎的时间
};

//弹簧类
struct stringlandclass
{
    void show(int sum = 0, int index = 0);
	///当判断出发生碰撞时调用。
	void contact();
	///这个函数返回是否与地面发生碰撞。
	bool is_contact(int last_t_bottom_y, int player_bottom_x, int player_bottom_y);
	int pos_x = 0;
	int pos_y = 0;
	bool live = FALSE;
    // -------------------------------------------------------------------
	bool triggerd = FALSE;//是否已经被触发
	landclass* base = nullptr;
	int relative_x = 0;
};

///火箭道具,由于他是基于玩家类和地面类的
struct rocketclass
{
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
// kind对应enum的物品类型 n对应在数组中的序号 id对应这类型中的图片编号 x y对应坐标
void change_vram(int kind, int n, int id, int x, int y)
{
    *(volatile int*)(DOODLE_REGS_BASE + (kind << 6) + (n << 2)) = (id << 22) | (x << 11) | y;
}
//每调用一次为所有时间变量加一，即时间增加一帧
playerclass& playerclass::operator++()
{
	t++;
	if (flying_t >= 0)
	{
		flying_t++;
	}
	return *this;
}
//bottom_x是判断玩家是否与地面接触的底部x点坐标
int playerclass::bottom_x()
{
	return pos_x + 52;
}
//bottom_y是判断玩家是否与地面接触的底部y点坐标
int playerclass::bottom_y()
{
	return pos_y + 116;
}
//contact_x是判断玩家是否与物品接触的x点坐标
int playerclass::contact_x()
{
	return pos_x + 65;
}
//contact_y是判断玩家是否与物品接触的y点坐标
int playerclass::contact_y()
{
	return pos_y + 60;
}
//初始设置设置玩家的新x，y坐标，以及人物状态
void playerclass::set(int nx, int ny, PLAYER_STATUS ns)
{
	pos_x = nx;
	pos_y = ny;
	status = ns;
	jump_from_y = pos_y;
	return;
}
//每帧根据状态调整高度
//是根据上一其他位置来修改pos_y的！跟所处的pos_y无关！
void playerclass::adjust_y_by_jumping_status()
{
	int ds;
	//根据上一次的起跳情况调节高度变化
	//这里有个坑！坐标要减去跳跃量!画布上的是y反的！
	switch (jump_status)
	{
	case 1:
		pos_y = jump_from_y - (V * t - t * t);
		break;
	case 2:
		pos_y = jump_from_y - (STRING_V * t - t * t);
		break;
	case 3:
		ds = (FLYING_T - flying_t) / 8;
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
//起跳函数，每次调用根据t和是否输入true来修改玩家当前的y值，注意！
void playerclass::adjust_jumping_status(int jump_strength)
{
	if (jump_strength==1)
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
//增减玩家x，y坐标的函数
void playerclass::move(int dx, int dy) 
{
	if (dx < 0)
		status = LEFT;
	else if (dx > 0)
		status = RIGHT;
	pos_x += dx;
	pos_y += dy;
	//如果进入了屏幕两侧要可以从另外一侧出现
	if (pos_x < -80)
	{
		pos_x += WINDOWW + 60;
		return;
	}
	if (pos_x > WINDOWW - 20)
	{
		pos_x -= WINDOWW + 60;
		return;
	}
	return;
}
//在图中绘制一次角色
void playerclass::show(int sum)
{
	//这个的目的是，如果t<10说明刚刚起跳,应该显示起跳动作，
	//正好起跳状态是正常状态的下一位，所以这里加一是把状态转换为当前状态对应的起跳状态
	switch (status)
	{
	case RIGHT:
		change_vram(ROLE, 0, DOODLE_RIGHT, pos_x, pos_y + sum);
		break;
	case LEFT:
		change_vram(ROLE, 0, DOODLE_LEFT, pos_x, pos_y + sum);
		break;
	default:
		break;
	}
	return;
}
//这个函数返回是否与地面发生碰撞。
bool landclass::is_contact(int last_t_bottom_y,int player_bottom_x,int player_bottom_y)
{
	//-20,+130 为了让玩家摸到一点砖也能跳 参数需要修改的
	if ((last_t_bottom_y < pos_y + 2) && (player_bottom_y >= pos_y + 2)
		&& (player_bottom_x >= pos_x - 20) && (player_bottom_x <= pos_x + 130))
	{
		return TRUE;
	}
	return FALSE;
}
//显示地面
void landclass::show(int sum, int index)
{
    if (type == GREENLAND)
    {
		change_vram(PLATFORM, index, GREEN_PLATFORM, pos_x, pos_y + sum);
    }
    else if (type == BLUELAND)
    {	
		if (direction == 1)
			pos_x += speed;
		else
			pos_x -= speed;
        //如果砖块撞到了屏幕两侧 则调换方向
        if (pos_x < 0 || pos_x > WINDOWW - BLUE_PLATFORM_W)
            direction = -direction;
		change_vram(PLATFORM, index, BLUE_PLATFORM, pos_x, pos_y + sum);
    }
    else if (type == FRAGILELAND)
    {
        //如果没坏
        if (!broken)
        {
			change_vram(BREAK_PLATFORM, index, NOT_BREAK, pos_x, pos_y + sum);
        }
        //如果坏了
        else
        {
			change_vram(BREAK_PLATFORM, index, BREAK, pos_x, pos_y + sum);
            pos_y += FRAGILELAND_DS;
            broken_t++;
        }
    }
	return;
}
///当脆弱砖块发生碰撞时
void landclass::contact()
{
    if(type == FRAGILELAND)
        broken = TRUE;
	return;
}
///显示弹簧
void stringlandclass::show(int sum, int index)
{
	//跟随基地面移动
	pos_x = base->pos_x + 18 + relative_x;
	pos_y = base->pos_y - 20;
	//如果弹簧被触发了
	if (triggerd)
	{
		change_vram(TOOL, index, SPRING_COMPRESSED, pos_x, pos_y + sum);
	}
	else
	{
		change_vram(TOOL, index, SPRING_FULL, pos_x, pos_y + sum);
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
	if ((last_t_bottom_y < pos_y) && (player_bottom_y >= pos_y)
		&& (player_bottom_x >= pos_x - 28) && (player_bottom_x <= pos_x + 43))
	{
		return TRUE;
	}
	return FALSE;
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
		change_vram(TOOL, 0, PROPELLER_RUNNING, pos_x, pos_y + sum);
		return;
	}
	if (triggerd_t > 0)
	{
		if (falling_t < 0)
		{
			base_y = pos_y;
			base_x = pos_x;
			falling_t = 0;
		}
		pos_y = base_y - (__mulsi3(8, falling_t) - __mulsi3(falling_t, falling_t));
		pos_x = pos_x + 3 * ((base_x % 2) * 2 - 1);
		change_vram(TOOL, 0, PROPELLER_RUNNING, pos_x, pos_y + sum);
		falling_t++;
		return;
	}
	//跟随基地面移动
	pos_x = base->pos_x + 21 + 10;
	pos_y = base->pos_y - 70;
	change_vram(TOOL, 1, PROPELLER, pos_x, pos_y + sum);
	return;
}

bool rocketclass::is_contact(int x, int y)
{
	if (x >= pos_x - 20 && x < pos_x + 50 + 20 && y > pos_y-40 && y < pos_y + 73 + 20)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//-------------------------------------------------------------------------------
// jump_sum 累计超过中线的高度,是一个正数
int jump_sum;
// 声明实体 
rocketclass rocket;
playerclass player;
stringlandclass strings[STRINGNUM];
landclass lands[LANDNUM];
int the_top_land_index;
int the_highest_solid_land_index;   //最后一个实体地面，绿色蓝色（不带弹簧的）地面的index，要保证任何两个实体方块间距离小于玩家可以跳的最大距离。
int the_bottom_land_index;
int last_t_bottom_y;    //这个静态变量用于储存上一帧玩家的底部高度坐标，如果上一帧到下一帧穿过了地面的平面则视为发生了碰撞。 
int dead_time;

void contact_rocket()
{
	if (rocket.live == FALSE) return;
	//判断玩家是否摸到了火箭，注意确保玩家没有晕，注意防止反复碰撞！
	if (player.flying_t < 0 && rocket.triggerd_t < 0 &&
        rocket.is_contact(player.contact_x(), player.contact_y()))
	{
		rocket.base_player = &player;
		rocket.live == FALSE;
		player.adjust_jumping_status(3);
	}
	return;
}
//查找是否与任何一个地面或弹簧相碰撞
void on_string()
{
	//扫描弹簧
	for (int i = 0; i < STRINGNUM; i++)
	{
		if (strings[i].live == FALSE)
			continue;
		if ((player.t >= time_for_a_jump / 2) && 
            strings[i].is_contact(last_t_bottom_y, player.bottom_x(), player.bottom_y()))
		{
            //因为有可能上一帧在地面上，但是下一帧已经在地面下了，不能从地面下其他，要将人物移动到地面上
			player.pos_y += (strings[i].pos_y - player.bottom_y());
			player.adjust_jumping_status(2);
			strings[i].contact();
			break;
		}
	}
	return;
}
//查找是否与任何一个地面相碰撞
void on_land()
{
	//扫描地面
	for (int i = 0; i < LANDNUM; i++)
	{
		//如果判断这个地面为基类地面，那么不需要进入碰撞判断
		if (lands[i].live == FALSE || lands[i].type == INVALID)
			continue;
		//player.t >= time_for_a_jump / 2 的意思是玩家正在下落
		if ((player.t >= time_for_a_jump / 2) && 
            lands[i].is_contact(last_t_bottom_y, player.bottom_x(), player.bottom_y()))
		{
			if (lands[i].type == GREENLAND)
			{
                //因为有可能上一帧在地面上，但是下一帧已经在地面下了
                //不能从地面下其他，要将人物移动到地面
				player.pos_y += (lands[i].pos_y - player.bottom_y());
				player.adjust_jumping_status(1);
				break;
			}
			if (lands[i].type == BLUELAND)
            {
                player.pos_y += (lands[i].pos_y - player.bottom_y());
                player.adjust_jumping_status(1);
                break;
            }
			// 易碎地面
			if (lands[i].type == FRAGILELAND)
            {
                //不起跳!
                lands[i].contact();
                break;
            }
		}
	}
	return;
}
//整合两个函数，查找是否与任何一个地面或者弹簧相碰撞,任何刷新上一帧的玩家底部位置。
void on_string_or_land()
{
	on_string();
	on_land();
	last_t_bottom_y = player.bottom_y();
}
//为一个地面生成一个弹簧。
void create_a_string(landclass* base_land)
{
	//先找出一个可以生成的空位。
	for (int i = 0; i < STRINGNUM; i++)
	{
		if (strings[i].live == FALSE)
		{
			strings[i].live = TRUE;
			strings[i].triggerd = FALSE;
			strings[i].base = base_land;
			strings[i].relative_x = __modsi3(rand(),58);
			strings[i].pos_x = base_land->pos_x + 18 + strings[i].relative_x;
			strings[i].pos_y = base_land->pos_y - 20;
			return;
		}
	}
	return;
}
//尝试生成火箭。
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
//初始化角色数据。
void initplayer()
{
	player.set(WINDOWW/5, ((WINDOWH*4)/5-jump_sum), RIGHT);
	player.adjust_jumping_status(1);
	player.t = 0;
	return;
}
//绘制初始界面的所有地面
void initlands()
{
	int seed = 0;
	int land_x, land_y;
	for (int i = 0; i < LANDNUM; i++)
	{
		seed = rand() % 3000;
		///land_x指当前这一块地随机生成的x坐标，505=620-115地图宽减去地砖长。
		land_x = seed % (WINDOWW - 115);
		///land_y指当前这一块地随机生成的y坐标，初始化时地图高度分成地面数份逐份向上生成。
		land_y = LANDS_SPAN_BOTTOM -  i * INTERVAL_LAND;
		///生成绿色地面。
        lands[i].live = TRUE;
		lands[i].pos_x = land_x;
		lands[i].pos_y = land_y;
		if (seed < 2000)
		{
			lands[i].type = GREENLAND;
			if (seed % 5 == 1 && i > 4)
			{
				create_a_string(&lands[i]);
			}
			else
			{
                //如果不创建弹簧的话他就是最高的那一个实体地面
				//我们来判断是否存在卡关现象，如果当前这个实体方块离现有（也就是上一个）
                //的最高的实体方块超过了玩家可以起跳的最高高度
				//需要为上一个最高的实体方块创建弹簧，防止出现卡关
                //没有超过玩家起跳高度就无所谓
				if (lands[the_highest_solid_land_index].pos_y - land_y > JUMP_HEIGHT)
				{
					create_a_string(&lands[the_highest_solid_land_index]);
				}
				//继续将之重置为上一个最高的实体方块
				the_highest_solid_land_index = i;
			}
			continue;
		}
		///生成脆弱地面
		else if (seed < 2500)
		{
			lands[i].type = FRAGILELAND;
			continue;
		}
		///生成空地面（地面基类）
		else
		{
			lands[i].type = INVALID;
			continue;
		}
	}
	//将最后生成的那一个定义为目前最高的地面
	the_top_land_index = LANDNUM - 1;
	return;
}
//初始化所有的全局变量
void initglobal_variable()
{
	jump_sum = 0;
	the_top_land_index = 0;
	the_highest_solid_land_index = 0;
	the_bottom_land_index = 0;
	last_t_bottom_y = 0;
	dead_time = -1;
	return;
}
//初始化所有弹簧
void initstrings()
{
	for (int i = 0; i < STRINGNUM; i++)
		strings[i].live = FALSE;
	return;
}
//初始化火箭
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
//一局结束后删除所有的弹簧
void delete_all_strings()
{
	for (int i = 0; i < STRINGNUM; i++)
        strings[i].live = FALSE;
	return;
}
//一局结束后删掉所有的地面
void delete_all_lands()
{
	for (int i = 0; i < LANDNUM; i++)
        lands[i].live = FALSE;
	return;
}
//更新当前所有的地图元素。即将已经在屏幕下方（live=FALSE）的地图元素，重新在上方生成
void refresh_all_elements()
{
	int seed = 0;
	int land_x, land_y;
	// srand(unsigned(time(0)));//这个函数有一个巨坑！time返回的是秒！
    // 但是你是每帧都调用一次srand！所以一秒内每一帧接收的种子都一样！一秒内生成的随机数都一样！
	while (lands[the_top_land_index].pos_y > LANDS_SPAN_BOTTOM - LANDS_SPAN)
	{
		//如果你重复调用srand，一秒内的rand产生的第一个随机数都一样，下一秒rand产生的第一个
		//数会只比上一秒产生的稍微大几而已，余了1000之后，导致seed几乎没有很大变化！
		seed = __modsi3(rand(), 3000);
		//land_x指当前这一块地随机生成的x坐标
		land_x = __modsi3(seed, (WINDOWW - 115));
		//land_y指当前这一块地随机生成的y坐标，初始化时地图高度分成地面数份逐份向上生成
		land_y = lands[the_top_land_index].pos_y - INTERVAL_LAND;
		the_top_land_index = the_bottom_land_index;
        // 重新赋值
        lands[the_bottom_land_index].live = TRUE;
		lands[the_bottom_land_index].pos_x = land_x;
		lands[the_bottom_land_index].pos_y = land_y;
		//生成绿色地面
		if (__modsi3(seed, 100) < 50)
		{
			lands[the_bottom_land_index].type = GREENLAND;
			if (__modsi3(seed, 10) == 0)
			{
				create_a_string(&lands[the_bottom_land_index]);
			}
			else
			{
                //如果不创建弹簧的话他就是最高的那一个实体地面
				//我们来判断是否存在卡关现象，如果当前这个实体方块离现有（也就是上一个）
                //的最高的实体方块超过了玩家可以起跳的最高高度，
				//需要为上一个最高的实体方块创建弹簧，防止出现卡关
				if (lands[the_highest_solid_land_index].pos_y - land_y > JUMP_HEIGHT)
				{
					create_a_string(&lands[the_highest_solid_land_index]);
				}
				else
				{
                    //如果没有超过玩家起跳高度那么说明上一个最高的实体方块肯定没有弹簧，可以生成火箭。
					if (__modsi3(seed, 10) < 2) 
                        create_rocket(&lands[the_highest_solid_land_index]);
				}
				//继续将之重置为上一个最高的实体方块。
				the_highest_solid_land_index = the_bottom_land_index;
			}
			the_bottom_land_index = __modsi3((++the_bottom_land_index), LANDNUM);
			continue;
		}
		//生成蓝色地面
		if (__modsi3(seed, 100) < 75)
		{
			lands[the_bottom_land_index].type = BLUELAND;
			if (__modsi3(seed, 5) == 1)
			{
				create_a_string(&lands[the_bottom_land_index]);
			}
			else
			{
                //如果不创建弹簧的话他就是最高的那一个实体地面
				//我们来判断是否存在卡关现象，如果当前这个实体方块离现有（也就是上一个）
                //的最高的实体方块超过了玩家可以起跳的最高高度，
				//需要为上一个最高的实体方块创建弹簧，防止出现卡关
				if (lands[the_highest_solid_land_index].pos_y - land_y > JUMP_HEIGHT)
				{
					create_a_string(&lands[the_highest_solid_land_index]);
				}
				else
				{
                    //如果没有超过玩家起跳高度那么说明上一个最高的实体方块肯定没有弹簧，可以生成火箭
					if (__modsi3(seed, 10) < 2)
						create_rocket(&lands[the_highest_solid_land_index]);
				}
				//继续将之重置为上一个最高的实体方块
				the_highest_solid_land_index = the_bottom_land_index;
			}
			the_bottom_land_index = __modsi3((++the_bottom_land_index), LANDNUM);
			continue;
		}
		//生成易碎地面
		if (__modsi3(seed, 100) < 100)
		{
			lands[the_bottom_land_index].type = FRAGILELAND;
			the_bottom_land_index = __modsi3((++the_bottom_land_index), LANDNUM);
			continue;
		}
	}
	return;
}
//更新当前的跳跃总高度
void refresh_jump_sum()
{
	if (player.pos_y < headline)
	{
		//jump_sum记为玩家y坐标小于400的最大差值
		int temp = headline - player.pos_y;
		if (temp > jump_sum)
			jump_sum = temp;
	}
	return;
}
//绘制所有的弹簧，并且将在地图下的标记为FALSE
void draw_all_strings(int sum)
{
	for (int i = 0; i < STRINGNUM; i++)
	{
		//别对还没生成的弹簧操作
		if (strings[i].live == FALSE)
			continue;
        //已经离开窗口的一定不能显示！仍在显示可能导致地面已经被回收弹簧仍在读取地面地面数据！
		if (strings[i].pos_y > WINDOW_BOTTOM)
			strings[i].live = FALSE;
		if (strings[i].live == TRUE)
			strings[i].show(jump_sum, i);
	}
	return;
}
//绘制当前界面所有的地面。并且将在地图下的标记为FALSE
void draw_all_lands(int sum)
{
	//显示在窗口范围内的地砖live = TRUE,
	for (int i = 0; i < LANDNUM; i++)
	{
		//如果地砖已经在地图底下的话那就关掉live，不显示了，也不判断碰撞
		if(lands[i].pos_y > WINDOW_BOTTOM)
		{
			lands[i].live = FALSE;
		}
		//只输出所有TRUE，即应该显示的砖块
		if (lands[i].live == TRUE)
		{
			lands[i].show(jump_sum, i);
		}
	}
	return;
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
//用于读取用户的键盘与鼠标输入。还有子弹的生成也在其中。
void player_control()
{		
	// 监测键盘的输入
	char ch = getchark();
	if (ch == 'w' || ch == 's' || ch == 'a' || ch == 'd' ||
		ch == 0x1 || ch == 0x2 || ch == 0x3 || ch == 0x4)
	{
		if (ch == 'a' || ch == 0x3)
		{
			player.move(-DS, 0);
		}
		if (ch == 'd' || ch == 0x4)
		{
			player.move(+DS, 0);
		}
		//如果没有按下左右方向键
		if (!(ch == 'a' || ch == 'd'))	
			player.move(0, 0);
	}
	return;
}
//这个是初始化游戏变量的函数，如初始化角色
void initgame()
{
	delete_all_lands();
	delete_all_strings();
	initglobal_variable();
	//初始化角色的位置，加载角色对象内的图片文件，初始化角色对象内的各各时间变量
	initplayer();
	//初始化所有的地面。即绘制第一帧中的地面
	initrocket();
	initstrings();
	initlands();
}
//调用后判断玩家是不是挂了（掉出屏幕），然后再指向对应操作
void if_player_dead()
{
	int base_x;
	int base_y;
	base_x = WINDOWW / 2 - 212;
	base_y = WINDOWH;

	char ch = getchark();
	if (dead_time >= 0 )
	{
		dead_time++;
	}
	if (player.bottom_y() > WINDOW_BOTTOM)
	{
		//直接重置位置，纯纯开发人员专属无敌版。
		//initplayer();
		if (dead_time == -1)
		{
			dead_time = 0;
		}
		if (ch == ' ')
		{
			initgame();
		}
		else if (ch == '\n')
		{
			delete_all_lands();
			delete_all_strings();
			asm("ebreak");
		}
	}
	return;
}

int main() 
{
	seed(114514);
	initgame();
	while (1)
	{
		//分别绘制不同对象
		draw_all_lands(jump_sum);
		draw_all_strings(jump_sum);
		draw_rocket(jump_sum);
		player.show(jump_sum);
		//玩家对象又运行了一帧，让其时间自增
		++player;
		//检测是否有键盘输入，修改玩家的坐标位置，以及判断是否要发生子弹
		player_control();
		//调整y
		player.adjust_y_by_jumping_status();
		//判断是否与火箭碰撞。
		contact_rocket();
		//判断是否与弹簧或地面相碰撞。
		on_string_or_land();
		//player的高度y坐标只在jumping调用后修改，故y修改后我们开始刷新跳跃总高度
		refresh_jump_sum();
		//更新地面
		refresh_all_elements();
		//判断玩家是否死亡
		if_player_dead();
	}
	return 0;
}