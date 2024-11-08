#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define SCREEN_W 1280
#define SCREEN_H 1024
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
#define LIGHT_BLUE_PLATFORM_W 90
#define LIGHT_BLUE_PLATFORM_H 23
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
int game_mode = 0;
int game_buf = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
std::vector<int> game_regs; // 游戏寄存器
int ram[1024]; // simulate RAM
std::vector<std::string> ascii_shape;
int cycle = 0;
int sleepstate = 0;
int sleepfor = 0;
auto timebegin = std::chrono::high_resolution_clock::now();
struct PictureElements
{
    SDL_Texture *green_platform;
    SDL_Texture *light_blue_platform;
    SDL_Texture *brown_platform_breaking_1;
    SDL_Texture *brown_platform_breaking_2;
    SDL_Texture *spring_compressed;
    SDL_Texture *spring_full;
    SDL_Texture *green_monster;
    SDL_Texture *red_monster;
    SDL_Texture *purple_monster;
    SDL_Texture *ball;
    SDL_Texture *propeller;
    SDL_Texture *propeller_running;
    SDL_Texture *doodle;
    SDL_Texture *doodle_up;
};
PictureElements pic_elem;
void render_game()
{
    /* TODO */
}

SDL_Texture *loadTexture(std::string path)
{
    // The final texture
    SDL_Texture *newTexture = NULL;

    // Load image at specified path

    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else
    {
        // Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL)
        {
            printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }

        // Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}
void loadMedia()
{
    pic_elem.green_platform = loadTexture("./image/green_platform.png");
    pic_elem.light_blue_platform = loadTexture("./image/light_blue_platform.png");
    pic_elem.brown_platform_breaking_1 = loadTexture("./image/brown_platform_breaking_1.png");
    pic_elem.brown_platform_breaking_2 = loadTexture("./image/brown_platform_breaking_3.png");
    pic_elem.spring_compressed = loadTexture("./image/spring_compressed.png");
    pic_elem.spring_full = loadTexture("./image/spring_full.png");
    pic_elem.red_monster = loadTexture("./image/red_monster.png");
    pic_elem.purple_monster = loadTexture("./image/purple_monster.png");
    pic_elem.ball = loadTexture("./image/ball.png");
    pic_elem.propeller = loadTexture("./image/propeller.png");
    pic_elem.propeller_running = loadTexture("./image/propeller_running.png");
    pic_elem.doodle = loadTexture("./image/doodle.png");
    pic_elem.doodle_up = loadTexture("./image/doodle_up.png");
}
SDL_Texture *texture = NULL;
void init_asciirom()
{
    // store ascii shape
    ascii_shape.assign(256, std::string());
    std::ifstream file("./asciirom.hex");
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
int mmio_read(int addr)
{
    int upper = (addr >> 20) & 0xfff;
    switch (upper)
    {
    case 0:
        return 0;
        break;
    case 1:
        return 0;
        break;
    case 2:
        return 0;
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
        break;
    case 2:
        break;
    case 3:
        /* TODO */
        break;
    case 4:
        break;
    case 5:
        sleepstate = 1;
        sleepfor = din;
        timebegin = std::chrono::high_resolution_clock::now();
        break;
    case 0xbee:
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
    game_regs0.assign(32, 0);
    game_regs1.assign(32, 0);
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
int main(int argc, char **argv)
{
    assert(argc == 3);
    const char *rom_file = argv[1];
    const char *ram_file = argv[2];
    Simple_CPU cpu(rom_file);
    init_mem(ram_file);
    char title[128] = "RV32I-EMU";
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(1280, 1024, 0, &window, &renderer);
    SDL_SetWindowTitle(window, title);
    SDL_SetRenderDrawColor(renderer, 247, 238, 214, SDL_ALPHA_OPAQUE);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB444,
                                SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);

    loadMedia();
    render_game();
    while (true)
    {
        if (!sleepstate)
        {
            cpu.eval();
        }
        else
        {
            auto curtime = std::chrono::high_resolution_clock::now();
            auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - timebegin);
            if (dur.count() > sleepfor)
            {
                sleepstate = 0;
            }
        }
        cycle++;
        doInput();
    }
}