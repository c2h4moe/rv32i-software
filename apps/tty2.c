#include "basicgui.h"
#define MODE_CONTROL_ADDR (0x00400000)

int main()
{
    unsigned char *p = (unsigned char *)MODE_CONTROL_ADDR;
    *p = 1;
    color_rgb c = common_colors[BLUE];
    DrawStraightLine(5, 5, 75, 5, TransferColor(c));
    DrawStraightLine(5, 5, 5, 55, TransferColor(c));
    DrawStraightLine(75, 5, 75, 55, TransferColor(c));
    DrawStraightLine(5, 55, 75, 55, TransferColor(c));
    // main need a break or else PC will keep increasing
    asm("ebreak");
}