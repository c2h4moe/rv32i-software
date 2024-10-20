#include "basictime.h"
#define GUICTL_SPACE 0x00400000
#define CHANGE_BUFFER_SPACE 0xbee00000
volatile unsigned short (*vram)[320] = (volatile unsigned short (*)[320])0x00300000;
short colors[8] = {
    0xf00,
    0x0f0,
    0x00f,
    0xff0,
    0xf0f,
    0x0ff,
    0xfff,
    0xccc};
void change_buffer(){
    *((volatile unsigned int*)CHANGE_BUFFER_SPACE) = 1;
}
void erase(){
    for (int i = 0; i < 240; i++)
    {
        for (int j = 0; j < 320; j++)
        {
            vram[i][j] = 0;
        }
    }
}
int main()
{
    *((volatile int *)GUICTL_SPACE) = 1;
    sleep(10000);
    while (1)
    {
        erase();
        for (int i = 100; i < 160; i++)
        {
            for (int j = 30; j < 30 + i - 100; j++)
            {
                vram[i][j] = colors[(i - 100) & 7];
            }
        }
        change_buffer();
        sleep(5000);
        
        for (int k = 0; k < 50; k++)
        {
            erase();
            for (int i = 100 + k; i < 160 + k; i++)
            {
                for (int j = 30 + k; j < 30 + k + i - 100 - k; j++)
                {
                    vram[i][j] = colors[(i - 100 - k) & 7];
                }
            }
            change_buffer();
            // sleep(80);
        }
    }
}