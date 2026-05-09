#include "project.h"

void rle1_encode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len) {
    size_t i = 0, j = 0;
    while (i < len) {
        unsigned char current = input[i];
        size_t run = 1;

        // Count consecutive identical bytes
        while (i + run < len && input[i + run] == current && run < 259)
            run++;

        if (run >= 4) {
            // Write 4 copies + extra count byte (extra = run - 4, max 255)
            size_t extra = run - 4;
            if (extra > 255) extra = 255;
            output[j++] = current;
            output[j++] = current;
            output[j++] = current;
            output[j++] = current;
            output[j++] = (unsigned char)extra;
            i += 4 + extra; // advance by actual encoded run
        }
        else {
            // Write as-is
            for (size_t k = 0; k < run; k++)
                output[j++] = input[i + k];
            i += run;
        }
    }
    *out_len = j;
}
void rle1_decode(unsigned char* input, size_t len, unsigned char* output, size_t* out_len) {
    size_t i = 0, j = 0;
    int consecutive = 0;
    unsigned char prev = 0;

    while (i < len) {
        unsigned char c = input[i++];
        output[j++] = c;

        if (consecutive == 3 && c == prev) {
            // We just wrote the 4th identical byte in a row
                                        // Next byte is the extra count
            unsigned char extra = input[i++];
            for (int k = 0; k < (int)extra; k++)
                output[j++] = c;
            consecutive = 0;            // reset after consuming the run
            prev = 0;        // force reset so next char starts fresh
        }
        else {
            if (c == prev)
                consecutive++;
            else {
                consecutive = 1;
                prev = c;
            }
        }
    }
    *out_len = j;
}


 // Identical format to standard RLE-1, but threshold is read from config.ini
 // Standard RLE-1 hardcodes threshold=4, this makes it configurable.


void rle1_encode_threshold(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len,
    int threshold) {
    size_t i = 0, j = 0;
    if (threshold < 4) threshold = 4; /* BZip2 minimum is 4 */

    while (i < len) {
        unsigned char cur = input[i];
        size_t run = 1;

        // Count consecutive identical bytes (Max run is threshold + 255)
        while (i + run < len && input[i + run] == cur && run < (size_t)(threshold + 255))
            run++;

        if ((int)run >= threshold) {     // Calculate how many are left over
            size_t extra = run - threshold;
            // Write exactly 'threshold' copies to act as the trigger
            for (int k = 0; k < threshold; k++) {
                output[j++] = cur;
            }
            // Write the remaining amount as a single number byte
            output[j++] = (unsigned char)extra;
            i += threshold + extra;   // Skip past the characters we just processed
        }
        else {
            // Write uncompressed run
            for (size_t k = 0; k < run; k++) output[j++] = input[i + k];
            i += run;
        }
    }
    *out_len = j;
}

/* Updated Decoder: Now uses the threshold to know when to trigger! */
void rle1_decode_threshold(unsigned char* input, size_t len,
    unsigned char* output, size_t* out_len, int threshold) {
    size_t i = 0, j = 0;
    int consecutive = 0;
    unsigned char prev = 0;
    if (threshold < 4) threshold = 4;

    while (i < len) {
        unsigned char c = input[i++];
        output[j++] = c;

        // Trigger ONLY when we hit the exact threshold number of identical bytes
        if (consecutive == threshold - 1 && c == prev) { 
            if (i < len) { // Safety bounds check
                unsigned char extra = input[i++];
                for (int k = 0; k < (int)extra; k++)
                    output[j++] = c;
            }
            consecutive = 0; // Reset after consuming
            prev = 0;        // Force reset
        }
        else {
            if (c == prev)
                consecutive++;
            else {
                consecutive = 1;
                prev = c;
            }
        }
    }
    *out_len = j;
}