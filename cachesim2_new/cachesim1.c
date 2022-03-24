#include "cachesim1.h"

// You will have access to the following globals:
// u8 block_bits, way_bits, sets_bits, address_bits;

/// Called once after each change to the defining extern values above.
/// we won't test it, but you can use it to set globals like `tag_bits` if you wish.
void global_init() {
    // optionally, add code here
}

/// update Tree-PLRU tracking bits based on an access of the given line
u16 newLRU(u16 oldLRU, size_t index) {
    return 0; // fix me
}

/// Given Tree-PLRU tracking bits, return index of least-recently-used line
size_t idxFromLRU(u16 lru) {
    return 0; // fix me
}

/// Given an address, return the block offset from it
size_t get_offset(u64 address) {
    return 0; // fix me
}
/// Given an address, return the set index from it
size_t get_index(u64 address) {
    return 0; // fix me
}
/// Given an address, return the tag from it
u64 get_tag(u64 address) {
    return 0; // fix me
}
