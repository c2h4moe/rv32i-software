#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <SDL2/SDL.h>
#define CHRBUF_MAX_LINES 30
#define CHRBUF_MAX_COLS 80 // character buffer size 30*80
#define GUIBUF_MAX_LINES 240
#define GUIBUF_MAX_COLS 320 // guibuffer size 60*80
#define SCREEN_W 640        // screen pixel width
#define SCREEN_H 480        // screen pixel height
using namespace std;
struct Keyboard
{
    char fifo[8];   // read write buffer
    int head, tail; // pointer head: next character to read; tail: next character to write
    int ready;      // whether the fifo has new character to read
    char get()
    {
        // simmulate the keyboard input fifo queue
        if (ready)
        {
            if ((head + 1) % 8 == tail)
            {
                ready = 0;
            }
            int res = fifo[head];
            head = (head + 1) % 8;
            return res;
        }
        else
        {
            return 0;
        }
    }
    void input(char c)
    {
        if ((tail + 1) % 8 == head)
        {
            return;
        }
        fifo[tail] = c;
        tail = (tail + 1) % 8;
        ready = 1;
    }
} kbd;

int ram[1024]; // simulate RAM
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
uint16_t *vmem;                                           // video memory
int stdout_buffer[CHRBUF_MAX_LINES * 32];                 // 30 * 128
uint16_t gui_buffer0[GUIBUF_MAX_LINES * GUIBUF_MAX_COLS]; // 60 * 128
uint16_t gui_buffer1[GUIBUF_MAX_LINES * GUIBUF_MAX_COLS];
int cur_gui_writebuffer = 0;
vector<string> ascii_shape;
int baseline = 0;
int gui = 0;
int cycle = 0;
int sleepstate = 0;
int sleepfor = 0;
auto timebegin = chrono::high_resolution_clock::now();
void init_asciirom()
{
    // store ascii shape
    ascii_shape.assign(256, string());
    std::ifstream file("asciirom.hex");
    std::string elem;
    int siz = 0;
    while (file >> elem)
    {
        std::string tmp;
        // change hex to binary
        for (int i = 0; i < elem.size(); i++)
        {
            int n = stoi(elem.substr(i, 1), nullptr, 16);
            for (int j = 3; j >= 0; j--)
            {
                if (n >> j & 1)
                {
                    tmp.push_back('1');
                }
                else
                {
                    tmp.push_back('0');
                }
            }
        }
        ascii_shape[siz++] = tmp;
    }
    file.close();
}
void flush_vmem()
{
    // update from buffer
    if (gui)
    {
        // vmem load the gui_buffer
        for (int i = 0; i < SCREEN_H; i++)
        {
            for (int j = 0; j < SCREEN_W; j++)
            {
                vmem[i * SCREEN_W + j] = cur_gui_writebuffer ? gui_buffer0[(i >> 1) * 320 + (j >> 1)] : gui_buffer1[(i >> 1) * 320 + (j >> 1)];
                // // GUI pixel size is 8*8
                // int pos = i / 8 * 80 + j / 8;
                // // notice that the pixel is 12 bit for convenience we treat it as 16 bit
                // // thereforer it is 16 bit rather than 32 bit in gui_buffer
                // // this code is to get the correct pixel value in the correct position
                // vmem[i * SCREEN_W + j] = gui_buffer[pos >> 1] >> ((pos & 1) * 16) & 0xffff;
            }
        }
    }
    else
    {
        for (int i = 0; i < SCREEN_H; i++)
        {
            for (int j = 0; j < SCREEN_W; j++)
            {
                // character size is 8*16
                // extract the ascii code from the stdout_buffer
                // each int in stdout_buffer store 4 ascii code
                int pos = i / 16 * 80 + j / 8;
                int ascii = 0;
                if (pos & 0b11 == 0)
                {
                    ascii = stdout_buffer[pos >> 2] & 0xff;
                }
                else if (pos & 0b11 == 1)
                {
                    ascii = stdout_buffer[pos >> 2] >> 8 & 0xff;
                }
                else if (pos & 0b11 == 2)
                {
                    ascii = stdout_buffer[pos >> 2] >> 16 & 0xff;
                }
                else if (pos & 0b11 == 3)
                {
                    ascii = stdout_buffer[pos >> 2] >> 24 & 0xff;
                }
                vmem[i * SCREEN_W + j] = ascii_shape[ascii][127 - ((i & 0xf) * 8 + j & 0b111)] == '1' ? 0xfff : 0;
            }
        }
    }
}
void update_vmem(int addr)
{
    // update from addr
    // the length of the instruction is 20
    int buffer_addr = addr & 0xfffff;
    if (gui)
    {
        int line = ((buffer_addr >> 1) / 320) << 1;
        int col = ((buffer_addr >> 1) % 320) << 1;
        for (int i = line; i < line + 2; i++)
        {
            for (int j = col; j < col + 2; j++)
            {
                vmem[i * SCREEN_W + j] = cur_gui_writebuffer ? gui_buffer0[buffer_addr >> 1] : gui_buffer1[buffer_addr >> 1];
            }
        }
        // line * 80 + col
        // int pos = (buffer_addr >> 1) / 128 * 80 + (buffer_addr >> 1) % 128;
        // for (int i = pos / 80 * 8; i < pos / 80 * 8 + 8; i++)
        // {
        //     for (int j = pos % 80 * 8; j < pos % 80 * 8 + 8; j++)
        //     {
        //         // extract high 16 bits or low 16 bits
        //         // buffer addr use bytes as unit
        //         // while gui_buffer use int as unit
        //         // and pixel use 16 bit as unit
        //         vmem[i * 640 + j] = (buffer_addr >> 1 & 1) ? gui_buffer[buffer_addr >> 2] >> 16 & 0xffff : gui_buffer[buffer_addr >> 2] & 0xffff;
        //     }
        // }
    }
    else
    {
        int ascii = 0;
        if ((buffer_addr & 0b11) == 0)
        {
            ascii = stdout_buffer[buffer_addr >> 2] & 0xff;
        }
        else if ((buffer_addr & 0b11) == 1)
        {
            ascii = stdout_buffer[buffer_addr >> 2] >> 8 & 0xff;
        }
        else if ((buffer_addr & 0b11) == 2)
        {
            ascii = stdout_buffer[buffer_addr >> 2] >> 16 & 0xff;
        }
        else if ((buffer_addr & 0b11) == 3)
        {
            ascii = stdout_buffer[buffer_addr >> 2] >> 24 & 0xff;
        }
        int pos = buffer_addr / 128 * 80 + buffer_addr % 128;
        for (int i = pos / 80 * 16; i < pos / 80 * 16 + 16; i++)
        {
            for (int j = pos % 80 * 8; j < pos % 80 * 8 + 8; j++)
            {
                vmem[i * 640 + j] = ascii_shape[ascii][(i & 0xf) * 8 + (j & 0b111)] == '1' ? 0xfff : 0;
            }
        }
    }
}
int mmio_read(int addr)
{
    int upper = (addr >> 20) & 0xfff;
    switch (upper)
    {
    case 0:
        return ram[(addr >> 2) & 1023];
        break;
    case 1:
        return stdout_buffer[(addr >> 2) & 1023];
        break;
    case 2:
        return baseline;
        break;
    case 3:
        return 0;
        break;
    case 4:
        return -1;
        break;
    case 5:
        return cycle;
        break;
    case 0xbad:
        return kbd.get();
        break;
    case 0xbee:
        return 0;
        break;
    default:
        return 0;
        break;
    }
}
void mmio_write(int addr, int din)
{
    int upper = (addr >> 20) & 0xfff;
    switch (upper)
    {
    case 0:
        ram[(addr >> 2) & 1023] = din;
        break;
    case 1:
        stdout_buffer[(addr >> 2) & 1023] = din;
        update_vmem(addr);
        break;
    case 2:
        baseline = din;
        break;
    case 3:
        if(cur_gui_writebuffer){
            gui_buffer1[(addr & 0xfffff) >> 1] = (addr >> 1 & 1) ? (din >> 16 & 0xffff) : (din & 0xffff);
        } else {
            gui_buffer0[(addr & 0xfffff) >> 1] = (addr >> 1 & 1) ? (din >> 16 & 0xffff) : (din & 0xffff);
        }
        // gui_buffer[(addr >> 2) & 4095] = din;
        update_vmem(addr);
        break;
    case 4:
        gui = din & 1;
        flush_vmem();
        break;
    case 5:
        sleepstate = 1;
        sleepfor = din;
        timebegin = chrono::high_resolution_clock::now();
        break;
    case 0xbee:
        if(din&1){
            cur_gui_writebuffer = !cur_gui_writebuffer;
            flush_vmem();
        }
        break;
    default:
        break;
    }
}
struct Simple_CPU
{
    std::vector<int> ROM;
    std::vector<int> regfile;
    int pc, next_pc, mem_we, mem_din, mem_wa, reg_we, reg_wa, reg_din, halt;
    Simple_CPU(const char *);
    void eval();
};

