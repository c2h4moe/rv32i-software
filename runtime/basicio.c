#define STDOUT_BUFFER_BASE (0x100000)
#define TTY_SCAN_BASELINE (0x200000)
#define KEYBOARD_ADDR (0xbad00000)
#define STDOUT_LINE_SIZE (80)
static volatile char (*stdout_buffer)[80] = (volatile char (*)[80])STDOUT_BUFFER_BASE;
static int scan_baseline = 0;
static int tty_screen_full = 0;
static int CUR_LINE = 0;
static int CUR_COL = 0;
static char kbd_shift = 0;
static char kbd_capslock = 0;

typedef enum {
    IDLE,
    EXTEND,
    BREAK
} KbdReadState;
KbdReadState curState = IDLE;

// // scancode is what keyboard input which need to transfer to ascii
// static char scancode_to_ascii[128] = {
//     [0x16] = '1',
//     [0x1E] = '2',
//     [0x26] = '3',
//     [0x25] = '4',
//     [0x2E] = '5',
//     [0x36] = '6',
//     [0x3D] = '7',
//     [0x3E] = '8',
//     [0x46] = '9',
//     [0x45] = '0',
//     [0x1C] = 'a',
//     [0x32] = 'b',
//     [0x21] = 'c',
//     [0x23] = 'd',
//     [0x24] = 'e',
//     [0x2B] = 'f',
//     [0x34] = 'g',
//     [0x33] = 'h',
//     [0x43] = 'i',
//     [0x3B] = 'j',
//     [0x42] = 'k',
//     [0x4B] = 'l',
//     [0x3A] = 'm',
//     [0x31] = 'n',
//     [0x44] = 'o',
//     [0x4D] = 'p',
//     [0x15] = 'q',
//     [0x2D] = 'r',
//     [0x1B] = 's',
//     [0x2C] = 't',
//     [0x3C] = 'u',
//     [0x2A] = 'v',
//     [0x1D] = 'w',
//     [0x22] = 'x',
//     [0x35] = 'y',
//     [0x1A] = 'z',
//     [0x0E] = '`',
//     [0x4E] = '-',
//     [0x55] = '=',
//     [0x54] = '[',
//     [0x5B] = ']',
//     [0x5D] = '\\',
//     [0x4C] = ';',
//     [0x52] = '\'',
//     [0x41] = ',',
//     [0x49] = '.',
//     [0x4A] = '/',
//     [0x29] = ' ',
//     [0x5A] = '\n',
//     [0x66] = 0x08, // BS
// };


    

#ifndef SIM_MODE
#ifndef TTY_MODE
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
            if(scan_code == 0x74){  // left direction
                res = 3;
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
    char scan_code = *(volatile char *)KEYBOARD_ADDR;
    char res = 0;
    if (scan_code == 0)
    {
        return -1;
    }
    
    if (scan_code == 0xF0)
    {
        scan_code = 0;
        while (scan_code == 0)
        {
            scan_code = *(volatile char *)KEYBOARD_ADDR;
        }
        if (scan_code == 0x12)
        {
            // shift release
            kbd_shift = 0;
        }
        return -1;
    }
    else
    {
        if (scan_code == 0x58)
        { // capslock
            kbd_capslock = !kbd_capslock;
            return -1;
        }
        else if (scan_code == 0x12)
        { // shift
            kbd_shift = 1;
            return -1;
        }
        else
        {
            res = scancode_to_ascii[scan_code];
            if ((kbd_capslock ^ kbd_shift) && res >= 97 && res <= 122)
            {
                res -= 32;
            }
            else if (kbd_shift)
            {
                switch (res)
                {
                case '1':
                    res = '!';
                    break;
                case '2':
                    res = '@';
                    break;
                case '3':
                    res = '#';
                    break;
                case '4':
                    res = '$';
                    break;
                case '5':
                    res = '%';
                    break;
                case '6':
                    res = '^';
                    break;
                case '7':
                    res = '&';
                    break;
                case '8':
                    res = '*';
                    break;
                case '9':
                    res = '(';
                    break;
                case '0':
                    res = ')';
                    break;
                case '`':
                    res = '~';
                    break;
                case '-':
                    res = '_';
                    break;
                case '=':
                    res = '+';
                    break;
                case '[':
                    res = '{';
                    break;
                case ']':
                    res = '}';
                    break;
                case '\\':
                    res = '|';
                    break;
                case ';':
                    res = ':';
                    break;
                case '\'':
                    res = '"';
                    break;
                case ',':
                    res = '<';
                    break;
                case '.':
                    res = '>';
                    break;
                case '/':
                    res = '?';
                    break;
                default:
                    break;
                }
            }
            return res;
        }
    }
}
#endif
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
static void newline()
{
    if (CUR_LINE == 31)
    {
        tty_screen_full = 1;
    }
    CUR_LINE = (CUR_LINE + 1) & 31;
    for (int j = 0; j < STDOUT_LINE_SIZE; j++)
    {
        stdout_buffer[CUR_LINE][j] = 0;
    }
    if (tty_screen_full)
    {
        scan_baseline = (scan_baseline + 1) & 31;
        *(volatile int *)TTY_SCAN_BASELINE = scan_baseline;
    }
    CUR_COL = 0;
}
void putchark(int c)
{
    if (c == '\n')
    {
        newline();
    }
    else if (c == 0x08)
    {
        CUR_COL--;
        if (CUR_COL < 0)
        {
            tty_screen_full = 0;
            if (CUR_LINE > 0)
            {
                CUR_LINE--;
                CUR_COL = 79;
            }
            else
            {
                CUR_COL = 0;
            }
        }
        stdout_buffer[CUR_LINE][CUR_COL] = ' ';
    }
    else
    {
        if (CUR_COL == 80)
        {
            newline();
        }
        stdout_buffer[CUR_LINE][CUR_COL++] = c;
    }
}
void printk(const char *s)
{
    int i = 0;
    while (s[i] != '\0')
    {
        putchark(s[i]);
        i++;
    }
}