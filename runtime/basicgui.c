#include <basicgui.h>
#define KEYBOARD_ADDR (0xbad00000)
#define GUI_MODE_BASE_ADDR (0x00300000)
#define MODE_CONTROL_ADDR (0x00400000)

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60
#define VMEM_WIDTH 128
#define VMEM_HEIGHT 60

color_rgb common_colors[] = {
    {0xf, 0, 0},     // red
    {0, 0xf, 0},     // green
    {0, 0, 0xf},     // blue
    {0xf, 0xf, 0},   // yellow
    {0, 0xf, 0xf},   // cyan
    {0xf, 0, 0xf},   // megenta
    {0xf, 0xf, 0xf}, // white
    {0, 0, 0}};      // black

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

void DrawStraightLine(int x0, int y0, int x1, int y1, color_t color)
{
    // drawing direction
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int tx0 = x0, ty0 = y0, tx1 = x1, ty1 = y1;
    while (1)
    {
        DrawPoint(tx0, ty0, color);
        if (tx0 == tx1 && ty0 == ty1)
            break;
        if (tx0 == tx1)
            ty0 += sy;
        else if (ty0 == ty1)
            tx0 += sx;
    }
}

// void swap(int *a, int *b)
// {
//     int temp = *a;
//     *a = *b;
//     *b = temp;
// }

// draw diagonal line
// Bresenham's line algorithm from Wikipedia
// boolean steep := abs(y1 - y0) > abs(x1 - x0)
//      if steep then
//          swap(x0, y0)
//          swap(x1, y1)
//      if x0 > x1 then
//          swap(x0, x1)
//          swap(y0, y1)
//      int deltax := x1 - x0
//      int deltay := abs(y1 - y0)
//      int error := deltax / 2
//      int ystep
//      int y := y0
//      if y0 < y1 then ystep := 1 else ystep := -1
//      for x from x0 to x1
//          if steep then plot(y,x) else plot(x,y)
//          error := error - deltay
//          if error < 0 then
//              y := y + ystep
//              error := error + deltax