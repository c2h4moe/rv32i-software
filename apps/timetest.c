#include "basictime.h"
#include "basicio.h"

int main(){
    printk("now start sleep for 1 sec...\n");
    sleep(1000);
    printk("wake up!\n");
    printk("now start sleep for 2 sec...\n");
    sleep(2000);
    printk("wake up!\n");
    printk("now start sleep for 3 sec...\n");
    sleep(3000);
    printk("wake up!\n");
}