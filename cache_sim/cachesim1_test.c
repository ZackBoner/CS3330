#include <stdio.h>
#include <stdarg.h>
#include "cachesim1.h"

u8 block_bits, way_bits, sets_bits, address_bits;

int errmsg(int idx, const char *msg, ...) {
    // static u64 mask = 0;
    // if (mask & (1<<idx)) return 0; // already reported one of these errors
    // mask |= (1<<idx);
    
    va_list myargs;
    va_start(myargs, msg);
    int ret = vprintf(msg, myargs);
    va_end(myargs);
    return ret;
}

typedef struct {
    u8 bb, wb, sb, ab;
    u64 addr; u32 off, idx; u64 tag;
    u32 lru1, idx1, idx2, lru2;
} tcase;

int runTest(tcase *test) {
    block_bits = test->bb;
    sets_bits = test->sb;
    way_bits = test->wb;
    address_bits = test->ab;
    int correct = 0;

    // check the globals
    global_init();
    
    // check parts of address
    if (get_offset(test->addr) == test->off) correct += 1;
    else errmsg(0, "Block Offset wrong (e.g. for 0x%llx it should be 0x%x not 0x%x)\n", test->addr, test->off, get_offset(test->addr));
    if (get_index(test->addr) == test->idx) correct += 1;
    else errmsg(1, "Set Index wrong (e.g. for 0x%llx it should be 0x%x not 0x%x)\n", test->addr, test->idx, get_index(test->addr));
    if (get_tag(test->addr) == test->tag) correct += 1;
    else errmsg(2, "Tag wrong (e.g. for 0x%llx it should be 0x%llx not 0x%llx)\n", test->addr, test->tag, get_tag(test->addr));
    
    // check LRU processing
    if (idxFromLRU(test->lru1) == test->idx1) correct += 1;
    else errmsg(3, "Wrong psuedo-LRU line number (e.g. for 0x%x it should be 0x%x not 0x%x)\n", test->lru1, test->idx1, idxFromLRU(test->lru1));
    if (newLRU(test->lru1, test->idx2) == test->lru2) correct += 1;
    else errmsg(4, "Wrong update to LRU (e.g. accessing line 0x%x with PLRU 0x%x should make PRLU 0x%x not 0x%x)\n", test->idx2, test->lru1, test->lru2, newLRU(test->lru1, test->idx2));
    
    return correct;
}

tcase tests[] = {
    {6,3,8,48, 0x0,0x0,0x0,0x0, 0,0,7,0},
    {6,3,8,48, 0xFFFFFFFFFFFFFFFF,0x3F,0xFF,0x3FFFFFFFF, 0,0,0,0xB},
    {8,3,6,48, 0x0123456789ABCDEF,0xEF,0xD,0x1159e26af, 0xB,4,0,0xB},
    {6,3,6,48, 0x0123456789ABCDEF,0x2F,0x37,0x456789abc, 0xB,4,1,0x3},
    {6,3,6,64, 0x0123456789ABCDEF,0x2F,0x37,0x123456789abc, 0x0,0,7,0x0},
    {0,3,0,48, 0x0123456789ABCDEF,0x0,0x0,0x456789abcdef, 0x0,0,6,0x40},
    {8,3,0,32, 0x0123456789ABCDEF,0xef,0x0,0x89abcd, 0x40,0,5,0x44},
    {8,3,20,48, 0x1abe1edadebac1e,0x1e,0xdebac,0xe1eda, 0x44,0,4,0x64},
    {1,3,12,30, 0x15bd5be1d,0x1,0xf0e,0xdead, 0x64,0,3,0x65},
    {0,3,8,55, 0x1abe1edf00d,0,0xd,0x1abe1edf0, 0x65,7,2,0x75},
    {1,3,5,48, 0xb01dface,0,0x7,0x2c077eb, 0x75,7,1,0x77},
    {2,3,2,4, 0xba11ad0fb100d,0x1,0x3,0x0, 0x77,7,0,0x7F},
    {3,2,10,24, 0xacc01ade5,0x5,0x1bc,0xd, 0x7,3,1,0x5},
    {4,2,7,34, 0x5eaf00d,0xd,0,0xbd5e, 0x6,1,1,0x5},
    {5,2,4,44, 0xcea5e1e55,0x15,0x2,0x6752f0f, 0x5,3,1,0x5},
    {6,2,1,54, 0xa1fa1fab10b,0xb,0,0x143f43f562, 0x4,0,1,0x5},
    {7,2,9,60, 0xce110c1a55,0x55,0x34,0xce110c, 0x3,2,1,0x1},
    {8,2,6,50, 0xdeca10f5e1f1e55,0x55,0x1e,0x2843d787c, 0x2,1,1,0x1},
    {9,2,3,21, 0x5a1e5a1e5a1e5a1e,0x1e,0x5,0x1e5, 0x1,2,1,0x1},
    {10,2,0,22, 0x8888888888888888,0x88,0,0x222, 0x0,0,1,0x1},
};

int main(int argc, char *argv[]) {
    int correct = 0;
    int outof = 0;
    for(int i=0; i<sizeof(tests)/sizeof(tcase); i+=1) {
        outof += 5;
        correct += runTest(&tests[i]);
    }
    printf("Passed %d / %d tests\n", correct, outof);
}
