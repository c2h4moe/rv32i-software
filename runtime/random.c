static int cur = 114514;
static const int A = 1103515245; // parameter from glibc
static const int C = 12345; // parameter from glibc

int rand(){
    cur = (A * cur + C) & 0x7fffffff;
    return cur;
}

void seed(int n){
    cur = n;
}