#include "project.h"

/* Forward MTF transform */
void mtf_encode(unsigned char* input, size_t len, unsigned char* output) {
    unsigned char palette[256];

    // Initialize palette with 0-255
    for (int i = 0; i < 256; i++) {
        palette[i] = (unsigned char)i;                  //so assigning the cuurent list 0-255 total 256 chracters ...
    }

    for (size_t i = 0; i < len; i++) {
        unsigned char target = input[i];      
        int index = 0;

        // Find character index
        for (int j = 0; j < 256; j++) {                   //if like the input symbol matches the list then index in output 
            if (palette[j] == target) {
                index = j;                                   //and move the sybmbol to first index in list
                break;
            }
        }

        output[i] = (unsigned char)index; // Output the index

        // Shift elements right and move target to the front
        for (int k = index; k > 0; k--) {
            palette[k] = palette[k - 1];
        }
        palette[0] = target;
    }
}

/* Inverse MTF transform */
void mtf_decode(unsigned char* input, size_t len, unsigned char* output) {
    unsigned char palette[256];

    // Initialize palette with 0-255
    for (int i = 0; i < 256; i++) {
        palette[i] = (unsigned char)i;                //again making up the list of chracters
    }

    for (size_t i = 0; i < len; i++) {
        int index = input[i];
        unsigned char target = palette[index];                         //thaking the input index then ouput the symbol on that index

        output[i] = target; // Restore the original character

        // Shift elements right and move target to the front
        for (int k = index; k > 0; k--) {
            palette[k] = palette[k - 1];                               //then moving the symbol to the index 0 of the list 
        }
        palette[0] = target;
    }
}