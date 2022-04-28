#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <smmintrin.h>
#include <immintrin.h>

/*
 * Please fill in the following struct with your name and the name you'd like to appear on the scoreboard
 */
who_t who = {
    "ZackB",           /* Scoreboard name */

    "Zachery Boner",   /* Full name */
    "zwb6kg@virginia.edu",  /* Email address */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/
char unroll_4_rotate_descr[] = "unroll_4_rotate: loop j then i and unroll j by 4";
void unroll_4_rotate(int dim, pixel *src, pixel *dst) {
  for (int j = 0; j+3 < dim; j+=4) {
      for (int i = 0; i < dim; i++) {
          dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
          dst[RIDX(dim-1-j-1, i, dim)] = src[RIDX(i, j+1, dim)];
          dst[RIDX(dim-1-j-2, i, dim)] = src[RIDX(i, j+2, dim)];
          dst[RIDX(dim-1-j-3, i, dim)] = src[RIDX(i, j+3, dim)];
      }
  }
}

char unroll_4i_rotate_descr[] = "unroll_4i_rotate: loop j then i and unroll j by 4";
void unroll_4i_rotate(int dim, pixel *src, pixel *dst) {
  for (int j = 0; j < dim; j++) {
      for (int i = 0; i+3 < dim; i+=4) {
          dst[RIDX(dim-1-j, i+0, dim)] = src[RIDX(i+0, j, dim)];
          dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
          dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
          dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];
      }
  }
}

char unroll_8_rotate_descr[] = "unroll_8_rotate: loop j then i and unroll j by 8";
void unroll_8_rotate(int dim, pixel *src, pixel *dst) {
     for (int j = 0; j+7 < dim; j+=8) {
         for (int i = 0; i < dim; i++) {
             dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
             dst[RIDX(dim-1-j-1, i, dim)] = src[RIDX(i, j+1, dim)];
             dst[RIDX(dim-1-j-2, i, dim)] = src[RIDX(i, j+2, dim)];
             dst[RIDX(dim-1-j-3, i, dim)] = src[RIDX(i, j+3, dim)];
             dst[RIDX(dim-1-j-4, i, dim)] = src[RIDX(i, j+4, dim)];
             dst[RIDX(dim-1-j-5, i, dim)] = src[RIDX(i, j+5, dim)];
             dst[RIDX(dim-1-j-6, i, dim)] = src[RIDX(i, j+6, dim)];
             dst[RIDX(dim-1-j-7, i, dim)] = src[RIDX(i, j+7, dim)];
         }
     }
 }

char unroll_4_block_4_rotate_descr[] = "unroll_4_block_4_rotate: unroll by 4 and block by 4";
void unroll_4_block_4_rotate(int dim, pixel *src, pixel *dst)
 {
     for (int ii = 0; ii < dim; ii+=4) {
         for (int j = 0; j+3 < dim; j+=4) {
           for (int i = ii; i < ii+4; i++) {
               dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
                dst[RIDX(dim-1-j-1, i, dim)] = src[RIDX(i, j+1, dim)];
                dst[RIDX(dim-1-j-2, i, dim)] = src[RIDX(i, j+2, dim)];
                dst[RIDX(dim-1-j-3, i, dim)] = src[RIDX(i, j+3, dim)];
             }
         }
     }
 }

char unroll_4_block_16_rotate_descr[] = "unroll_4_block_16_rotate: unroll by 4 and block by 16";
void unroll_4_block_16_rotate(int dim, pixel *src, pixel *dst)
 {
     for (int ii = 0; ii < dim; ii+=16) {
         for (int j = 0; j+3 < dim; j+=4) {
         	for (int i = ii; i < ii+16; i++) {
         	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
                dst[RIDX(dim-1-j-1, i, dim)] = src[RIDX(i, j+1, dim)];
                dst[RIDX(dim-1-j-2, i, dim)] = src[RIDX(i, j+2, dim)];
                dst[RIDX(dim-1-j-3, i, dim)] = src[RIDX(i, j+3, dim)];
             }
         }
     }
 }

