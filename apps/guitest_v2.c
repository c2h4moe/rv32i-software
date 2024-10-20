#include "basictime.h"
#define GUICTL_SPACE 0x00400000
volatile short (*vram)[320] = (volatile short (*)[320])0x00300000;
short colors[8] = {
    0xf00,
    0x0f0,
    0x00f,
    0xff0,
    0xf0f,
    0x0ff,
    0xfff,
    0xccc};
int main()
{
    *((volatile int *)GUICTL_SPACE) = 1;
    while (1)
    {
        for (int i = 100; i < 160; i++)
        {
            for (int j = 30; j < 30 + i - 100; j++)
            {
                vram[i][j] = colors[(i - 100) & 7];
            }
        }
        sleep(5000);
        for (int k = 0; k < 50; k++)
        {
            for (int i = 100 + k; i < 160 + k; i++)
            {
                for (int j = 30 + k; j < 30 + k + i - 100 - k; j++)
                {
                    vram[i][j] = colors[(i - 100 - k) & 7];
                }
            }
            sleep(80);
            for (int i = 100 + k; i < 160 + k; i++)
            {
                for (int j = 30 + k; j < 30 + k + i - 100 - k; j++)
                {
                    vram[i][j] = 0;
                }
            }
        }
        for (int i = 100; i < 160; i++)
        {
            for (int j = 30; j < 30 + i - 100; j++)
            {
                vram[i][j] = colors[(i - 100) & 7];
            }
        }
    }
}