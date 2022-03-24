#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t size_t;

/// These will be defined in the tester file
extern u8 block_bits, way_bits, sets_bits, address_bits;

/// Called once after each change to the defining extern values above.
/// we won't test it, but you can use it to set globals like `tag_bits` if you wish.
void global_init();

/// update Tree-PLRU tracking bits based on an access of the given line
u16 newLRU(u16 oldLRU, size_t index);

/// Given Tree-PLRU tracking bits, return index of least-recently-used line
size_t idxFromLRU(u16 lru);

/// Given an address, return the block offset from it
size_t get_offset(u64 address);

/// Given an address, return the set index from it
size_t get_index(u64 address);

/// Given an address, return the tag from it
u64 get_tag(u64 address);
