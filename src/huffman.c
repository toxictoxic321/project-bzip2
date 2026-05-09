#include "project.h"

typedef struct {
    unsigned char* buffer;
    size_t byte_pos;
    unsigned char current_byte;
    int bits_filled;
} BitStream;

void write_bits(BitStream* stream, unsigned short code, unsigned char length) {
    for (int i = length - 1; i >= 0; i--) {
        int bit = (code >> i) & 1;
        stream->current_byte = (stream->current_byte << 1) | bit;
        stream->bits_filled++;

        if (stream->bits_filled == 8) {
            stream->buffer[stream->byte_pos++] = stream->current_byte;
            stream->current_byte = 0;
            stream->bits_filled = 0;
        }
    }
}

int read_bit(BitStream* stream) {
    if (stream->bits_filled == 0) {
        stream->current_byte = stream->buffer[stream->byte_pos++];
        stream->bits_filled = 8;
    }
    int bit = (stream->current_byte >> 7) & 1;
    stream->current_byte <<= 1;
    stream->bits_filled--;
    return bit;
}

                              // tree building 
void build_huffman_tree(int* frequencies, HuffmanNode** root) {
    HuffmanNode* nodes[512];
    int node_count = 0;

    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            nodes[node_count] = (HuffmanNode*)calloc(1, sizeof(HuffmanNode));
            nodes[node_count]->symbol = (unsigned char)i;
            nodes[node_count]->freq = frequencies[i];
            node_count++;
        }
    }

    if (node_count == 0) { *root = NULL; return; }
    if (node_count == 1) { *root = nodes[0]; return; }

    while (node_count > 1) {
        int min1 = 0, min2 = 1;
        if (nodes[min2]->freq < nodes[min1]->freq) { min1 = 1; min2 = 0; }

        for (int i = 2; i < node_count; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1; min1 = i;
            }
            else if (nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        HuffmanNode* parent = (HuffmanNode*)calloc(1, sizeof(HuffmanNode));
        parent->freq = nodes[min1]->freq + nodes[min2]->freq;
        parent->left = nodes[min1];
        parent->right = nodes[min2];

        nodes[min1] = parent;
        nodes[min2] = nodes[node_count - 1];
        node_count--;
    }
    *root = nodes[0];
}

                                 // --- Recursive  finding  code lengths ---
void find_lengths(HuffmanNode* node, int depth, unsigned char* lengths) {
    if (!node) return;
    if (!node->left && !node->right) {
        lengths[node->symbol] = depth;
        return;
    }
    find_lengths(node->left, depth + 1, lengths);
    find_lengths(node->right, depth + 1, lengths);
}

                 // Generateing  Canonical Codes 
void generate_canonical_codes(HuffmanNode* root, HuffmanCode* codes) {
    unsigned char lengths[256] = { 0 };
    find_lengths(root, 0, lengths);

    unsigned short current_code = 0;
    // Capped at 16 to guarantee we never overflow the unsigned short struct!
    for (int length = 1; length <= 16; length++) {
        for (int symbol = 0; symbol < 256; symbol++) {
            if (lengths[symbol] == length) {
                codes[symbol].code = current_code++;
                codes[symbol].length = length;
            }
        }
        current_code <<= 1;
    }
}

                     // Write Header 
void write_header(HuffmanCode* codes, unsigned char* output, size_t* out_len) {
    for (int i = 0; i < 256; i++) {
        output[i] = codes[i].length;
    }
    *out_len = 256;
}

                   // - Encodeing  Data -
void encode_data(unsigned char* input, size_t len, HuffmanCode* codes, unsigned char* output, size_t* out_len) {
    BitStream stream = { output, 0, 0, 0 };

    memcpy(&output[0], &len, sizeof(size_t));
    stream.byte_pos += sizeof(size_t);

    for (size_t i = 0; i < len; i++) {
        unsigned char sym = input[i];
        write_bits(&stream, codes[sym].code, codes[sym].length);
    }

    if (stream.bits_filled > 0) {
        stream.current_byte <<= (8 - stream.bits_filled);
        stream.buffer[stream.byte_pos++] = stream.current_byte;
    }
    *out_len = stream.byte_pos;
}

                       // --- MASTER ENCODE function complete here---
void huffman_encode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len) {
    int frequencies[256] = { 0 };
    for (size_t i = 0; i < len; i++) frequencies[input[i]]++;

                         // BZIP2 DEPTH LIMITER 
    // If the total frequency is too high, the tree depth exceeds 15 bits.
    // We scale down the frequencies mathematically to keep the tree shallow and safe.
    int total_freq = 0;
    for (int i = 0; i < 256; i++) total_freq += frequencies[i];

    while (total_freq > 800) {
        total_freq = 0;
        for (int i = 0; i < 256; i++) {
            if (frequencies[i] > 1) {
                frequencies[i] /= 2; // Halve the frequency
            }
            total_freq += frequencies[i];
        }
    }

    HuffmanNode* root = NULL;
    build_huffman_tree(frequencies, &root);

    HuffmanCode codes[256] = { 0 };
    if (root) generate_canonical_codes(root, codes);

    size_t header_len = 0;
    write_header(codes, output, &header_len);

    size_t data_len = 0;
    encode_data(input, len, codes, output + header_len, &data_len);

    *out_len = header_len + data_len;
}

                     //  MASTER DECODEing here
void huffman_decode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len) {
    unsigned char lengths[256];
    for (int i = 0; i < 256; i++) {
        lengths[i] = input[i];
    }

    HuffmanCode codes[256] = { 0 };
    unsigned short current_code = 0;
    for (int length = 1; length <= 16; length++) {
        for (int symbol = 0; symbol < 256; symbol++) {
            if (lengths[symbol] == length) {
                codes[symbol].code = current_code++;
                codes[symbol].length = length;
            }
        }
        current_code <<= 1;
    }

    HuffmanNode* root = (HuffmanNode*)calloc(1, sizeof(HuffmanNode));
    for (int sym = 0; sym < 256; sym++) {
        if (codes[sym].length > 0) {
            HuffmanNode* curr = root;
            for (int b = codes[sym].length - 1; b >= 0; b--) {
                int bit = (codes[sym].code >> b) & 1;
                if (bit == 0) {
                    if (!curr->left) curr->left = (HuffmanNode*)calloc(1, sizeof(HuffmanNode));
                    curr = curr->left;
                }
                else {
                    if (!curr->right) curr->right = (HuffmanNode*)calloc(1, sizeof(HuffmanNode));
                    curr = curr->right;
                }
            }
            curr->symbol = (unsigned char)sym;
        }
    }

    size_t original_len;
    memcpy(&original_len, &input[256], sizeof(size_t));

    BitStream stream = { input, 256 + sizeof(size_t), 0, 0 };

    for (size_t i = 0; i < original_len; i++) {
        HuffmanNode* curr = root;
        while (curr->left || curr->right) {
            int bit = read_bit(&stream);
            if (bit == 0) curr = curr->left;
            else curr = curr->right;

                               //  SAFETY CHECK  
                         // Stops Visual Studio from throwing an access violation if a bit is flipped
            if (curr == NULL) break;
        }
        if (curr) {
            output[i] = curr->symbol;
        }
        else {
            output[i] = 0;                               // Safe fallback
        }
    }
    *out_len = original_len;
}