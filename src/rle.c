/**
 * @file rle.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Implementation of `rle.h`
 */

#include "rle.h"
#include "error.h"
#include <string.h>

BitArray rle_encode(uint8_t *bytes, size_t len) { 
    BitArray result = bit_array_new(NULL, 0); 
    if (!len) return result;
    bool has_previous = false;
    uint8_t previous;
    uint16_t count = 0;
    size_t index = 0;

    for (size_t i = 0; i < len; i++) {
        uint8_t byte = bytes[i];

        if (!has_previous) {
            logfmt("Start new byte %d", byte);
            previous = byte;
            has_previous = true;
            count = 0;

            if (index % 8 == 0) {
                index = bit_array_bit_len(&result);
                bit_array_push_n(&result, 0, 8); // For metadata
            }

            continue;
        }

        if (byte == previous) {
            count += 1;

            /// Reach maximum number of count allowed
            if (count == 0xFF + 1) {
                logfmt("Insert Index %ld (%ld) byte %d count %d", index, index % 8, byte, count);
                bit_array_set_one_at(&result, index);
                bit_array_push_n(&result, count - 1, 8);
                bit_array_push_n(&result, previous, 8);
                has_previous = false;
                index += 1;
            }
        } else {
            logfmt("Insert Index %ld (%ld) byte %d count %d", index, index % 8, byte, count);
            if (count) {
                bit_array_set_one_at(&result, index);
                bit_array_push_n(&result, count - 1, 8);
            }

            bit_array_push_n(&result, previous, 8);
            has_previous = false;
            index += 1;
            i -= 1; // Redo this loop
        }
    }

    /// Push the remaining into the output
    if (has_previous) {
        if (count) {
            bit_array_set_one_at(&result, index);
            bit_array_push_n(&result, count, 8);
        }

        bit_array_push_n(&result, previous, 8);
    }

    return result;
}

size_t rle_decode(uint8_t *bytes, size_t len, uint8_t *output, size_t output_len) {
    size_t output_index = 0;
    size_t i = 0;

    while (i < len) {
        uint8_t metadata = bytes[i++];
        logfmt("RLE decoding with metadata %d", metadata);
        for (int j = 0; j < 8 && i < len; j++) {
            uint16_t repeat = 1; // 16bit number because it can be up to 257 times

            if (metadata & 1) {
                repeat = bytes[i++] + 2;
                if (i == len) {
                    set_error(Error_IndexOutOfBound);
                    return i;
                }
            }

            uint8_t byte = bytes[i++];

            logfmt("RLE pushing %d %d times", byte, repeat);

            // Pushing bytes into the result
            for (int k = 0; k < repeat; k++) {
                output[output_index++] = byte;
            }

            if (output_index >= output_len) {
                return i;
            }

            metadata >>= 1;
        }
    }

    return i;
}
