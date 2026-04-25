#include "project.h"

/*
 * Helper struct for prefix-doubling suffix array construction
 */
typedef struct {
    int idx;                     // original position in text
    int r0;   // rank of current position
    int r1;      // rank of position + gap
} SuffixRank;

static int cmp_suffix_rank(const void* a, const void* b) {
    const SuffixRank* x = (const SuffixRank*)a;
    const SuffixRank* y = (const SuffixRank*)b;
    if (x->r0 != y->r0) return x->r0 - y->r0;
    return x->r1 - y->r1;
}


 //  suffix Array-based BWT
  //Uses prefix-doubling on the DOUBLED string (text+text) to correctly
 // handle CYCLIC comparisons required by BWT - O(N log^2 N).
 
//  Why doubled string?
  //  BWT needs cyclic rotation order, not suffix order.
   // Comparing suffix[i] of (text+text) for the first N characters
    //is identical to comparing cyclic rotation i of text.
 
int* build_suffix_array(unsigned char* text, int n) {
    if (n == 0) return NULL;

    int  n2 = n * 2;
    int* sa = (int*)malloc(n * sizeof(int));
    int* rank2 = (int*)malloc(n2 * sizeof(int));
    SuffixRank* sr = (SuffixRank*)malloc(n * sizeof(SuffixRank));

    if (!sa || !rank2 || !sr) { free(sa); free(rank2); free(sr); return NULL; }

    /* Initial rank = character value, mirrored for the doubled string */
    for (int i = 0; i < n; i++) rank2[i] = rank2[i + n] = (int)text[i];

    for (int gap = 1; gap < n; gap *= 2) {
        /* Build (rank[i], rank[i+gap]) pairs for positions 0..n-1 */
        for (int i = 0; i < n; i++) {
            sr[i].idx = i;
            sr[i].r0 = rank2[i];
            sr[i].r1 = rank2[i + gap]; /* always valid: i+gap < n+n */
        }
        qsort(sr, n, sizeof(SuffixRank), cmp_suffix_rank);

        /* Assign new compressed ranks */
        int* new_rank = (int*)calloc(n2, sizeof(int));
        if (!new_rank) { free(sa); free(rank2); free(sr); return NULL; }

        new_rank[sr[0].idx] = 0;
        for (int i = 1; i < n; i++) {
            new_rank[sr[i].idx] = new_rank[sr[i - 1].idx];
            if (sr[i].r0 != sr[i - 1].r0 || sr[i].r1 != sr[i - 1].r1)
                new_rank[sr[i].idx]++;
        }
            /* Mirror: cyclic rotation i and i+n are the same */
        for (int i = 0; i < n; i++) new_rank[i + n] = new_rank[i];

        for (int i = 0; i < n2; i++) rank2[i] = new_rank[i];
        free(new_rank);

            /* Capture current sorted order into sa */
        for (int i = 0; i < n; i++) sa[i] = sr[i].idx;

               /* Early exit: all ranks are unique */
        if (rank2[sa[n - 1]] == n - 1) break;
    }

    free(rank2);
    free(sr);
    return sa;
}

/*
 * Forward Burrows-Wheeler Transform
 * Useing the  suffix array of doubled string for correct cyclic sorting.
 */
void bwt_encode(unsigned char* input, size_t len, unsigned char* output, int* primary_index) {
    int n = (int)len;
    int* sa = build_suffix_array(input, n);
    if (!sa) return;

    for (int i = 0; i < n; i++) {
        if (sa[i] == 0) {
            output[i] = input[n - 1];       /* wrap-around character */
            *primary_index = i;
        }
        else {
            output[i] = input[sa[i] - 1];
        }
    }
    free(sa);
}

/*
 * Inverse Burrows-Wheeler Transform
          * Uses the Last-to-First ..............(T-vector) mapping to reconstruct original data.
 */
void bwt_decode(unsigned char* input, size_t len, int primary_index, unsigned char* output) {
    int* count = (int*)calloc(256, sizeof(int));
    int* running_sum = (int*)malloc(256 * sizeof(int));
    int* T = (int*)malloc(len * sizeof(int));

    if (!count || !running_sum || !T) {
        free(count); free(running_sum); free(T);
        return;
    }

                    /* Count occurrences of each character in the last column (input) */
    for (size_t i = 0; i < len; i++)
        count[input[i]]++;

                         /* Cumulative sum gives starting position of each char in first column */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        running_sum[i] = sum;
        sum += count[i];
    }

         /* Build T-vector: T[i] = position in first column of the char at input[i] */
    for (size_t i = 0; i < len; i++)
        T[running_sum[input[i]]++] = (int)i;

                   /* Walk T-vector from primary_index to reconstruct original string */
    int curr = T[primary_index];
    for (size_t i = 0; i < len; i++) {
        output[i] = input[curr];
        curr = T[curr];
    }

    free(count);
    free(running_sum);
    free(T);
}