#define SERIAL_PORT 0xbee00000
static void serial_putchar(char c) {
    *(volatile char*)(SERIAL_PORT) = c;
}
void output(const char* s) {
    while(*s) {
        serial_putchar(*s);
        s ++;
    }
}