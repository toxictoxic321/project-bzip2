#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


      //  data structures hereeeeee


typedef struct {
    unsigned char* data;
    size_t size;
    size_t original_size;
} Block;

typedef struct {
    Block* blocks;
    int     num_blocks;
    size_t  block_size;
} BlockManager;

typedef struct {
    int            index;
    unsigned char* rotation;
} Rotation;


     //  Function Prototypes to bes used are hereeee


 // Config
size_t parse_config_block_size(const char* filename);
int    parse_config_threshold(const char* filename);

             // blockkk management
BlockManager divide_into_blocks(const char* filename, size_t block_size);
int          reassemble_blocks(BlockManager* manager, const char* output_filename);
void         free_block_manager(BlockManager* manager);

      // Standard RLE-1
void rle1_encode(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len);
void rle1_decode(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len);

//      Threshold RLE-1
//           Threshold RLE-1
void rle1_encode_threshold(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len,
    int threshold);
void rle1_decode_threshold(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len,
    int threshold);

// bwt suufix array here prototype
int* build_suffix_array(unsigned char* text, int n);
void bwt_encode(unsigned char* input, size_t len,
    unsigned char* output, int* primary_index);
void bwt_decode(unsigned char* input, size_t len,
    int primary_index, unsigned char* output);


// Stage 2: Move-to-Front (MTF) & RLE-2

void mtf_encode(unsigned char* input, size_t len, unsigned char* output);
void mtf_decode(unsigned char* input, size_t len, unsigned char* output);

void rle2_encode(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len);
void rle2_decode(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len);


        //  Canonical Huffman Coding


typedef struct {
    unsigned short code;    // Huffman code
    unsigned char length;   // Code length in bits
} HuffmanCode;

typedef struct Node {
    unsigned char symbol;   // Byte value (0-255)
    int freq;               // Frequency count
    struct Node* left;      // Left child
    struct Node* right;     // Right child
} HuffmanNode;

// Function Prototypes
void build_huffman_tree(int* frequencies, HuffmanNode** root);
void generate_canonical_codes(HuffmanNode* root, HuffmanCode* codes);
void huffman_encode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len);
void huffman_decode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len);
void write_header(HuffmanCode* codes, unsigned char* output, size_t* out_len);
void encode_data(unsigned char* input, size_t len, HuffmanCode* codes, unsigned char* output, size_t* out_len);




#endif