Simple_CPU::Simple_CPU(const char *src) : pc{0}, next_pc{0x8000000}, mem_we{0}, mem_din{0},
                                          mem_wa{0}, reg_we{0}, reg_wa{0},
                                          reg_din{0}, halt{0}
{
    regfile.assign(32, 0);
    ROM.assign(1024, 0);
    std::ifstream file(src);
    std::string inst;
    int size = 0;
    while (file >> inst)
    {
        ROM[size++] = std::stoll(inst, nullptr, 16);
    }
    file.close();
}
void Simple_CPU::eval()
{
    pc = next_pc;
    int inst = ROM[(pc & 0xfff) >> 2];
    int opcode = (inst >> 2) & 0b11111;
    int funct3 = (inst >> 12) & 0b111;
    int rs0 = (inst >> 15) & 0b11111;
    int rs1 = (inst >> 20) & 0b11111;
    int reg0 = regfile[rs0];
    int reg1 = regfile[rs1];
    int rd = (inst >> 7) & 0b11111;
    int I_imm = inst >> 20;
    int B_imm = ((inst >> 19) & 0xfffff000) | ((inst << 4) & 0x800) | ((inst >> 20) & 0b11111100000) | ((inst >> 7) & 0b11110);
    int S_imm = ((inst >> 20) & 0xffffffe0) | ((inst >> 7) & 0x1f);
    int U_imm = inst & 0xfffff000;
    int J_imm = ((inst >> 11) & 0xfff00000) | (inst & 0x000ff000) | ((inst >> 9) & 0x800) | ((inst >> 20) & 0x7fe);
    int addr = reg0 + I_imm;
    int jump = 0;
    int mem_dout = 0;
    next_pc = pc + 4;
    halt = 0;
    switch (opcode)
    {
    case 0xc:
        reg_we = 1;
        reg_wa = rd;
        mem_we = 0;
        switch (funct3)
        {
        case 0:
            reg_din = (inst >> 30 & 1) ? reg0 - reg1 : reg0 + reg1;
            break;
        case 1:
            reg_din = reg0 << reg1;
            break;
        case 2:
            reg_din = reg0 < reg1;
            break;
        case 3:
            reg_din = ((unsigned)reg0) < ((unsigned)reg1);
            break;
        case 4:
            reg_din = reg0 ^ reg1;
            break;
        case 5:
            reg_din = (inst >> 30 & 1) ? (reg0 >> reg1) : ((unsigned)reg0) >> ((unsigned)reg1);
            break;
        case 6:
            reg_din = reg0 | reg1;
            break;
        case 7:
            reg_din = reg0 & reg1;
            break;
        default:
            reg_din = 0;
            break;
        }
        regfile[rd] = reg_din;
        break;
    case 0x4:
        reg_wa = rd;
        reg_we = 1;
        mem_we = 0;
        switch (funct3)
        {
        case 0:
            reg_din = reg0 + I_imm;
            break;
        case 1:
            reg_din = reg0 << (I_imm & 0x1f);
            break;
        case 2:
            reg_din = reg0 < I_imm;
            break;
        case 3:
            reg_din = ((unsigned)reg0) < ((unsigned)I_imm);
            break;
        case 4:
            reg_din = reg0 ^ I_imm;
            break;
        case 5:
            reg_din = (inst >> 30 & 1) ? reg0 >> (I_imm & 0x1f) : ((unsigned)reg0) >> ((unsigned)(I_imm & 0x1f));
            break;
        case 6:
            reg_din = reg0 | I_imm;
            break;
        case 7:
            reg_din = reg0 & I_imm;
            break;
        default:
            reg_din = 0;
            break;
        }
        regfile[rd] = reg_din;
        break;
    case 0x0:
        reg_wa = rd;
        reg_we = 1;
        mem_we = 0;
        switch (funct3)
        {
        case 0:
            reg_din = ((mmio_read(addr)) << (24 - (addr & 0x3) * 8)) >> 24;
            break;
        case 1:
            reg_din = ((mmio_read(addr)) << (16 - (addr >> 1 & 1))) >> 16;
            break;
        case 2:
            reg_din = mmio_read(addr);
            break;
        case 4:
            reg_din = ((mmio_read(addr)) >> ((addr & 0x3) * 8)) & 0xff;
            break;
        case 5:
            reg_din = ((mmio_read(addr)) >> ((addr >> 1 & 1) * 16)) & 0xffff;
            break;
        default:
            reg_din = 0;
            break;
        }
        regfile[rd] = reg_din;
        break;
    case 0x18:
        reg_we = 0;
        mem_we = 0;
        jump = 0;
        switch (funct3)
        {
        case 0:
            jump = reg0 == reg1;
            break;
        case 1:
            jump = reg0 != reg1;
            break;
        case 4:
            jump = reg0 < reg1;
            break;
        case 5:
            jump = reg0 >= reg1;
            break;
        case 6:
            jump = ((unsigned)reg0) < ((unsigned)reg1);
            break;
        case 7:
            jump = ((unsigned)reg0) >= ((unsigned)reg1);
            break;
        default:
            jump = 0;
            break;
        }
        if (jump)
        {
            next_pc = pc + B_imm;
        }
        break;
    case 0x08:
        reg_we = 0;
        mem_we = 1;
        mem_wa = reg0 + S_imm;
        mem_dout = mmio_read(mem_wa);
        if (funct3 == 0)
        {
            switch (mem_wa & 3)
            {
            case 0:
                mem_din = (mem_dout & (~0xff)) | (reg1 & 0xff);
                break;
            case 1:
                mem_din = (mem_dout & (~0xff00)) | ((reg1 & 0xff) << 8);
                break;
            case 2:
                mem_din = (mem_dout & (~0xff0000)) | ((reg1 & 0xff) << 16);
                break;
            case 3:
                mem_din = (mem_dout & (~0xff000000)) | ((reg1 & 0xff) << 24);
                break;
            }
        }
        else if (funct3 == 1)
        {
            switch (mem_wa >> 1 & 1)
            {
            case 0:
                mem_din = (mem_dout & (~0xffff)) | (reg1 & 0xffff);
                break;
            case 1:
                mem_din = (mem_dout & (~0xffff0000)) | ((reg1 & 0xffff) << 16);
                break;
            }
        }
        else
        {
            mem_din = reg1;
        }
        mmio_write(mem_wa, mem_din);
        break;
    case 0x0d:
        mem_we = 0;
        reg_we = 1;
        reg_wa = rd;
        reg_din = U_imm;
        regfile[rd] = reg_din;
        break;
    case 0x05:
        mem_we = 0;
        reg_we = 1;
        reg_wa = rd;
        reg_din = pc + U_imm;
        regfile[rd] = reg_din;
        break;
    case 0x1b:
        mem_we = 0;
        reg_we = 1;
        reg_wa = rd;
        reg_din = pc + 4;
        next_pc = pc + J_imm;
        regfile[rd] = reg_din;
        break;
    case 0x19:
        mem_we = 0;
        reg_we = 1;
        reg_wa = rd;
        reg_din = pc + 4;
        next_pc = regfile[rs0] + I_imm;
        regfile[rd] = reg_din;
        break;
    case 0x1c:
        mem_we = 0;
        reg_we = 0;
        next_pc = pc;
        halt = 1;
        break;
    }
    regfile[0] = 0;
}
void init_mem(const char *ram_file)
{
    std::ifstream file(ram_file);
    std::string elem;
    int size = 0;
    while (file >> elem)
    {
        ram[size++] = std::stoll(elem, nullptr, 16);
    }
    file.close();
    vmem = new uint16_t[SCREEN_H * SCREEN_W]{0};
}
void doInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            kbd.input(event.key.keysym.sym == '\r' ? '\n' : event.key.keysym.sym);
            break;
        default:
            break;
        }
    }
}
static inline void update_screen()
{
    if (gui)
    {
        SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint16_t));
    }
    else
    {
        SDL_Rect rect_u;
        rect_u.x = rect_u.y = 0;
        rect_u.w = SCREEN_W;
        rect_u.h = SCREEN_H - baseline * 16;
        SDL_Rect rect_d;
        rect_d.x = 0;
        rect_d.y = rect_u.h;
        rect_d.w = SCREEN_W;
        rect_d.h = SCREEN_H - rect_u.h;
        SDL_UpdateTexture(texture, &rect_u, vmem + baseline * 16 * 640, SCREEN_W * sizeof(uint16_t));
        SDL_UpdateTexture(texture, &rect_d, vmem, SCREEN_W * sizeof(uint16_t));
    }
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
int main(int argc, char **argv)
{
    assert(argc == 3);
    const char *rom_file = argv[1];
    const char *ram_file = argv[2];
    Simple_CPU cpu(rom_file);
    init_mem(ram_file);
    init_asciirom();
    SDL_Window *window = NULL;
    char title[128] = "RV32I-EMU";
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
    SDL_SetWindowTitle(window, title);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB444,
                                SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
    int cnt = 0;
    // auto ta = chrono::high_resolution_clock::now();
    // int cntcycle = 0;
    while (1)
    {
        if (!sleepstate)
        {
            cpu.eval();
        }
        else
        {
            auto curtime = chrono::high_resolution_clock::now();
            auto dur = chrono::duration_cast<chrono::milliseconds>(curtime - timebegin);
            if (dur.count() > sleepfor)
            {
                sleepstate = 0;
            }
        }
        // printf("pc:%x, mem_wa:%x, mem_we:%d, mem_din:%x\n", cpu.pc, cpu.mem_wa, cpu.mem_we, cpu.mem_din);
        cnt++;
        if (cnt == 2000)
        {
            update_screen();
            cnt = 0;
        }
        doInput();
        // auto tb = chrono::high_resolution_clock::now();
        // auto dur = std::chrono::duration_cast<chrono::milliseconds>(tb - ta);
        // cntcycle ++;
        // if(dur.count() > 1000){
        //     cerr<<cntcycle<<"\n";
        //     cntcycle = 0;
        //     ta = chrono::high_resolution_clock::now();
        // }
        cycle++;
    }
}