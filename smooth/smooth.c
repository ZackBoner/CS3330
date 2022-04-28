#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <immintrin.h>

/*
 * Please fill in the following team struct
 */
who_t who = {
    "ZackB",           /* Scoreboard name */

    "Zachery Boner",      /* First member full name */
    "zwb6kg@virginia.edu",     /* First member email address */
};

/*** UTILITY FUNCTIONS ***/

/* You are free to use these utility functions, or write your own versions
 * of them. */

/* A struct used to compute averaged pixel value */
typedef struct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned short alpha;
    unsigned short num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/*
 * initialize_pixel_sum - Initializes all fields of sum to 0
 */
static void initialize_pixel_sum(pixel_sum *sum)
{
    sum->red = sum->green = sum->blue = sum->alpha = 0;
    sum->num = 0;
    return;
}

/*
 * accumulate_sum - Accumulates field values of p in corresponding
 * fields of sum
 */
static void accumulate_sum(pixel_sum *sum, pixel p)
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->alpha += (int) p.alpha;
    sum->num++;
    return;
}

/*
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum)
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    current_pixel->alpha = (unsigned short) (sum.alpha/sum.num);
    return;
}

/*
 * avg - Returns averaged pixel value at (i,j)
 */
static pixel avg(int dim, int i, int j, pixel *src)
{
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(int jj=max(j-1, 0); jj <= min(j+1, dim-1); jj++)
	for(int ii=max(i-1, 0); ii <= min(i+1, dim-1); ii++)
	    accumulate_sum(&sum, src[RIDX(ii,jj,dim)]);

    assign_sum_to_pixel(&current_pixel, sum);

    return current_pixel;
}



