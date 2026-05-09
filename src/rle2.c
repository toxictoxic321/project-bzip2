#include "project.h"

/* Encodes MTF output using specialized zero-run RLE */
void rle2_encode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len) {
    size_t i = 0, j = 0;
    while (i < len) {
        if (input[i] == 0) {
            size_t zero_count = 0;
            // Count consecutive zeros (max 255 per run to fit in one byte)
            while (i < len && input[i] == 0 && zero_count < 255) {
                zero_count++;
                i++;
            }                           //here counting zeros as only no of zeros are compressed and sotre count as (0,3)  for 0005
            output[j++] = 0; // Marker for a run of zeros                      
            output[j++] = (unsigned char)zero_count; // How many zeros
        }
        else {
            output[j++] = input[i++]; // Write non-zeros exactly as they are
        }
    }
    *out_len = j;
}

/* Decodes RLE-2 encoded data back into MTF data */
void rle2_decode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len) {
    size_t i = 0, j = 0;
    while (i < len) {
        if (input[i] == 0) {
            i++; // Skip the 0 marker
            unsigned char count = input[i++]; // Read the count
            for (int k = 0; k < count; k++) {
                output[j++] = 0; // Write that many zeros
            }
        }                             //and if non zero no comes write it as it is in the output array and move on thats it for rle22222
        else {
            output[j++] = input[i++];
        }
    }
    *out_len = j; // This should end up equaling the original MTF length
}