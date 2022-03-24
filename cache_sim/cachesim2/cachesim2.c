#include "cachesim2.h"
#include <stdio.h>

/**
 * Return a byte from the cache if possible. If not, read the block the byte
 * would be part of from the backing cache (or RAM) into the cache, replacing the
 * pseudo-least-recently-used line in the set. Whether it was already in the
 * cache or not, update the Tree-PLRU tracking bits to make the line with this
 * byte in it the most-recently-used in its set. If the cache is writeback
 * and you evict a dirty line, remember to write it to the backing cache first.
 */
u8 getByte(Cache *c, u64 address) {
    u8 value; 

    size_t tag = get_tag(address);
    size_t set_idx = get_index(address);
    size_t block_off = get_offset(address);

    // search for the address in the cache
    for(int i = 0; i < (1uL << way_bits); i++){
        Line line = getLine(c, set_idx, i);

        // CACHE HIT
        if(line.meta->live && line.meta->tag == tag){
            value = line.block[block_off];
            c->plru[set_idx] = newLRU(c->plru[set_idx], i);
            return value;
        }
    }

    // CACHE MISS

    // get the cache line we'll be replacing
    size_t set_line = idxFromLRU(c->plru[set_idx]);
    Line line = getLine(c, set_idx, set_line);

    // evict
    if(c->isWriteback && line.meta->dirty){
        u64 addr = ((line.meta->tag << sets_bits) | set_idx);
        writeToRAM(addr << block_bits, 1uL << block_bits , line.block);
        line.meta->dirty = 0;
    }
    readFromRAM((address >> block_bits) << block_bits, 1uL << block_bits, line.block);
    line.meta->live = 1;
    line.meta->tag = tag;

    value = line.block[block_off];
    c->plru[set_idx] = newLRU(c->plru[set_idx], set_line);
    return value;
}

/**
 * Write to a byte in the cache. If the byte is not in the cache, first fetch it
 * as you would for `getByte`. If the cache is writethrough, also send the write
 * (in a full block, not the single byte) to the backing cache. In all cases,
 * update the Tree-PLRU tracking bits to make the line with this byte in it the
 * most-recently-used in its set.
 */
void setByte(Cache *c, u64 address, u8 value) {
    size_t tag = get_tag(address);
    size_t set_idx = get_index(address);
    size_t block_off = get_offset(address);
    Line line;

    // search for the address in the cache
    u8 hit = 0;
    for(int i = 0; i < (1uL << way_bits); i++){
        line = getLine(c, set_idx, i);

        // CACHE HIT
        if(line.meta->live && line.meta->tag == tag){
            line.block[block_off] = value;
            c->plru[set_idx] = newLRU(c->plru[set_idx], i);
            hit = 1;
            break;
        }
    }

    if(!hit){
        size_t set_line = idxFromLRU(c->plru[set_idx]);
        line = getLine(c, set_idx, set_line);

        // evict
        if(c->isWriteback && line.meta->dirty){
            u64 addr = ((line.meta->tag << sets_bits) | set_idx);
            writeToRAM(addr << block_bits, 1uL << block_bits , line.block);
        }
        readFromRAM((address >> block_bits) << block_bits, 1uL << block_bits, line.block);
        line.block[block_off] = value;
        line.meta->live = 1;
        line.meta->tag = tag;
        
        c->plru[set_idx] = newLRU(c->plru[set_idx], set_line);
    }

    if(!c->isWriteback){
        writeToRAM((address >> block_bits) << block_bits, 1uL << block_bits, line.block);
    }
    else{
        line.meta->dirty = 1;
    }
}

/**
 * Write to a block consisting of bytes src[0] through src[bytes-1] to the cache.
 * You may assume that all of the bytes are in the same block of the cache,
 * through they may be a subset of the full block. Handle all the updating and
 * so on as you would for `setByte`.
 */
void setBlock(Cache *c, u64 address, const u8 *src, size_t bytes) {
    size_t tag = get_tag(address);
    size_t set_idx = get_index(address);
    size_t block_off = get_offset(address);
    Line line;

    // search for the address in the cache
    u8 hit = 0;
    for(int i = 0; i < (1uL << way_bits); i++){
        line = getLine(c, set_idx, i);

        // CACHE HIT
        if(line.meta->live && line.meta->tag == tag){
            for(int j = 0; j < bytes; j++){
                line.block[block_off+j] = src[j];
            }
            c->plru[set_idx] = newLRU(c->plru[set_idx], i);
            hit = 1;
            break;
        }
    }

    if(!hit){
        size_t set_line = idxFromLRU(c->plru[set_idx]);
        line = getLine(c, set_idx, set_line);

        // evict
        if(c->isWriteback && line.meta->dirty){
            u64 addr = ((line.meta->tag << sets_bits) | set_idx);
            writeToRAM(addr << block_bits, 1uL << block_bits , line.block);
        }

        // read block into cache
        readFromRAM((address >> block_bits) << block_bits, 1uL << block_bits, line.block);
        line.meta->live = 1;
        line.meta->tag = tag;

        // set portion of block to the contents of src
        for(int i =0; i < bytes; i++){
            line.block[block_off+i] = src[i];
        }
        
        c->plru[set_idx] = newLRU(c->plru[set_idx], set_line);
    }

    if(!c->isWriteback){
        writeToRAM((address >> block_bits) << block_bits, 1uL << block_bits, line.block);
    }
    else{
        line.meta->dirty = 1;
    }
}

