#include "basicgui.h"
#define MODE_CONTROL_ADDR (0x00400000)

int main()
{
    unsigned char *p = (unsigned char *)MODE_CONTROL_ADDR;
    *p = 1;
    color_t blue = TransferColor(common_colors[BLUE]);
    color_t yellow = TransferColor(common_colors[YELLOW]);
    color_t black = TransferColor(common_colors[BLACK]);

    DrawStraightLine(5, 5, 75, 5, blue);
    DrawStraightLine(5, 5, 5, 55, blue);
    DrawStraightLine(75, 5, 75, 55, blue);
    DrawStraightLine(5, 55, 75, 55, blue);

    int x0 = 10, y0 = 10, x1 = 65, y1 = 45;
    int tempx = x0;
    int prex = x0;
    // main need a break or else PC will keep increasing
    asm("ebreak");
}