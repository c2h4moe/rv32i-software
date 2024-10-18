unsigned int __mulsi3(unsigned int a, unsigned int b) {
    unsigned int res = 0;
    while (a) {
        if (a & 1) res += b;
        a >>= 1;
        b <<= 1;
    }
    return res;
}