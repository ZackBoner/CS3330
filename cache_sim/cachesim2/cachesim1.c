#include "cachesim1.h"
#include <stdio.h>

// You will have access to the following globals:
// u8 block_bits, way_bits, sets_bits, address_bits;

/// Called once after each change to the defining extern values above.
/// we won't test it, but you can use it to set globals like `tag_bits` if you wish.
void global_init() {
    // optionally, add code here
}

// 
u64 getbits(u64 x, u8 y, u8 z){
    u64 mask = 1;
    mask = (mask << (z-y)) - 1;
    return mask & (x >> y);
}

/// update Tree-PLRU tracking bits based on an access of the given line
u16 newLRU(u16 oldLRU, size_t index) {
    u16 newLRU = oldLRU;
    size_t loc = 0;
    size_t line_bit = 0;
    for(int i = 0; i < way_bits; i++){
        // get the i'th bit of the line
        line_bit = (index >> (way_bits-i-1)) & 1;
        // set the current node to the opposite of line_bit
        // left side of expression clears, right side sets
        newLRU = (newLRU & ~(1 << loc)) | (!line_bit << loc);
        // traverse the tree
        loc = 2*loc + 1 + line_bit;
    }    
    return newLRU; // fix me
}

/// Given Tree-PLRU tracking bits, return index of least-recently-used line
size_t idxFromLRU(u16 lru) {
    size_t line = 0;
    int bit = 0;
    int loc = 0;
    for(int i = 0; i < way_bits; i++){
        // get the node at loc in the tree
        bit = (lru >> loc) & 1;
        // update our line
        line = 2*line + bit;
        // traverse the tree
        loc = 2*loc + 1 + bit;
    }
    return line; // fix me
}

/// Given an address, return the block offset from it
size_t get_offset(u64 address) {
    size_t block_offset = getbits(address, 0, block_bits);
    return block_offset; // fix me
}
/// Given an address, return the set index from it
size_t get_index(u64 address) {
    size_t set_index = getbits(address, block_bits, block_bits+sets_bits);
    return set_index; // fix me
}
/// Given an address, return the tag from it
u64 get_tag(u64 address) {
    u64 tag = getbits(address, block_bits+sets_bits, address_bits);
    return tag; // fix me
}
