#include "project.h"
#include <time.h>

int main() {
    // 1. Load config
    size_t config_size = parse_config_block_size("config.ini");
    int    threshold = parse_config_threshold("config.ini");
    clock_t start = clock();

    printf("=== BZip2 Final Pipeline (Stage 3) ===\n");
    printf("Block size    =  %zu bytes\n", config_size);
    printf("RLE-1 mode    =  threshold \n");
    printf("RLE threshold =  %d\n", threshold);
    printf("BWT type      =  suffix array \n");
    printf("--------------------------------------\n");

    // 2. Divide into blocks
    BlockManager manager = divide_into_blocks("input.txt", config_size);
    if (manager.num_blocks == 0) return 1;

    Block* b = &manager.blocks[0];

    // Allocate buffers for all pipeline stages
    // Added 1024 bytes padding for safety with bit-level Huffman headers
    unsigned char* rle_buf = malloc(b->size * 2 + 1024);
    unsigned char* bwt_buf = malloc(b->size * 2 + 1024);
    unsigned char* mtf_buf = malloc(b->size * 2 + 1024);
    unsigned char* rle2_buf = malloc(b->size * 2 + 1024);
    unsigned char* huff_buf = malloc(b->size * 2 + 1024);

    unsigned char* ihuff_buf = malloc(b->size * 2 + 1024);
    unsigned char* imtf_buf = malloc(b->size * 2 + 1024);
    unsigned char* ibwt_buf = malloc(b->size * 2 + 1024);
    unsigned char* irle_buf = malloc(b->size * 2 + 1024);
    unsigned char* final_dec = malloc(b->size * 2 + 1024);

    size_t rle_len, rle2_len, huff_len;
    size_t decoded_rle2_len, decoded_mtf_len, final_len;
    int    p_index;

    // === ENCODING PIPELINE ===
    rle1_encode_threshold(b->data, b->size, rle_buf, &rle_len, threshold);
    bwt_encode(rle_buf, rle_len, bwt_buf, &p_index);
    mtf_encode(bwt_buf, rle_len, mtf_buf);
    rle2_encode(mtf_buf, rle_len, rle2_buf, &rle2_len);
    huffman_encode(rle2_buf, rle2_len, huff_buf, &huff_len);

    // Saving compressed binary (Now saving purely bit-compressed Huffman data)
    FILE* comp_file = fopen("compressed", "wb");
    if (comp_file) {
        fwrite(&p_index, sizeof(int), 1, comp_file);
        fwrite(&rle_len, sizeof(size_t), 1, comp_file);
        fwrite(&threshold, sizeof(int), 1, comp_file);
        fwrite(huff_buf, 1, huff_len, comp_file);
        fclose(comp_file);
        printf("Compressed binary saved to disk.\n");
    }

    // === DECODING PIPELINE ===
    huffman_decode(huff_buf, huff_len, ihuff_buf, &decoded_rle2_len);
    rle2_decode(ihuff_buf, decoded_rle2_len, imtf_buf, &decoded_mtf_len);
    mtf_decode(imtf_buf, decoded_mtf_len, ibwt_buf);
    bwt_decode(ibwt_buf, decoded_mtf_len, p_index, irle_buf);
    rle1_decode_threshold(irle_buf, decoded_mtf_len, final_dec, &final_len, threshold);

    // Verification of round-trip
    if (final_len == b->original_size && memcmp(final_dec, b->data, final_len) == 0)
        printf("Round-trip verification =  PASSED\n");
    else
        printf("Round-trip verification =  FAILED (orig=%zu decoded=%zu)\n", b->original_size, final_len);

    // Saving restored output
    memcpy(b->data, final_dec, final_len);
    b->size = final_len;
    reassemble_blocks(&manager, "restored_output.txt");
    printf("Restored file saved: restored_output.txt\n");

    // Metrics
    size_t compressed_size = sizeof(int) + sizeof(size_t) + sizeof(int) + huff_len;
    double ratio = (double)b->original_size / compressed_size;
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    // Results CSV file
    FILE* res = fopen("results.csv", "a");
    if (res) {
        fprintf(res, "input.txt,%zu,%zu,threshold-%d,%.4f,%.4f,%zu\n",
            b->original_size, config_size, threshold, ratio, time_taken, compressed_size);
        fclose(res);
    }

    printf("--------------------------------------\n");
    printf("Original    =  %zu bytes\n", b->original_size);
    printf("After RLE1  =  %zu bytes\n", rle_len);
    printf("After RLE2  =  %zu bytes\n", rle2_len);
    printf("Final Huff  =  %zu bytes\n", compressed_size);
    printf("Final Ratio =  %.4f\n", ratio);
    printf("Total Time  =  %.4f seconds\n", time_taken);
    printf("--------------------------------------\n");

    // Clean up memory
    free(rle_buf); free(bwt_buf); free(mtf_buf); free(rle2_buf); free(huff_buf);
    free(ihuff_buf); free(imtf_buf); free(ibwt_buf); free(irle_buf); free(final_dec);
    free_block_manager(&manager);
    return 0;
}