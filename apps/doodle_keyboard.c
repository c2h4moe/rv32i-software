#include "basictime.h"
#define DOODLE_REGS_BASE (0x300000)
#define M_DOODLE_REGS(n, id, x, y) *((volatile int*)(DOODLE_REGS_BASE + ((n) << 2))) = (((id) << 22) | ((x) << 11) | (y))
typedef enum{
    PLATFORM,
    BREAK_PLATFORM,
    MONSTER,
    BALL,
    TOOL,
    ROLE
} PicElemKind;
typedef enum{
    GREEN_PLATFORM = 1,
    LIGHT_BLUE_PLATFORM
} PlatformKind;
typedef enum {
    BROWN_PLATFORM_BREAKING_1 = 1,
    BROWN_PLATFORM_BREAKING_2,
    BROWN_PLATFORM_BREAKING_3,
    BROWN_PLATFORM_BREAKING_4,
} BreakPlatformKind;
typedef enum {
    PURPLE_MONSTER = 1,
    RED_MONSTER
} MonsterKind;
typedef enum {
    SPRING_COMPRESSED = 1,
    SPRING_FULL,
    PROPELLER,
    PROPELLER_RUNNING
} ToolKind;
typedef enum {
    DOODLE_LEFT = 1,
    DOODLE_RIGHT,
    DOODLE_UP
} RoleKind;
void change_vram(int kind, int n, int id, int x, int y){
    *(volatile int*)(DOODLE_REGS_BASE + (kind << 6) + (n << 2)) = (id << 22) | (x << 11) | y;
}
int main(){
    change_vram(BREAK_PLATFORM, 0, BROWN_PLATFORM_BREAKING_1, 0, 0);
    change_vram(BREAK_PLATFORM, 1, BROWN_PLATFORM_BREAKING_2, 0, 50);
    change_vram(BREAK_PLATFORM, 3, BROWN_PLATFORM_BREAKING_2, 0, 80);
    change_vram(BREAK_PLATFORM, 2, BROWN_PLATFORM_BREAKING_2, 100, 50);
    change_vram(PLATFORM, 0, LIGHT_BLUE_PLATFORM, 30, 0);
    change_vram(PLATFORM, 1, LIGHT_BLUE_PLATFORM, 30, 40);
    change_vram(PLATFORM, 2, GREEN_PLATFORM, 30, 80);
    change_vram(PLATFORM, 3, GREEN_PLATFORM, 30, 110);
    change_vram(PLATFORM, 4, GREEN_PLATFORM, 30, 140);
    change_vram(PLATFORM, 5, LIGHT_BLUE_PLATFORM, 30, 200);
    change_vram(PLATFORM, 6, GREEN_PLATFORM, 400, 0);
    change_vram(PLATFORM, 7, GREEN_PLATFORM, 400, 80);
    change_vram(PLATFORM, 8, LIGHT_BLUE_PLATFORM, 400, 160);
    change_vram(PLATFORM, 9, GREEN_PLATFORM, 400, 200);
    change_vram(PLATFORM, 10, GREEN_PLATFORM, 500, 0);
    change_vram(PLATFORM, 11, GREEN_PLATFORM, 500, 80);
    change_vram(PLATFORM, 12, LIGHT_BLUE_PLATFORM, 500, 160);
    change_vram(PLATFORM, 14, GREEN_PLATFORM, 500, 200);
    change_vram(PLATFORM, 13, GREEN_PLATFORM, 100, 500);
    change_vram(PLATFORM, 15, GREEN_PLATFORM, 800, 200);
    change_vram(MONSTER, 0, RED_MONSTER, 40, 80);
    change_vram(MONSTER, 1, PURPLE_MONSTER, 40, 300);
    change_vram(TOOL, 0, SPRING_FULL, 30, 180);
    change_vram(TOOL, 1, SPRING_COMPRESSED, 500, 50);
    change_vram(TOOL, 2, PROPELLER, 400, 100);
    change_vram(TOOL, 3, PROPELLER_RUNNING, 400, 400);
    change_vram(BALL, 3, 1, 100, 100);
    change_vram(BALL, 0, 1, 500, 500);
    change_vram(BALL, 1, 1, 400, 400);
    change_vram(BALL, 2, 1, 300, 300);
    int i = 500;
    change_vram(ROLE, 0, DOODLE_LEFT, 120, i);

    int upstate = 1;
    int clk = 0;
    while(1) {
    }
    asm("ebreak");
}