char unroll_8_block_16_rotate_descr[] = "unroll_8_block_16_rotate: unroll by 8 and block by 16";
void unroll_8_block_16_rotate(int dim, pixel *src, pixel *dst)
 {
     for (int ii = 0; ii < dim; ii+=16) {
         for (int j = 0; j+7 < dim; j+=8) {
         	for (int i = ii; i < ii+16; i++) {
                dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
                dst[RIDX(dim-1-j-1, i, dim)] = src[RIDX(i, j+1, dim)];
                dst[RIDX(dim-1-j-2, i, dim)] = src[RIDX(i, j+2, dim)];
                dst[RIDX(dim-1-j-3, i, dim)] = src[RIDX(i, j+3, dim)];
                dst[RIDX(dim-1-j-4, i, dim)] = src[RIDX(i, j+4, dim)];
                dst[RIDX(dim-1-j-5, i, dim)] = src[RIDX(i, j+5, dim)];
                dst[RIDX(dim-1-j-6, i, dim)] = src[RIDX(i, j+6, dim)];
                dst[RIDX(dim-1-j-7, i, dim)] = src[RIDX(i, j+7, dim)];
            }
        }
    }
}

char unroll_8_block_i_32_j_16_descr[] = "unroll_8_block_i_32_j_16";
void unroll_8_block_i_32_j_16(int dim, pixel *src, pixel *dst)
{
    for (int ii = 0; ii < dim; ii+=32)
    for (int jj = 0; jj < dim; jj+=16)
    for (int i = ii; i+7 < ii+32; i+=8)
        for (int j = jj; j < jj+16; j++)
             {
            dst[RIDX(dim-1-j, i+0, dim)] = src[RIDX(i+0, j, dim)];
            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
            dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
            dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];
            dst[RIDX(dim-1-j, i+4, dim)] = src[RIDX(i+4, j, dim)];
            dst[RIDX(dim-1-j, i+5, dim)] = src[RIDX(i+5, j, dim)];
            dst[RIDX(dim-1-j, i+6, dim)] = src[RIDX(i+6, j, dim)];
            dst[RIDX(dim-1-j, i+7, dim)] = src[RIDX(i+7, j, dim)];
        }
}

/*
 * naive_rotate - The naive baseline version of rotate
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst)
{
    for(int i = 0; i < dim; i++) {
        for(int j = 0; j < dim; j++) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
        }
    }
}
/*
 * rotate - Your current working version of rotate
 *          Our supplied version simply calls naive_rotate
 */
char another_rotate_descr[] = "another_rotate: Another version of rotate";
void another_rotate(int dim, pixel *src, pixel *dst)
{
    for(int ii = 0; ii < dim; ii+=32)
    for(int jj = 0; jj < dim; jj+=32)
    for(int i = ii; i+7 < ii+32; i+=8) {
        for(int j = jj; j < jj+32; j++) {
            dst[RIDX(i+7, j, dim)] = src[RIDX(j, dim-1-i-7, dim)];
            dst[RIDX(i+6, j, dim)] = src[RIDX(j, dim-1-i-6, dim)];
            dst[RIDX(i+5, j, dim)] = src[RIDX(j, dim-1-i-5, dim)];
            dst[RIDX(i+4, j, dim)] = src[RIDX(j, dim-1-i-4, dim)];
            dst[RIDX(i+3, j, dim)] = src[RIDX(j, dim-1-i-3, dim)];
            dst[RIDX(i+2, j, dim)] = src[RIDX(j, dim-1-i-2, dim)];
            dst[RIDX(i+1, j, dim)] = src[RIDX(j, dim-1-i-1, dim)];
            dst[RIDX(i+0, j, dim)] = src[RIDX(j, dim-1-i-0, dim)];
        }
    }
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate function by calling the add_rotate_function() for
 *     each test function. When you run the benchmark program, it will
 *     test and report the performance of each registered test
 *     function.
 *********************************************************************/

void register_rotate_functions() {
    add_rotate_function(&naive_rotate, naive_rotate_descr);
    add_rotate_function(&another_rotate, another_rotate_descr);
    //add_rotate_function(&unroll_4_rotate, unroll_4_rotate_descr);
    add_rotate_function(&unroll_4i_rotate, unroll_4i_rotate_descr);
    add_rotate_function(&unroll_8_rotate, unroll_8_rotate_descr);
    //add_rotate_function(&unroll_4_block_4_rotate, unroll_4_block_4_rotate_descr);
    //add_rotate_function(&unroll_4_block_16_rotate, unroll_4_block_16_rotate_descr);
    //add_rotate_function(&unroll_8_block_16_rotate, unroll_8_block_16_rotate_descr);
    add_rotate_function(&unroll_8_block_i_32_j_16, unroll_8_block_i_32_j_16_descr);

}
