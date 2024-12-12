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
    int head = 0, tail = 0; // pointer head: next character to read; tail: next character to write
    int ready = 0;      // whether the fifo has new character to read
    char get()
    {
        // simmulate the keyboard input fifo queue
        if (ready)
        {
            if ((head + 1) % 8 == tail) // fifo has only one element
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

bool shouldRender = false; // 在这个周期是否应该渲染,set in mmio_write
int game_mode = 0;
int game_buf = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int ram[4000000]; // simulate RAM
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
    SDL_Texture *doodle_right;
    SDL_Texture *doodle_up;
};

PictureElements pic_elem;

enum ImageType {
    PLATFORM,
    FAKE_PLATFORM,
    MONSTER,
    BALL,
    SPRING_OR_PROPELLER,
    DOODLE
};

enum class RegNum {
    PLATFORM = 16,
    FAKE_PLATFORM = 4,
    MONSTER = 2,
    BALL = 4,
    SPRING_OR_PROPELLER = 4,
    DOODLE = 1
};
int RegStart[6] = {0, 16, 20, 22, 26, 30}; //第i项表示第i类寄存器的在gameRegs中的起始序号，从0开始

#define REGS_NUM 31
#define PIC_NUM 14

typedef struct {
    int type; //固有属性,只在initReg里面设置
    int content;
    int xAddr;
    int yAddr;
    int num;
    int getyAddr() {
        return yAddr;
    }

    int getxAddr() {
        return xAddr;
    }

    int getNum() {
        return num;
    }

    void set(int content) {
        this->content = content;
        this->yAddr = content & 0x7ff;
        this->xAddr = (content >> 11) & 0x7ff;
        this->num = (content >> 22) & 0x3ff;
    }
} imageReg;

imageReg gameRegs[REGS_NUM];

typedef struct {
    int type;
    int num;
    int wide;
    int high;
    SDL_Texture * texture;

    void draw(imageReg * reg) {
        int x = reg->getxAddr();
        int y = reg->getyAddr();
        SDL_Rect dest = {x, y, wide, high};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
    }
} image;

image gameImgs[PIC_NUM];

image * findImage(int type, int num) {
    for (int i = 0; i < PIC_NUM; i++) {
        if (gameImgs[i].type == type && gameImgs[i].num == num) {
            return &gameImgs[i];
        }
    }
    return NULL;
}



void initImage() {
    gameImgs[0].type = PLATFORM;
    gameImgs[0].num = 1;
    gameImgs[0].wide = GREEN_PLATFORM_W;
    gameImgs[0].high = GREEN_PLATFORM_H;
    gameImgs[0].texture = pic_elem.green_platform;

    gameImgs[1].type = PLATFORM;
    gameImgs[1].num = 2;
    gameImgs[1].wide = LIGHT_BLUE_PLATFORM_W;
    gameImgs[1].high = LIGHT_BLUE_PLATFORM_H;
    gameImgs[1].texture = pic_elem.light_blue_platform;

    gameImgs[2].type = FAKE_PLATFORM;
    gameImgs[2].num = 1;
    gameImgs[2].wide = BROWN_PLATFORM_BREAKING_1_W;
    gameImgs[2].high = BROWN_PLATFORM_BREAKING_1_H;
    gameImgs[2].texture = pic_elem.brown_platform_breaking_1;

    gameImgs[3].type = FAKE_PLATFORM;
    gameImgs[3].num = 2;
    gameImgs[3].wide = BROWN_PLATFORM_BREAKING_2_W;
    gameImgs[3].high = BROWN_PLATFORM_BREAKING_2_H;
    gameImgs[3].texture = pic_elem.brown_platform_breaking_2;

    gameImgs[4].type = MONSTER;
    gameImgs[4].num = 1;
    gameImgs[4].wide = PURPLE_MONSTER_W;
    gameImgs[4].high = PURPLE_MONSTER_H;
    gameImgs[4].texture = pic_elem.purple_monster;

    gameImgs[5].type = MONSTER;
    gameImgs[5].num = 2;
    gameImgs[5].wide = RED_MONSTER_W;
    gameImgs[5].high = RED_MONSTER_H;
    gameImgs[5].texture = pic_elem.red_monster;

    gameImgs[6].type = BALL;
    gameImgs[6].num = 1;
    gameImgs[6].wide = BALL_W;
    gameImgs[6].high = BALL_H;
    gameImgs[6].texture = pic_elem.ball;

    gameImgs[7].type = SPRING_OR_PROPELLER;
    gameImgs[7].num = 1;
    gameImgs[7].wide = SPRING_COMPRESSED_W;
    gameImgs[7].high = SPRING_COMPRESSED_H;
    gameImgs[7].texture = pic_elem.spring_compressed;

    gameImgs[8].type = SPRING_OR_PROPELLER;
    gameImgs[8].num = 2;
    gameImgs[8].wide = SPRING_FULL_W;
    gameImgs[8].high = SPRING_FULL_H;
    gameImgs[8].texture = pic_elem.spring_full;

    gameImgs[9].type = SPRING_OR_PROPELLER;
    gameImgs[9].num = 3;
    gameImgs[9].wide = PROPELLER_W;
    gameImgs[9].high = PROPELLER_H;
    gameImgs[9].texture = pic_elem.propeller;

    gameImgs[10].type = SPRING_OR_PROPELLER;
    gameImgs[10].num = 4;
    gameImgs[10].wide = PROPELLER_RUNNING_W;
    gameImgs[10].high = PROPELLER_RUNNING_H;
    gameImgs[10].texture = pic_elem.propeller_running;

    gameImgs[11].type = DOODLE;
    gameImgs[11].num = 1;
    gameImgs[11].wide = DOODLE_W;
    gameImgs[11].high = DOODLE_H;
    gameImgs[11].texture = pic_elem.doodle;

    gameImgs[12].type = DOODLE;
    gameImgs[12].num = 2;
    gameImgs[12].wide = DOODLE_W;
    gameImgs[12].high = DOODLE_H;
    gameImgs[12].texture = pic_elem.doodle_right;

    gameImgs[13].type = DOODLE;
    gameImgs[13].num = 3;
    gameImgs[13].wide = DOODLE_UP_W;
    gameImgs[13].high = DOODLE_UP_H;
    gameImgs[13].texture = pic_elem.doodle_up;
}

void initRegs() {
    for(int i = 0; i < REGS_NUM; i++) {
        gameRegs[i].set(0);
    }

    int index = 0;

    // Initialize PLATFORM registers
    for(int i = 0; i < static_cast<int>(RegNum::PLATFORM); i++, index++) {
        gameRegs[index].type = PLATFORM;
    }

    // Initialize FAKE_PLATFORM registers
    for(int i = 0; i < static_cast<int>(RegNum::FAKE_PLATFORM); i++, index++) {
        gameRegs[index].type = FAKE_PLATFORM;
    }

    // Initialize MONSTER registers
    for(int i = 0; i < static_cast<int>(RegNum::MONSTER); i++, index++) {
        gameRegs[index].type = MONSTER;
    }

    // Initialize BALL registers
    for(int i = 0; i < static_cast<int>(RegNum::BALL); i++, index++) {
        gameRegs[index].type = BALL;
    }

    // Initialize SPRING_OR_PROPELLER registers
    for(int i = 0; i < static_cast<int>(RegNum::SPRING_OR_PROPELLER); i++, index++) {
        gameRegs[index].type = SPRING_OR_PROPELLER;
    }

    // Initialize DOODLE registers
    for(int i = 0; i < static_cast<int>(RegNum::DOODLE); i++, index++) {
        gameRegs[index].type = DOODLE;
    }
}

void render_game()
{
    SDL_RenderClear(renderer);
    int i;
    imageReg * reg;
    image * img;
    for( i = 0; i < REGS_NUM - 5; i++) {
        reg = &gameRegs[i];
        if(reg->num == 0) {
            continue;
        }
        img = findImage(reg->type, reg->getNum());
        if (img != NULL) {
            img->draw(reg);
        }
    }
    
    //之前的优先级和寄存器顺序是对应的，但是主角和道具要颠倒一下
    reg = &gameRegs[REGS_NUM - 1];
    img = findImage(reg->type, reg->getNum());
    if (img != NULL) {
        img->draw(reg);
    }

    for(;i < REGS_NUM - 1; i++) {
        reg = &gameRegs[i];
        if(reg->num == 0) {
            continue;
        }
        img = findImage(reg->type, reg->getNum());
        if (img != NULL) {
            img->draw(reg);
        }
    }

    SDL_RenderPresent(renderer);
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
    pic_elem.doodle_right = loadTexture("./image/doodle_right.png");
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
        return ram[(addr & 0xfffff) >> 2];;
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
    int typeNum = (addr >> 6) & 0x7;  //寄存器类别的序号，0是平台类寄存器
    int typeIndex = (addr >> 2) & 0xf; //在该类别中是第几个，0表示第一个
    int regIndex = RegStart[typeNum] + typeIndex;
    switch (upper)
    {
    case 0:
        ram[(addr & 0xfffff) >> 2] = din; // 支持1MiB寻址
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        /* TODO */
        gameRegs[regIndex].set(din);
        shouldRender = true;
        break;
    case 4:
        break;
    case 5:
        sleepstate = 1;
        sleepfor = din;
        timebegin = std::chrono::high_resolution_clock::now();
        break;
    case 0xbee:
        std::cerr<<static_cast<char>(din);
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
    ROM.assign(4096, 0);
    std::ifstream file(src);
    std::string inst;
    int size = 0;
    while (file >> inst)
    {
        std::cout<<inst<<std::endl;
        ROM[size++] = std::stoll(inst, nullptr, 16);
    }
    file.close();
}
void Simple_CPU::eval()
{
    if(halt == 1) {
        return;
    }
    pc = next_pc;
    
    int inst = ROM[(pc & 0x3fff) >> 2]; // 最多支持4096条指令
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
    memset(ram,0,sizeof(ram));
    std::ifstream file(ram_file);
    std::string elem;
    int size = 0;
    while (file >> elem)
    {
        ram[size++] = std::stoll(elem, nullptr, 16);
    }
    file.close();
}
void doInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT) {
            exit(0);
        }
        else if(event.type == SDL_KEYDOWN) {
            //kbd.input(event.key.keysym.sym == '\r' ? '\n' : event.key.keysym.sym);
            switch(event.key.keysym.sym) {
                case SDLK_UP:
                    kbd.input(0x1);
                    break;
                case SDLK_DOWN:
                    kbd.input(0x2);
                    break;
                case SDLK_LEFT:
                    kbd.input(0x3);
                    break;
                case SDLK_RIGHT:
                    kbd.input(0x4);
                    break;
                case SDLK_RETURN:
                    kbd.input(0x0D);
                    break;
                case SDLK_SPACE:
                    kbd.input(0x20);
                    break;
                default:
                    break;
            }
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
    initImage();
    initRegs();     
    
    while (true)
    {
        doInput();
        if (!sleepstate)
        {
            cpu.eval();
            if(shouldRender) { //只有当前指令修改了imgReg才会再渲染
                render_game();
                shouldRender = false;
            }
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
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}