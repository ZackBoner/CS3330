#include <stdlib.h>
#include <limits.h>  /* for USHRT_MAX */
#include <stdio.h>
#include <immintrin.h>

#include "min.h"
/* reference implementation in C */
short min_C(long size, short * a) {
    short result = SHRT_MAX;
    for (int i = 0; i < size; ++i) {
        if (a[i] < result)
            result = a[i];
    }
    return result;
}

short min_AVX(long size, short * a) {
    __m256i current_mins = _mm256_set1_epi16(SHRT_MAX);
    for (int i = 0; i < size; i += 16) {
        __m256i vals = _mm256_loadu_si256((__m256i*) &a[i]);
        current_mins = _mm256_min_epi16(current_mins, vals);
    }
    short extracted_mins[16];
    _mm256_storeu_si256((__m256i*) &extracted_mins, current_mins);

    short result = SHRT_MAX;
    for (int i = 0; i < 16; ++i) {
        if (extracted_mins[i] < result)
            result = extracted_mins[i];
    }

    return result;
}


/* This is the list of functions to test */
function_info functions[] = {
    {min_C, "C (local)"},
    {min_AVX, "AVX"},
    // add entries here!
    {NULL, NULL},
};
