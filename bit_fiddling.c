#include <stdio.h>
#include <string.h>

int endian(int x){
    int z = x;
    int y = z & 0xFF;
    y = y << 8;
    z = z >> 8;
    y = y ^ z & 0xFF;
    y = y << 8;
    z = z >> 8;
    y = y ^ z & 0xFF;
    y = y << 8;
    z = z >> 8;
    y = y ^ z & 0xFF;
    return y;
}

int bitcount(int x){
    int z = x;
    z = (z & 0x55555555) + ((z >> 1) & 0x55555555);
    z = (z & 0x33333333) + ((z >> 2) & 0x33333333);
    z = (z & 0x0f0f0f0f) + ((z >> 4) & 0x0f0f0f0f);
    z = (z & 0x00ff00ff) + ((z >> 8) & 0x00ff00ff);
    z = (z & 0x0000ffff) + ((z >> 16)& 0x0000ffff);
    return z;
}

int getbits(int x, int y, int z){
    unsigned mask = ((1 << (z-y))-1) << y;
    return (x & mask) >> y;
}

int anybit(int x){
    int y = x;
    y = 1 ^ ((y | (~y + 1))>>31) + 1;
    return y;
}

int reverse(int x){
    int y = x;
    // y = (y & 0xFFFF0000) >> 16 |(y & 0x0000FFFF) << 16;
    // y = (y & 0xFF00FF00) >> 8 | (y & 0x00FF00FF) << 8;
    // y = (y & 0xF0F0F0F0) >> 4 | (y & 0x0F0F0F0F) << 4;
    // y = (y & 0xCCCCCCCC) >> 2 | (y & 0x33333333) << 2;
    // y = (y & 0xAAAAAAAA) >> 1 | (y & 0x55555555) << 1;
    
    y = (y & 0xFFFF0000) >> 16 | (y & 0x0000FFFF) << 16;
    y = ((y & 0xFF00FF00) >> 8) & 0x00FFFFFF | (y & 0x00FF00FF) << 8;
    y = ((y & 0xF0F0F0F0) >> 4) & 0x0FFFFFFF | (y & 0x0F0F0F0F) << 4;
    y = ((y & 0xCCCCCCCC) >> 2) & 0x3FFFFFFF | (y & 0x33333333) << 2;
    y = ((y & 0xAAAAAAAA) >> 1) & 0x7FFFFFFF | (y & 0x55555555) << 1;
    return y;
}

int main(){
    // int x = -1;
    // printf("%x\n\n", endian(x));

    // x = 0;
    // printf("%d\n\n", bitcount(x));

    int x = 0x058;
    printf("%x\n\n", getbits(x, 0, 4));

    // x = 0;
    // printf("%x\n", anybit(x));
    // x = 0x57;
    // printf("%x\n\n", anybit(x));

    // x = 0x9E3779B8;
    // printf("%x\n", x);
    // printf("%x\n", reverse(x));

    // printf("%x\n", (int)(((long)0xFFFF0000) >> 16));
    
    return 0;
}
