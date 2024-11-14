int main()
{
    
    while(1) {
        char a = *(volatile char *)(0xbad << 20);
        if(a != 0) {
            break;
        }
    }
    asm volatile("ebreak");
    return 0;
}