#include "cachesim2.h"
#include <stdlib.h>

Line getLine(Cache *c, size_t setindex, size_t linenumber) {
    Line ans;
    ans.meta = c->_line_md + ((setindex << c->way_bits) + linenumber);
    ans.block = c->_blocks + (((setindex << c->way_bits) + linenumber) << c->block_bits);
    return ans;
}

void makeCache(Cache *answer, u8 sets_bits, u8 way_bits, u8 block_bits) {
    answer->block_bits = block_bits;
    answer->sets_bits = sets_bits;
    answer->way_bits = way_bits;
    answer->plru = calloc(1<<sets_bits, sizeof(u16));
    answer->isWriteback = 0;
    answer->backing = 0;
    answer->_line_md = calloc(1<<(sets_bits+way_bits), sizeof(LineMetadata));
    answer->_blocks = calloc(1<<(sets_bits+way_bits+block_bits), sizeof(u8));
}

void freeCache(Cache *c) {
    if (c->plru) {
        free(c->plru); c->plru = 0; 
        free(c->_line_md); c->_line_md = 0; 
        free(c->_blocks); c->_blocks = 0; 
    }
}

