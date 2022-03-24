#include "cachesim2.h"

/**
 * Return a byte from the cache if possible. If not, read the block the byte
 * would be part of from the backing cache (or RAM) into the cache, replacing the
 * pseudo-least-recently-used line in the set. Whether it was already in the
 * cache or not, update the Tree-PLRU tracking bits to make the line with this
 * byte in it the most-recently-used in its set. If the cache is writeback
 * and you evict a dirty line, remember to write it to the backing cache first.
 */
u8 getByte(Cache *c, u64 address) {
    u8 value; readFromRAM(address, sizeof(u8), &value); return value; // FIX ME
}

/**
 * Write to a byte in the cache. If the byte is not in the cache, first fetch it
 * as you would for `getByte`. If the cache is writethrough, also send the write
 * (in a full block, not the single byte) to the backing cache. In all cases,
 * update the Tree-PLRU tracking bits to make the line with this byte in it the
 * most-recently-used in its set.
 */
void setByte(Cache *c, u64 address, u8 value) {
    writeToRAM(address, sizeof(u8), &value); // FIX ME
}

/**
 * Write to a block consisting of bytes src[0] through src[bytes-1] to the cache.
 * You may assume that all of the bytes are in the same block of the cache,
 * through they may be a subset of the full block. Handle all the updating and
 * so on as you would for `setByte`.
 */
void setBlock(Cache *c, u64 address, const u8 *src, size_t bytes) {
    writeToRAM(address, bytes, src); // FIX ME
}

/**
 * Read `bytes` from a block into memory pointed to by `dst`.
 * You may assume that all of the bytes are in the same block of the cache,
 * through they may be a subset of the full block. Handle all the updating and
 * so on as you would for `getByte`.
 */
void getBlock(Cache *c, u64 address, u8 *dst, size_t bytes) {
    readFromRAM(address, bytes, dst); // FIX ME
}

/**
 * Read 8 bytes as a little-endian unsigned integer, similar to `getBlock` except
 * that the 8-byte value might spans two blocks, in which case you'd do full accesses
 * on both lines, including access to backing cache and update the PLRU.
 * 
 * You may assume that `c.block_size` >= 8.
 */
u64 getLong(Cache *c, u64 address) {
    u64 value; readFromRAM(address, sizeof(value), (u8*)&value); return value; // FIX ME
}

/**
 * Read 8 bytes as a little-endian unsigned integer, similar to `setBlock` except
 * that the 8-byte value might spans two blocks, in which case you'd do full accesses
 * on both lines, including access to backing cache and update the PLRU.
 * 
 * You may assume that `c.block_size` >= 8.
 */
void setLong(Cache *c, u64 address, u64 value) {
    writeToRAM(address, sizeof(value), (u8*)&value); // FIX ME
}