/******************************************************
 * Your different versions of the smooth go here
 ******************************************************/
 char unrolled_2_descr[] = "unrolled_2: unroll loop by 2";
 void unrolled_2(int dim, pixel *src, pixel *dst) {
     pixel_sum sum;
     pixel current_pixel, p;
     int i, j, ii, jj;
     // __m256i val1, val2, val3;
     __m128i mp1, mp2, mp3, mp4, mp5, mp6, mp7, mp8, mp9;
     __m256i p1, p2, p3, p4, p5, p6, p7, p8, p9;

     __m128i mp12, mp22, mp32, mp42, mp52, mp62, mp72, mp82, mp92;
     __m256i p12, p22, p32, p42, p52, p62, p72, p82, p92;

     // middle of image
     for (i = 1; i+1 < dim; i++) {
        for (j = 1; j+1 < dim; j+=2) {
            mp1 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j-1, dim)]);
            mp2 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j, dim)]);
            mp3 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+1, dim)]);
            mp4 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j-1, dim)]);
            mp5 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j, dim)]);
            mp6 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j+1, dim)]);
            mp7 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j-1, dim)]);
            mp8 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j, dim)]);
            mp9 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+1, dim)]);

            p1 = _mm256_cvtepu8_epi16(mp1);
            p2 = _mm256_cvtepu8_epi16(mp2);
            p3 = _mm256_cvtepu8_epi16(mp3);
            p4 = _mm256_cvtepu8_epi16(mp4);
            p5 = _mm256_cvtepu8_epi16(mp5);
            p6 = _mm256_cvtepu8_epi16(mp6);
            p7 = _mm256_cvtepu8_epi16(mp7);
            p8 = _mm256_cvtepu8_epi16(mp8);
            p9 = _mm256_cvtepu8_epi16(mp9);

            p1 = _mm256_add_epi16(p1, p2);
            p3 = _mm256_add_epi16(p3, p4);
            p5 = _mm256_add_epi16(p5, p6);
            p7 = _mm256_add_epi16(p7, p8);

            p1 = _mm256_add_epi16(p1, p3);
            p5 = _mm256_add_epi16(p5, p7);
            p1 = _mm256_add_epi16(p1, p5);
            p1 = _mm256_add_epi16(p1, p9);

            // divide by 9 using formula (x * 7282) >> 16
            p2 = _mm256_set1_epi16(7282);

            // mulhi effectively does the shift for us by taking
            // the high 16 bits of the multiplication result
            p1 = _mm256_mulhi_epi16(p1, p2);

            p2 = _mm256_permute2x128_si256(p1, p1, 0x34);
            p1 = _mm256_packus_epi16(p1, p2);

            mp1 = _mm256_extracti128_si256(p1, 0);
            mp2 = _mm_setr_epi32(-1, -1, dim-j-2, dim-j-1);
            _mm_maskstore_epi32((int*)&dst[RIDX(i, j, dim)], mp2, mp1);

            mp12 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j, dim)]);
            mp22 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+1, dim)]);
            mp32 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+2, dim)]);
            mp42 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j, dim)]);
            mp52 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j+1, dim)]);
            mp62 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j+2, dim)]);
            mp72 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j, dim)]);
            mp82 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+1, dim)]);
            mp92 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+2, dim)]);

            p12 = _mm256_cvtepu8_epi16(mp12);
            p22 = _mm256_cvtepu8_epi16(mp22);
            p32 = _mm256_cvtepu8_epi16(mp32);
            p42 = _mm256_cvtepu8_epi16(mp42);
            p52 = _mm256_cvtepu8_epi16(mp52);
            p62 = _mm256_cvtepu8_epi16(mp62);
            p72 = _mm256_cvtepu8_epi16(mp72);
            p82 = _mm256_cvtepu8_epi16(mp82);
            p92 = _mm256_cvtepu8_epi16(mp92);

            p12 = _mm256_add_epi16(p12, p22);
            p32 = _mm256_add_epi16(p32, p42);
            p52 = _mm256_add_epi16(p52, p62);
            p72 = _mm256_add_epi16(p72, p82);

            p12 = _mm256_add_epi16(p12, p32);
            p52 = _mm256_add_epi16(p52, p72);
            p12 = _mm256_add_epi16(p12, p52);
            p12 = _mm256_add_epi16(p12, p92);

            // divide by 9 using formula (x * 7282) >> 16
            p22 = _mm256_set1_epi16(7282);

            // mulhi effectively does the shift for us by taking
            // the high 16 bits of the multiplication result
            p12 = _mm256_mulhi_epi16(p12, p22);

            p22 = _mm256_permute2x128_si256(p12, p12, 0x34);
            p12 = _mm256_packus_epi16(p12, p22);

            mp12 = _mm256_extracti128_si256(p12, 0);
            mp22 = _mm_setr_epi32(-1, -1, dim-j-3, dim-2);
            _mm_maskstore_epi32((int*)&dst[RIDX(i, j+1, dim)], mp22, mp12);
         }
     }

     // sides of image
     // TOP
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(0,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(0, j, dim)] = current_pixel;
     }

     // BOTTOM
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(dim-1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(dim-2,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(dim-1, j, dim)] = current_pixel;
     }

     // LEFT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, 0, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, 1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, 0, dim)] = current_pixel;
     }

     // RIGHT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, dim-1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, dim-2, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, dim-1, dim)] = current_pixel;
     }

     // corners of image
     // TOP LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, 0, dim)] = current_pixel;

     // TOP RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, dim-1, dim)] = current_pixel;

     // BOTTOM LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, 0, dim)] = current_pixel;

     // BOTTOM RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, dim-1, dim)] = current_pixel;
 }

 char steps_1234ab_descr[] = "steps_1234ab: implement divide by 9";
 void steps_1234ab(int dim, pixel *src, pixel *dst) {
     pixel_sum sum;
     pixel current_pixel, p;
     int i, j, ii, jj;
     // __m256i val1, val2, val3;
     __m128i mp1, mp2, mp3, mp4, mp5, mp6, mp7, mp8, mp9;
     __m256i p1, p2, p3, p4, p5, p6, p7, p8, p9;

     // middle of image
     for (i = 1; i+1 < dim; i++) {
        for (j = 1; j+1 < dim; j++) {
            mp1 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j-1, dim)]);
            mp2 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j, dim)]);
            mp3 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+1, dim)]);
            mp4 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j-1, dim)]);
            mp5 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j, dim)]);
            mp6 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j+1, dim)]);
            mp7 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j-1, dim)]);
            mp8 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j, dim)]);
            mp9 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+1, dim)]);

            p1 = _mm256_cvtepu8_epi16(mp1);
            p2 = _mm256_cvtepu8_epi16(mp2);
            p3 = _mm256_cvtepu8_epi16(mp3);
            p4 = _mm256_cvtepu8_epi16(mp4);
            p5 = _mm256_cvtepu8_epi16(mp5);
            p6 = _mm256_cvtepu8_epi16(mp6);
            p7 = _mm256_cvtepu8_epi16(mp7);
            p8 = _mm256_cvtepu8_epi16(mp8);
            p9 = _mm256_cvtepu8_epi16(mp9);

            p1 = _mm256_add_epi16(p1, p2);
            p3 = _mm256_add_epi16(p3, p4);
            p5 = _mm256_add_epi16(p5, p6);
            p7 = _mm256_add_epi16(p7, p8);

            p1 = _mm256_add_epi16(p1, p3);
            p5 = _mm256_add_epi16(p5, p7);
            p1 = _mm256_add_epi16(p1, p5);
            p1 = _mm256_add_epi16(p1, p9);

            // divide by 9 using formula (x * 7282) >> 16
            p2 = _mm256_set1_epi16(7282);

            // mulhi effectively does the shift for us by taking
            // the high 16 bits of the multiplication result
            p1 = _mm256_mulhi_epi16(p1, p2);

            p2 = _mm256_permute2x128_si256(p1, p1, 0x34);
            p1 = _mm256_packus_epi16(p1, p2);

            mp1 = _mm256_extracti128_si256(p1, 0);
            mp2 = _mm_setr_epi32(-1, -1, dim-j-2, dim-j-1);
            _mm_maskstore_epi32((int*)&dst[RIDX(i, j, dim)], mp2, mp1);
         }
     }

     // sides of image
     // TOP
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(0,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(0, j, dim)] = current_pixel;
     }

     // BOTTOM
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(dim-1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(dim-2,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(dim-1, j, dim)] = current_pixel;
     }

     // LEFT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, 0, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, 1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, 0, dim)] = current_pixel;
     }

     // RIGHT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, dim-1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, dim-2, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, dim-1, dim)] = current_pixel;
     }

     // corners of image
     // TOP LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, 0, dim)] = current_pixel;

     // TOP RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, dim-1, dim)] = current_pixel;

     // BOTTOM LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, 0, dim)] = current_pixel;

     // BOTTOM RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, dim-1, dim)] = current_pixel;
 }

 char steps_1234a_descr[] = "steps_1234a: compute 4 pixels at a time";
 void steps_1234a(int dim, pixel *src, pixel *dst) {
     pixel_sum sum;
     pixel current_pixel, p;
     int i, j, ii, jj;

     // middle of image
     for (i = 1; i+1 < dim; i++) {
        for (j = 1; j+1 < dim; j++) {
            __m128i mp1 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j-1, dim)]);
            __m128i mp2 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j, dim)]);
            __m128i mp3 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+1, dim)]);
            __m128i mp4 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j-1, dim)]);
            __m128i mp5 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j, dim)]);
            __m128i mp6 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j+1, dim)]);
            __m128i mp7 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j-1, dim)]);
            __m128i mp8 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j, dim)]);
            __m128i mp9 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+1, dim)]);

            __m256i p1 = _mm256_cvtepu8_epi16(mp1);
            __m256i p2 = _mm256_cvtepu8_epi16(mp2);
            __m256i p3 = _mm256_cvtepu8_epi16(mp3);
            __m256i p4 = _mm256_cvtepu8_epi16(mp4);
            __m256i p5 = _mm256_cvtepu8_epi16(mp5);
            __m256i p6 = _mm256_cvtepu8_epi16(mp6);
            __m256i p7 = _mm256_cvtepu8_epi16(mp7);
            __m256i p8 = _mm256_cvtepu8_epi16(mp8);
            __m256i p9 = _mm256_cvtepu8_epi16(mp9);

            p1 = _mm256_add_epi16(p1, p2);
            p3 = _mm256_add_epi16(p3, p4);
            p5 = _mm256_add_epi16(p5, p6);
            p7 = _mm256_add_epi16(p7, p8);
            __m256i sum1 = _mm256_add_epi16(p1, p3);
            __m256i sum2 = _mm256_add_epi16(p5, p7);
            __m256i sum3 = _mm256_add_epi16(sum1, sum2);
            __m256i px_sum = _mm256_add_epi16(sum3, p9);

            // divide by 9 using formula (x * 7282) >> 16
            __m256i val1 = _mm256_set1_epi16(7282);

            // mulhi effectively does the shift for us by taking
            // the high 16 bits of the multiplication result
            __m256i val2 = _mm256_mulhi_epi16(px_sum, val1);

            __m256i second_half_first = _mm256_permute2x128_si256(val2, val2, 0x34);
            __m256i packed = _mm256_packus_epi16(val2, second_half_first);

            __m128i result = _mm256_extracti128_si256(packed, 0);
            __m128i mask = _mm_setr_epi32(-1, -1, dim-j-2, dim-j-1);
            _mm_maskstore_epi32((int*)&dst[RIDX(i, j, dim)], mask, result);

         }
     }

     // sides of image
     // TOP
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(0,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(0, j, dim)] = current_pixel;
     }

     // BOTTOM
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(dim-1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(dim-2,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(dim-1, j, dim)] = current_pixel;
     }

     // LEFT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, 0, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, 1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, 0, dim)] = current_pixel;
     }

     // RIGHT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, dim-1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, dim-2, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, dim-1, dim)] = current_pixel;
     }

     // corners of image
     // TOP LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, 0, dim)] = current_pixel;

     // TOP RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, dim-1, dim)] = current_pixel;

     // BOTTOM LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, 0, dim)] = current_pixel;

     // BOTTOM RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, dim-1, dim)] = current_pixel;
 }

 char steps_123_descr[] = "steps_123: vectorize pixel math";
 void steps_123(int dim, pixel *src, pixel *dst) {
     pixel_sum sum;
     pixel current_pixel, p;
     int i, j, ii, jj;

     // middle of image
     for (i = 1; i+1 < dim; i++) {
     	for (j = 1; j+1 < dim; j++) {
            __m128i mp1 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j-1, dim)]);
            __m128i mp2 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j, dim)]);
            __m128i mp3 = _mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+1, dim)]);
            __m128i mp4 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j-1, dim)]);
            __m128i mp5 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j, dim)]);
            __m128i mp6 = _mm_loadu_si128((__m128i*) &src[RIDX(i, j+1, dim)]);
            __m128i mp7 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j-1, dim)]);
            __m128i mp8 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j, dim)]);
            __m128i mp9 = _mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+1, dim)]);

            __m256i p1 = _mm256_cvtepu8_epi16(mp1);
            __m256i p2 = _mm256_cvtepu8_epi16(mp2);
            __m256i p3 = _mm256_cvtepu8_epi16(mp3);
            __m256i p4 = _mm256_cvtepu8_epi16(mp4);
            __m256i p5 = _mm256_cvtepu8_epi16(mp5);
            __m256i p6 = _mm256_cvtepu8_epi16(mp6);
            __m256i p7 = _mm256_cvtepu8_epi16(mp7);
            __m256i p8 = _mm256_cvtepu8_epi16(mp8);
            __m256i p9 = _mm256_cvtepu8_epi16(mp9);

            p1 = _mm256_add_epi16(p1, p2);
            p3 = _mm256_add_epi16(p3, p4);
            p5 = _mm256_add_epi16(p5, p6);
            p7 = _mm256_add_epi16(p7, p8);
            __m256i sum1 = _mm256_add_epi16(p1, p3);
            __m256i sum2 = _mm256_add_epi16(p5, p7);
            __m256i sum3 = _mm256_add_epi16(sum1, sum2);
            __m256i px_sum = _mm256_add_epi16(sum3, p9);

            unsigned short px_elements[16];
            _mm256_storeu_si256((__m256i*) px_elements, px_sum);

            pixel final_pixel;
            final_pixel.red = px_elements[0]/9;
            final_pixel.green = px_elements[1]/9;
            final_pixel.blue = px_elements[2]/9;
            final_pixel.alpha = px_elements[3]/9;
            dst[RIDX(i, j, dim)] = final_pixel;
         }
     }

     // sides of image
     // TOP
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(0,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(0, j, dim)] = current_pixel;
     }

     // BOTTOM
     for (j = 1; j+1 < dim; j++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(jj=j-1; jj <= j+1; jj++) {
             p = src[RIDX(dim-1,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(dim-2,jj,dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(dim-1, j, dim)] = current_pixel;
     }

     // LEFT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, 0, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, 1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, 0, dim)] = current_pixel;
     }

     // RIGHT
     for (i = 1; i+1 < dim; i++) {
         sum.red = sum.green = sum.blue = sum.alpha = 0;
         for(ii=i-1; ii <= i+1; ii++) {
             p = src[RIDX(ii, dim-1, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;

             p = src[RIDX(ii, dim-2, dim)];
             sum.red += (int) p.red;
             sum.green += (int) p.green;
             sum.blue += (int) p.blue;
             sum.alpha += (int) p.alpha;
         }
         current_pixel.red = (unsigned short) (sum.red/6);
         current_pixel.green = (unsigned short) (sum.green/6);
         current_pixel.blue = (unsigned short) (sum.blue/6);
         current_pixel.alpha = (unsigned short) (sum.alpha/6);
         dst[RIDX(i, dim-1, dim)] = current_pixel;
     }

     // corners of image
     // TOP LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, 0, dim)] = current_pixel;

     // TOP RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(0, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(0, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(0, dim-1, dim)] = current_pixel;

     // BOTTOM LEFT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 0, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, 1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, 0, dim)] = current_pixel;

     // BOTTOM RIGHT
     sum.red = sum.green = sum.blue = sum.alpha = 0;

         p = src[RIDX(dim-1, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-1, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-1, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

         p = src[RIDX(dim-2, dim-2, dim)];
         sum.red += (int) p.red;
         sum.green += (int) p.green;
         sum.blue += (int) p.blue;
         sum.alpha += (int) p.alpha;

     current_pixel.red = (unsigned short) (sum.red >> 2);
     current_pixel.green = (unsigned short) (sum.green >> 2);
     current_pixel.blue = (unsigned short) (sum.blue >> 2);
     current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
     dst[RIDX(dim-1, dim-1, dim)] = current_pixel;
 }

char steps_1_and_2_descr[] = "steps_1_and_2: separate edge cases and unroll inner loop";
void steps_1_and_2(int dim, pixel *src, pixel *dst) {
    pixel_sum sum;
    pixel current_pixel;
    pixel p;
    int i, j, ii, jj;

    // middle of image
    for (i = 1; i+1 < dim; i++) {
    	for (j = 1; j+1 < dim; j++) {
            sum.red = sum.green = sum.blue = sum.alpha = 0;

                p = src[RIDX(i-1,j-1,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i-1,j,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i-1,j+1,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i,j-1,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i,j,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i,j+1,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i+1,j-1,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i+1,j,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

                p = src[RIDX(i+1,j+1,dim)];
                sum.red += (int) p.red;
                sum.green += (int) p.green;
                sum.blue += (int) p.blue;
                sum.alpha += (int) p.alpha;

            current_pixel.red = (unsigned short) (sum.red/9);
            current_pixel.green = (unsigned short) (sum.green/9);
            current_pixel.blue = (unsigned short) (sum.blue/9);
            current_pixel.alpha = (unsigned short) (sum.alpha/9);
            dst[RIDX(i, j, dim)] = current_pixel;
        }
    }

    // sides of image
    // TOP
    for (j = 1; j+1 < dim; j++) {
        sum.red = sum.green = sum.blue = sum.alpha = 0;
        for(jj=j-1; jj <= j+1; jj++) {
            p = src[RIDX(0,jj,dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;

            p = src[RIDX(1,jj,dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;
        }
        current_pixel.red = (unsigned short) (sum.red/6);
        current_pixel.green = (unsigned short) (sum.green/6);
        current_pixel.blue = (unsigned short) (sum.blue/6);
        current_pixel.alpha = (unsigned short) (sum.alpha/6);
        dst[RIDX(0, j, dim)] = current_pixel;
    }

    // BOTTOM
    for (j = 1; j+1 < dim; j++) {
        sum.red = sum.green = sum.blue = sum.alpha = 0;
        for(jj=j-1; jj <= j+1; jj++) {
            p = src[RIDX(dim-1,jj,dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;

            p = src[RIDX(dim-2,jj,dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;
        }
        current_pixel.red = (unsigned short) (sum.red/6);
        current_pixel.green = (unsigned short) (sum.green/6);
        current_pixel.blue = (unsigned short) (sum.blue/6);
        current_pixel.alpha = (unsigned short) (sum.alpha/6);
        dst[RIDX(dim-1, j, dim)] = current_pixel;
    }

    // LEFT
    for (i = 1; i+1 < dim; i++) {
        sum.red = sum.green = sum.blue = sum.alpha = 0;
        for(ii=i-1; ii <= i+1; ii++) {
            p = src[RIDX(ii, 0, dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;

            p = src[RIDX(ii, 1, dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;
        }
        current_pixel.red = (unsigned short) (sum.red/6);
        current_pixel.green = (unsigned short) (sum.green/6);
        current_pixel.blue = (unsigned short) (sum.blue/6);
        current_pixel.alpha = (unsigned short) (sum.alpha/6);
        dst[RIDX(i, 0, dim)] = current_pixel;
    }

    // RIGHT
    for (i = 1; i+1 < dim; i++) {
        sum.red = sum.green = sum.blue = sum.alpha = 0;
        for(ii=i-1; ii <= i+1; ii++) {
            p = src[RIDX(ii, dim-1, dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;

            p = src[RIDX(ii, dim-2, dim)];
            sum.red += (int) p.red;
            sum.green += (int) p.green;
            sum.blue += (int) p.blue;
            sum.alpha += (int) p.alpha;
        }
        current_pixel.red = (unsigned short) (sum.red/6);
        current_pixel.green = (unsigned short) (sum.green/6);
        current_pixel.blue = (unsigned short) (sum.blue/6);
        current_pixel.alpha = (unsigned short) (sum.alpha/6);
        dst[RIDX(i, dim-1, dim)] = current_pixel;
    }

    // corners of image
    // TOP LEFT
    sum.red = sum.green = sum.blue = sum.alpha = 0;

        p = src[RIDX(0, 0, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(0, 1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(1, 0, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(1, 1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

    current_pixel.red = (unsigned short) (sum.red >> 2);
    current_pixel.green = (unsigned short) (sum.green >> 2);
    current_pixel.blue = (unsigned short) (sum.blue >> 2);
    current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
    dst[RIDX(0, 0, dim)] = current_pixel;

    // TOP RIGHT
    sum.red = sum.green = sum.blue = sum.alpha = 0;

        p = src[RIDX(0, dim-1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(0, dim-2, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(1, dim-2, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(1, dim-1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

    current_pixel.red = (unsigned short) (sum.red >> 2);
    current_pixel.green = (unsigned short) (sum.green >> 2);
    current_pixel.blue = (unsigned short) (sum.blue >> 2);
    current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
    dst[RIDX(0, dim-1, dim)] = current_pixel;

    // BOTTOM LEFT
    sum.red = sum.green = sum.blue = sum.alpha = 0;

        p = src[RIDX(dim-1, 0, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(dim-2, 0, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(dim-2, 1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(dim-1, 1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

    current_pixel.red = (unsigned short) (sum.red >> 2);
    current_pixel.green = (unsigned short) (sum.green >> 2);
    current_pixel.blue = (unsigned short) (sum.blue >> 2);
    current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
    dst[RIDX(dim-1, 0, dim)] = current_pixel;

    // BOTTOM RIGHT
    sum.red = sum.green = sum.blue = sum.alpha = 0;

        p = src[RIDX(dim-1, dim-1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(dim-2, dim-1, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(dim-1, dim-2, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

        p = src[RIDX(dim-2, dim-2, dim)];
        sum.red += (int) p.red;
        sum.green += (int) p.green;
        sum.blue += (int) p.blue;
        sum.alpha += (int) p.alpha;

    current_pixel.red = (unsigned short) (sum.red >> 2);
    current_pixel.green = (unsigned short) (sum.green >> 2);
    current_pixel.blue = (unsigned short) (sum.blue >> 2);
    current_pixel.alpha = (unsigned short) (sum.alpha >> 2);
    dst[RIDX(dim-1, dim-1, dim)] = current_pixel;
}

/*
 * naive_smooth - The naive baseline version of smooth
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst)
{
    for (int i = 0; i < dim; i++)
	for (int j = 0; j < dim; j++)
            dst[RIDX(i,j, dim)] = avg(dim, i, j, src);
}
/*
 * smooth - Your current working version of smooth
 *          Our supplied version simply calls naive_smooth
 */
char another_smooth_descr[] = "another_smooth: Another version of smooth";
void another_smooth(int dim, pixel *src, pixel *dst)
{
    naive_smooth(dim, src, dst);
}

/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth function by calling the add_smooth_function() for
 *     each test function. When you run the benchmark program, it will
 *     test and report the performance of each registered test
 *     function.
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    add_smooth_function(&another_smooth, another_smooth_descr);
    add_smooth_function(&steps_1_and_2, steps_1_and_2_descr);
    add_smooth_function(&steps_123, steps_123_descr);
    add_smooth_function(&steps_1234a, steps_1234a_descr);
    add_smooth_function(&steps_1234ab, steps_1234ab_descr);
    add_smooth_function(&unrolled_2, unrolled_2_descr);
}
