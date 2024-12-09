void foo(){
	volatile int a;
	for(a=0;a<1;a++){
		a++;
	}
	return;
}
int main(){
    foo();
    asm volatile("ebreak");
}