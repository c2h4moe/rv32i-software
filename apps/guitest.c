#define GUICTL_SPACE 0x00400000
short (*vram)[128] = (short (*)[128])0x00300000;
void sleep(int units){
    for(int i=0;i<units;i++){
        for(int j=0;j<7000000;j++){
            asm("nop");
        }
    }
}
int main(){
    *(int*)GUICTL_SPACE = 1;
    short red = 0x0f00;
    for(int i = 20; i < 40; i ++) {
        for(int j = 30; j < 50; j ++) {
            vram[i][j] = red;
        }
    }
    sleep(5);
    for(int k=0;k<20;k++){
        for(int i = 20; i < 40; i ++) {
            for(int j = 30; j < 50; j ++) {
                vram[i+k][j] = red;
            }
        }
        sleep(1);
        for(int i = 20; i < 40; i ++) {
            for(int j = 30; j < 50; j ++) {
                vram[i+k][j] = 0;
            }
        }
    }
    for(int i = 20; i < 40; i ++) {
        for(int j = 30; j < 50; j ++) {
            vram[i][j] = red;
        }
    }
    asm("ebreak");
    return 0;
}