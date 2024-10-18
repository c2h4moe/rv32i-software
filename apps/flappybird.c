#include <basicio.h>
#include "basictime.h"

#define KEYBOARD_ADDR (0xbad00000)
#define GUI_MODE_BASE_ADDR (0x00300000)
#define MODE_CONTROL_ADDR (0x00400000)

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60
#define VMEM_WIDTH 128
#define VMEM_HEIGHT 60

#define STDOUT_BUFFER_BASE (0x100000)
#define TTY_SCAN_BASELINE (0x200000)
#define STDOUT_LINE_SIZE (80)

int g_MapHight, g_MapWidth; // 地图的高度和宽度
int g_BridX, g_BridY;		// 鸟的xy坐标
int index;					// 指向最前面的障碍
int barrier_num;
int start_flag;
int g_ThroughX[2], g_ThroughY[2]; // 通道的两个基准中心坐标
int g_Result = 0;				  // 1---游戏结束     0---游戏未结束
int g_Score;					  // 分数成绩
int Book[2][61] = {0};			  // 标记地图上的位置是否有障碍物

typedef unsigned short color_t;
typedef struct color_rgb
{
	int r;
	int g;
	int b;
} color_rgb;

enum color_enum
{
	RED,
	GREEN,
	BLUE,
	YELLOW,
	CYAN,
	MAGENTA,
	WHITE,
	BLACK
};

color_rgb common_colors[] = {
	{0xf, 0, 0},	 // red
	{0, 0xf, 0},	 // green
	{0, 0, 0xf},	 // blue
	{0xf, 0xf, 0},	 // yellow
	{0, 0xf, 0xf},	 // cyan
	{0xf, 0, 0xf},	 // megenta
	{0xf, 0xf, 0xf}, // white
	{0, 0, 0}};		 // black

color_t TransferColor(color_rgb c)
{
	return ((c.r & 0xf) << 8) | ((c.g & 0xf) << 4) | ((c.b & 0xf));
}

void DrawPoint(int x, int y, color_t color)
{
	if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
	{
		unsigned short *p = (unsigned short *)(GUI_MODE_BASE_ADDR + (x + y * VMEM_WIDTH) * 2);
		*p = color;
	}
}

color_t yellow = 0x0ff0;
color_t black = 0x0;
color_t red = 0x0f00;

//.初始化最基础的数据
void StartUp();
//.初始化地图等
void StartMap();
//.刷新游戏的界面
void UpDateOutPut();
//.监测键盘的输入
void UpdateInPut();

int main()
{
	unsigned char *p = (unsigned char *)MODE_CONTROL_ADDR;
	*p = 1;

	int cnt = 0;
	StartUp(); //.初始化最基础的数据

	while (1)
	{
		cnt++;
		StartMap();		//.初始化地图等
		UpdateInPut();	//.监测键盘的输入
		UpDateOutPut(); //.刷新游戏的界面
		if (g_Result == 1)
		{
			break;
		}
		sleep(200);
	}

	asm("ebreak");
}

//.初始化最基础的数据
void StartUp()
{
	g_Score = 0;
	g_MapHight = 60;
	g_MapWidth = 80;
	g_BridX = g_MapWidth / 4;
	g_BridY = g_MapHight / 2;
	index = 0;
	g_ThroughX[index] = g_MapWidth / 4 * 3;
	g_ThroughY[index] = g_MapHight / 2;
	start_flag = 0;
	barrier_num = 2;

	// 初始化上下围墙
	for (int j = 0; j < g_MapWidth; j++)
	{
		DrawPoint(j, 0, yellow);
		DrawPoint(j, g_MapHight - 1, yellow);
	}
}

// 初始化地图
// 1---触碰死亡   0---触碰未死亡
void StartMap()
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 61; j++)
		{
			Book[i][j] = 0;
		}
	}
	// 初始化障碍墙
	// 初始化一个通道出来
	if (start_flag == 0)
	{
		barrier_num = 1;
	}
	for (int j = 0; j < barrier_num; j++)
	{
		for (int i = 1; i <= g_ThroughY[j] - 5; i++)
		{
			DrawPoint(g_ThroughX[j], i, yellow);
			DrawPoint(g_ThroughX[j] + 1, i, yellow);
			DrawPoint(g_ThroughX[j] + 2, i, black);
			Book[j][i] = 1;
		}

		for (int i = g_ThroughY[j] + 6; i < g_MapHight - 1; i++)
		{
			DrawPoint(g_ThroughX[j], i, yellow);
			DrawPoint(g_ThroughX[j] + 1, i, yellow);
			DrawPoint(g_ThroughX[j] + 2, i, black);
			Book[j][i] = 1;
		}
	}
	if (start_flag == 0)
	{
		barrier_num = 2;
	}
	start_flag = 1;
}

//.刷新游戏的界面
void UpDateOutPut()
{
	g_BridY++;
	DrawPoint(g_BridX, g_BridY, yellow);
	DrawPoint(g_BridX, g_BridY - 1, black);

	if (start_flag == 1)
	{
		for (int i = 0; i < barrier_num; i++)
		{
			g_ThroughX[i]--;
		}
	}
	else
	{
		g_ThroughX[0]--;
	}

	// 鸟触碰死亡
	if (g_BridY <= 0 || g_BridY >= g_MapHight - 1)
	{
		g_Result = 1;
	}
	if (Book[index][g_BridY] == 1 && g_BridX == g_ThroughX[index])
	{
		g_Result = 1;
	}

	// 鸟跨过障碍物，得分
	if (g_BridX == g_ThroughX[index] + 1)
	{
		g_Score++;
		index = (index + 1) % barrier_num;
	}

	if (g_ThroughX[index] < g_MapWidth / 4)
	{
		g_ThroughY[index] = 12 % g_MapHight;
		g_ThroughX[index] = g_MapWidth / 4 * 3;
	}

	for (int i = 1; i < g_MapHight - 1; i++)
	{
		DrawPoint(0, i, black);
	}
}

// 监测键盘的输入
void UpdateInPut()
{
	char ch = getchark();
	if (ch == ' ' || ch == 'w')
	{
		DrawPoint(g_BridX, g_BridY, black);
		DrawPoint(g_BridX, g_BridY - 2, yellow);
		g_BridY -= 2;
	}
}
