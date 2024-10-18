#include "random.h"
#include "basictime.h"
#include "basicio.h"
void toString(int n, char* buf){
    int tmp = n;
    int bits = 0;
    while(tmp){
        tmp /= 10;
        bits ++;
    }
    while(n){
        int d = n % 10;
        buf[bits - 1] = d + '0';
        bits --;
        n /= 10;
    }
}
int main(){
    char s[15];
    while(1){
        int n = rand();
        for(int i = 0; i < 15; i ++){
            s[i] = 0;
        }
        toString(n, s);
        printk(s);
        printk("\n");
        sleep(500);
    }
}