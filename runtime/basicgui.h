
typedef unsigned short color_t;
typedef struct color_rgb
{
    // each color is 4 bits only not an integer
    int r;
    int g;
    int b;
} color_rgb;

extern color_rgb common_colors[];

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

extern color_t TransferColor(color_rgb c);
extern void DrawPoint(int x, int y, color_t color);
extern void DrawStraightLine(int x1, int y1, int x2, int y2, color_t color);

