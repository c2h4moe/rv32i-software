#define KEYBOARD_ADDR (0xbad00000)

typedef enum {
    IDLE,
    EXTEND,
    BREAK
} KbdReadState;
KbdReadState curState = IDLE;

#ifndef SIM_MODE
int getchark(){
    char scan_code = *(volatile char *)KEYBOARD_ADDR;
    char res = 0;
    if(curState == IDLE){
        if (scan_code == 0xF0){
            curState = BREAK;
        } else if (scan_code == 0xE0) {
            curState = EXTEND;
        } else {
            if(scan_code == 0x6B){  // left direction
                res = 3;
            } else if(scan_code == 0x75) { // up direction
                res = 1;
            } else if(scan_code == 0x72){ // down direction
                res = 2;
            } else if(scan_code == 0x29){
                res = ' ';
            } else if(scan_code == 0x5A){
                res = '\n';
            }
            curState = IDLE;
        }
    } else if(curState == EXTEND) {
        if (scan_code == 0xF0){
            curState = BREAK;
        } else {
            if(scan_code == 0x74){  // right direction
                res = 4;
            }else {
                curState = IDLE;
            }
        }
    } else {
        curState = IDLE;
    }
    return res;
}
#else
int getchark()
{
    char c = *(char *)KEYBOARD_ADDR;
    if (c == 0)
    {
        return -1;
    }
    else
    {
        return c;
    }
}
#endif