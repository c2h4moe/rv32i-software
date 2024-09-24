#include<basicio.h>
int main(){
    printk("There is no magic in computer world.\n");
    while(1){
        int c = getchark();
        if(c == -1){
            continue;
        }
        putchark(c);
    }
}