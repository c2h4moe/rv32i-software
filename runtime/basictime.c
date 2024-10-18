#define RTC_SPACE (0x500000)
#define MS_TO_CLKCYCLES (25000)
unsigned int time(){
    return (*((volatile unsigned int*)RTC_SPACE));
}

#ifndef SIM_MODE
void sleep(int n){
    int cycles = n * (MS_TO_CLKCYCLES >> 1);
    int i = 0;
    asm volatile(
        "lp:\n"
        "addi %[i], %[i], 1\n"
        "bge %[cyc], %[i], lp\n"
        : [i] "+r" (i)
        : [cyc] "r" (cycles)
    );
}
#else
void sleep(int n){
    (*(volatile unsigned int*)RTC_SPACE) = n;
}
#endif