/**
 * Read `bytes` from a block into memory pointed to by `dst`.
 * You may assume that all of the bytes are in the same block of the cache,
 * through they may be a subset of the full block. Handle all the updating and
 * so on as you would for `getByte`.
 */
void getBlock(Cache *c, u64 address, u8 *dst, size_t bytes) {
    size_t tag = get_tag(address);
    size_t set_idx = get_index(address);
    size_t block_off = get_offset(address);

    // search for the address in the cache
    for(int i = 0; i < (1uL << way_bits); i++){
        Line line = getLine(c, set_idx, i);

        // CACHE HIT
        if(line.meta->live && line.meta->tag == tag){
            // value = line.block[block_off];
            for(int j = 0; j < bytes; j++){
                dst[j] = line.block[block_off + j];
            }
            c->plru[set_idx] = newLRU(c->plru[set_idx], i);
            return;
        }
    }

    // CACHE MISS

    // get the cache line we'll be replacing
    size_t set_line = idxFromLRU(c->plru[set_idx]);
    Line line = getLine(c, set_idx, set_line);

    // evict
    if(c->isWriteback && line.meta->dirty){
        u64 addr = ((line.meta->tag << sets_bits) | set_idx);
        writeToRAM(addr << block_bits, 1uL << block_bits , line.block);
        line.meta->dirty = 0;
    }

    // read block into cache
    readFromRAM((address >> block_bits) << block_bits, 1uL << block_bits, line.block);
    line.meta->live = 1;
    line.meta->tag = tag;

    // write contents of block to dst
    for(int i = 0; i < bytes; i++){
        dst[i] = line.block[block_off + i];
    }

    c->plru[set_idx] = newLRU(c->plru[set_idx], set_line);
}

/**
 * Read 8 bytes as a little-endian unsigned integer, similar to `getBlock` except
 * that the 8-byte value might spans two blocks, in which case you'd do full accesses
 * on both lines, including access to backing cache and update the PLRU.
 * 
 * You may assume that `c.block_size` >= 8.
 */
u64 getLong(Cache *c, u64 address) {
    u64 value = 0; 
    //readFromRAM(address, sizeof(value), (u8*)&value); return value; // FIX ME

    size_t block_off = get_offset(address);

    // if we have to read two lines
    if(block_off + 8 > (1uL << block_bits)){
        size_t bytes_in_first = (1 << block_bits) - block_off;
        size_t bytes_in_second = 8-bytes_in_first;

        u8 block1[8];
        u8 block2[8];

        u64 address2 = ((address >> block_bits) << block_bits) + (1 << block_bits);

        getBlock(c, address, block1, bytes_in_first);
        getBlock(c, address2, block2, bytes_in_second);

        for(int i = 0; i < bytes_in_first; i++){
            // block2[i] = (value << 8*i) & 0xFF;
            value = (value << 8) | block1[i];
        }
        for(int i = 0; i < bytes_in_second; i++){
            // block1[i] = (value << 8*(i+bytes_in_second)) & 0xFF;
            value = (value << 8) | block2[i];
        }
    }
    else{
        u8 block[8];
        getBlock(c, address, block, 8);
        for(int i = 7; i >= 0; i--){
            value = (value << 8) | block[i];
        }
    }
    return value;
}

/**
 * Read 8 bytes as a little-endian unsigned integer, similar to `setBlock` except
 * that the 8-byte value might spans two blocks, in which case you'd do full accesses
 * on both lines, including access to backing cache and update the PLRU.
 * 
 * You may assume that `c.block_size` >= 8.
 */
void setLong(Cache *c, u64 address, u64 value) {
    size_t block_off = get_offset(address);

    // if we have to write to two lines
    if(block_off + 8 > (1uL << block_bits)){
        size_t bytes_in_first = (1 << block_bits) - block_off;
        size_t bytes_in_second = 8-bytes_in_first;

        u8 block1[8];
        u8 block2[8];

        u64 address2 = ((address >> block_bits) << block_bits) + (1 << block_bits);
        
        for(int i = 0; i < bytes_in_second; i++){
            block2[i] = (value << 8*i) & 0xFF;
        }
        for(int i = 0; i < bytes_in_first; i++){
            block1[i] = (value << 8*(i+bytes_in_second)) & 0xFF;
        }

        setBlock(c, address, block1, bytes_in_first);
        setBlock(c, address2, block2, bytes_in_second);
    }
    else{
        u8 block[8];
        for(int i = 7; i >= 0; i--){
            block[i] = (value >> 8*i) & 0xFF;
        }
        setBlock(c, address, block, 8);
    }
    // writeToRAM(address, sizeof(value), (u8*)&value); // FIX ME
}
