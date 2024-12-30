#define KEYBOARD_ADDR (0xbad00000)

#ifndef SIM_MODE
int getchark(){
    char scan_code = *(volatile char *)KEYBOARD_ADDR;
    if(scan_code == 0) {
        return 0;
    }
    char res = 0;
    if (scan_code == 0xF0){
        while((*(volatile char *)KEYBOARD_ADDR) == 0) {}
    } else if (scan_code == 0xE0) {
        scan_code = *(volatile char *)KEYBOARD_ADDR;
        while(scan_code == 0) {
            scan_code = *(volatile char *)KEYBOARD_ADDR;
        }
        if(scan_code == 0xF0) { // BREAK
            while((*(volatile char *)KEYBOARD_ADDR) == 0) {}
        } else {
            if(scan_code == 0x74) { // right direction
                res = 4;
            }
        }
    } else {
        if(scan_code == 0x6B) {  // left direction
            res = 3;
        } else if(scan_code == 0x75) { // up direction
            res = 1;
        } else if(scan_code == 0x72) { // down direction
            res = 2;
        } else if(scan_code == 0x29) {
            res = ' ';
        } else if(scan_code == 0x5A) {
            res = '\n';
        }
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