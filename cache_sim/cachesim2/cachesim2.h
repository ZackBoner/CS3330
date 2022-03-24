#include "cachesim1.h"


/**
 * A packed structure for the live and dirty bits and the tag.
 * The __attribute__((packed)) tells most compilers (including GCC and Clang)
 * lets us have smaller-than-a-byte values. `live` and `dirty` are single-bit
 * values, so `metadata.dirty = x` will set actually do `metadata.dirty = x&1`.
 * By making the tag 62 bits, the entire structure fits in 64 bits = 8 bytes.
 * 
 * Warning: don't take addresses of packed struct fields; all pointers are
 * byte-aligned and the fields of this struct are not.
 */
typedef struct __attribute__((packed)) {
    u8 live:1;
    u8 dirty:1;
    u64 tag:62;
} LineMetadata;

/**
 * A line has metadata and a block.
 * 
 * We don't store this structure, just use it as a return value, which is why
 * its fields are pointers not values.
 */
typedef struct {
    LineMetadata *meta;
    u8 *block;
} Line;

/**
 * To be able to store caches of multiple sizes using the same structure,
 * we store three separate arrays under the hood:
 * plru[sets], _line_md[sets*way], and _blocks[sets*way*blocksize].
 * 
 * Initialize using `makeCache` and free using the `freeCache`, both implemented
 * for you in cachesim2_lib.c.
 * 
 * Access PLRU tracking info using mycache.plru[setindex]
 * Access lines using getline(&mycache, setindex, linenumber)
 * 
 * e.g. to see if the LRU line of set 3 matches tag `x`, you'd do something like
 * 
 * if (getLine(&mycache, 3, idxFromLRU(mycache.plru[3])).meta->tag == x) {...}
 */
typedef struct _Cache_s {
    u8 sets_bits;             // cache has pow(2,sets_bits) sets
    u8 way_bits;              // each set is pow(2,way_bits) lines; way_bits <= 4
    u8 block_bits;            // each line is pow(2, block_bits) bytes
    u16 *plru;                // plru[i] is the LRU-tracking data for set i
    struct _Cache_s *backing; // on a miss, go here; if NULL, go to RAM
    u8 isWriteback;           // 1 if writeback, 0 if writethrough
    LineMetadata *_line_md;
    u8 *_blocks;
} Cache;


/**
 * The principle method for interacting with a cache.
 * `getLine(&mycache, setindex, linenumber)` returns a Line structure.
 * 
 * This function is implemented in cachesim2_lib.c
 */
Line getLine(Cache *c, size_t setindex, size_t linenumber);

/**
 * Allocates a cache inside the provided structure. Initially set to
 * writethrough with NULL (i.e., RAM) as the backing cache.
 * 
 * ----
 * Cache l1; // a 4-way set-associative cache with 32 sets and 16-byte blocks
 * makeCache(&l1, 32, 4, 16);
 * l1.isWriteback = true;
 * l1.backing = &l2; // another cache allocated earlier
 * 
 * // use the l1 cache here
 * 
 * freeCache(&l1);
 * ----
 * 
 * This function is implemented in cachesim2_lib.c
 */
void makeCache(Cache *answer, u8 sets_bits, u8 way_bits, u8 block_bits);

/**
 * Release the memory allocated by makeCache. Do not try to use the cache after
 * freeing it.
 * 
 * This function is implemented in cachesim2_lib.c
 */
void freeCache(Cache *c);

/**
 * An abstraction of RAM, used as a testing stub.
 * Implemented in cachesim2_test.c
 */
void writeToRAM(u64 address, size_t bytes, const u8* data);

/**
 * An abstraction of RAM, used as a testing stub. Modifies memory data[0]
 * through data[bytes-1]. Implemented in cachesim2_test.c
 */
void readFromRAM(u64 address, size_t bytes, u8* data);



/**
 * Return a byte from the cache if possible. If not, read the block the byte
 * would be part of from the backing cache (or RAM) into the cache, replacing the
 * pseudo-least-recently-used line in the set. Whether it was already in the
 * cache or not, update the Tree-PLRU tracking bits to make the line with this
 * byte in it the most-recently-used in its set. If the cache is writeback
 * and you evict a dirty line, remember to write it to the backing cache first.
 */
u8 getByte(Cache *c, u64 address);

/**
 * Write to a byte in the cache. If the byte is not in the cache, first fetch it
 * as you would for `getByte`. If the cache is writethrough, also send the write
 * (in a full block, not the single byte) to the backing cache. In all cases,
 * update the Tree-PLRU tracking bits to make the line with this byte in it the
 * most-recently-used in its set.
 */
void setByte(Cache *c, u64 address, u8 value);

/**
 * Write to a block consisting of bytes src[0] through src[bytes-1] to the cache.
 * You may assume that all of the bytes are in the same block of the cache,
 * through they may be a subset of the full block. Handle all the updating and
 * so on as you would for `setByte`.
 */
void setBlock(Cache *c, u64 address, const u8 *src, size_t bytes);

/**
 * Read `bytes` from a block inro memory pointed to by `dst`.
 * You may assume that all of the bytes are in the same block of the cache,
 * through they may be a subset of the full block. Handle all the updating and
 * so on as you would for `getByte`.
 */
void getBlock(Cache *c, u64 address, u8 *dst, size_t bytes);

/**
 * Read 8 bytes as a little-endian unsigned integer, similar to `getBlock` except
 * that the 8-byte value might spans two blocks, in which case you'd do full accesses
 * on both lines, including access to backing cache and update the PLRU.
 * 
 * You may assume that `c.block_size` >= 8.
 */
u64 getLong(Cache *c, u64 address);

/**
 * Read 8 bytes as a little-endian unsigned integer, similar to `setBlock` except
 * that the 8-byte value might spans two blocks, in which case you'd do full accesses
 * on both lines, including access to backing cache and update the PLRU.
 * 
 * You may assume that `c.block_size` >= 8.
 */
void setLong(Cache *c, u64 address, u64 value);
