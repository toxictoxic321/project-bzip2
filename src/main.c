#include "project.h"
#include <time.h>

int main() {
    //  loading  config
    size_t config_size = parse_config_block_size("config.ini");
    int    threshold = parse_config_threshold("config.ini");
    clock_t start = clock();

    printf(" BZip2  1st Stage  \n");
    printf("Block size    =  %zu bytes\n", config_size);
    printf("RLE-1 mode    =  threshold \n");
    printf("RLE threshold =  %d\n", threshold);
    printf("BWT type      =  suffix array \n");
    printf("---------------------\n");

    //  dividinge into blocks
    BlockManager manager = divide_into_blocks("input.txt", config_size);
    if (manager.num_blocks == 0) return 1;

    Block* b = &manager.blocks[0];

    unsigned char* rle_buf = malloc(b->size * 2 + 64);
    unsigned char* bwt_buf = malloc(b->size * 2 + 64);
    unsigned char* ibwt_buf = malloc(b->size * 2 + 64);
    unsigned char* final_dec = malloc(b->size * 2 + 64);

    size_t rle_len, final_len;
    int    p_index;

    //  ENCODING is here
    rle1_encode_threshold(b->data, b->size, rle_buf, &rle_len, threshold);
    bwt_encode(rle_buf, rle_len, bwt_buf, &p_index);

    //  Saveing  compressed binary
    FILE* comp_file = fopen("compressed", "wb");
    if (comp_file) {
        fwrite(&p_index, sizeof(int), 1, comp_file);
        fwrite(&rle_len, sizeof(size_t), 1, comp_file);
        fwrite(&threshold, sizeof(int), 1, comp_file);
        fwrite(bwt_buf, 1, rle_len, comp_file);
        fclose(comp_file);
        printf("Compressed file saved (%zu bytes)\n",
            sizeof(int) + sizeof(size_t) + sizeof(int) + rle_len);
    }

    //  DECODING is hereee
    bwt_decode(bwt_buf, rle_len, p_index, ibwt_buf);
    rle1_decode_threshold(ibwt_buf, rle_len, final_dec, &final_len, threshold);

    //  Verification of  round-trip
    if (final_len == b->original_size &&
        memcmp(final_dec, b->data, final_len) == 0)
        printf("Round-trip verification =  PASSED\n");
    else
        printf("Round-trip verification =  FAILED (orig=%zu decoded=%zu)\n",
            b->original_size, final_len);

    //  Saveing restored output
    memcpy(b->data, final_dec, final_len);
    b->size = final_len;
    reassemble_blocks(&manager, "restored_output.txt");
    printf("Restored file saved: restored_output.txt\n");

    //  Metrics are hreer
    size_t compressed_size = sizeof(int) + sizeof(size_t) + sizeof(int) + rle_len;
    double ratio = (double)b->original_size / compressed_size;
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    //  Results CSV file
    FILE* res = fopen("results.csv", "a");
    if (res) {
        fprintf(res, "input.txt,%zu,%zu,threshold-%d,%.4f,%.4f,%zu\n",
            b->original_size, config_size, threshold,
            ratio, time_taken, compressed_size);
        fclose(res);
    }

    printf("---------------------\n");
    printf("Original    =  %zu bytes\n", b->original_size);
    printf("After RLE1 =  %zu bytes\n", rle_len);
    printf("Compressed =  %zu bytes\n", compressed_size);
    printf("Ratio      =  %.4f\n", ratio);
    printf("Time       =  %.4f seconds\n", time_taken);

    free(rle_buf); free(bwt_buf); free(ibwt_buf); free(final_dec);
    free_block_manager(&manager);
    return 0;
}