int findComplement(int num) {
    unsigned int mask = 0;
    unsigned int n = num;

    while (n) {
        mask = (mask << 1) | 1;
        n >>= 1;
    }

    return (~num) & mask;
}