/**
 * @file bit_array.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 09/04/2024
 * @brief Implementation for `bit_array.h`
 */

#include "bit_array.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

#define BYTE_CHUNK 10

BitArray bit_array_new(uint8_t *bytes, size_t len) {
    BitArray arr = {0};

    if (len) {
        arr.data = malloc(len);

        if (!arr.data) {
            set_error(Error_OutOfMemory);
            return arr;
        }

        memcpy(arr.data, bytes, len);
        arr.len = len * 8;
        arr.capacity = len;
    }

    return arr;
}

void bit_array_free(BitArray *arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }

    arr->len = 0;
    arr->cursor = 0;
    arr->capacity = 0;
}

size_t bit_array_bit_len(BitArray *arr) {
    return arr->len;
}

size_t bit_array_byte_len(BitArray *arr) {
    return (arr->len / 8) + (bool)(arr->len % 8);
}

void bit_array_realloc(BitArray *arr, size_t capacity) {
    uint8_t *new_data = realloc(arr->data, capacity);

    if (!new_data) {
        fprintf(stderr, "ERR bit array: Cannot allocate memory\n");
        set_error(Error_OutOfMemory);
        return;
    }

    /// Set new data to 0
    size_t byte_index = arr->len / 8;
    memset(new_data + byte_index, 0, capacity - byte_index);

    arr->data = new_data;
    arr->capacity = capacity;
}

void bit_array_push(BitArray *arr, bool is_one) {
    logfmt("Pushing %d", is_one);
    size_t byte_index = arr->len / 8;
    size_t bit_index = arr->len % 8;

    if (bit_index == 0 && byte_index >= arr->capacity) {
        bit_array_realloc(arr, arr->capacity + BYTE_CHUNK);
        if (got_error()) return;
    }

    if (is_one) {
        arr->data[byte_index] |= (1 << bit_index);
    }

    arr->len += 1;
}

void bit_array_push_n(BitArray *arr, uint64_t data, size_t n) {
    if (n > 64) {
        set_error(Error_IndexOutOfBound);
        return;
    }

    // while (n) {
    //     bit_array_push(arr, data & 1);
    //     data >>= 1;
    //     n -= 1;
    // }

    /// Allocate enough memory to store the new data
    size_t len = arr->len + n;
    size_t expected_bytes = len / 8 + (bool)(len % 8);
    if (expected_bytes > arr->capacity) {
        bit_array_realloc(arr, expected_bytes + BYTE_CHUNK);
        if (got_error()) {
            return;
        }
    }

    size_t byte_index = arr->len / 8;
    size_t bit_index = arr->len % 8;

    /// Fill up the remaining bits of the byte
    size_t avaiable_bits = 8 - bit_index;
    if (avaiable_bits != 8) {
        size_t nof_bits = avaiable_bits;

        if (n < avaiable_bits) {
            nof_bits = n;
        }

        uint8_t mask = (1 << nof_bits) - 1;
        arr->data[byte_index++] |= (data & mask) << bit_index;
        
        data >>= nof_bits;
        n -= nof_bits;
    }

    /// Freely push byte into the array without worring of overwriting data
    while (n != 0) {
        size_t nof_bits = 8;
        uint8_t mask = 0xFF;

        if (n < 8) {
            nof_bits = n;
            mask = (1 << n) - 1;
        }

        arr->data[byte_index++] |= data & mask;
        
        data >>= nof_bits;
        n -= nof_bits;
    }
    
    arr->len = len;

}

void bit_array_pad_to_byte(BitArray *arr) {
    size_t remaining_bits = arr->len % 8;
    if (remaining_bits == 0) {
        return;
    }

    // Calculate number of padding bits needed
    size_t padding_bits = 8 - remaining_bits;

    for (size_t i = 0; i < padding_bits; i++) {
        bit_array_push(arr, false);
    }
}

void bit_array_concat(BitArray *arr, BitArray *other) {
    size_t len = other->len;

    while (len) {
        size_t nof_bits = 16;
        if (len < nof_bits) {
            nof_bits = len;
        }

        uint64_t val = bit_array_read_n(other, nof_bits);
        bit_array_push_n(arr, val, nof_bits);
        len -= nof_bits;
    }
}

int bit_array_read(BitArray *arr) {
    if (arr->cursor >= arr->len) {
        set_error(Error_IndexOutOfBound);
        return 2; // End of bit array
    }

    int byte_index = arr->cursor / 8;
    int bit_index = arr->cursor % 8;

    arr->cursor++;

    uint8_t byte = arr->data[byte_index];

    int result = (byte & (1 << bit_index)) != 0;

    logfmt("bit_array_read: arr %d bit %d: %d", byte_index, bit_index, result);

    return result;
    // return ((byte >> bit_index) & 1) != 0;
}

uint64_t bit_array_read_n(BitArray *arr, size_t n) {
    if (n > 64) {
        set_error(Error_InternalError);
        fprintf(stderr, "Error: Get more than 64 bits at a time\n");
        return 0;
    }

    uint64_t result = 0;
    for (size_t i = 0; i < n; i++) {
        int bit = bit_array_read(arr);
        if (bit == 2) {
            // End of bit array
            return result;
        }

        result |= (bit << i);
    }

    logfmt("bit_array_read_n: got %ld", result);

    return result;
}

void bit_array_set_one_at(BitArray *arr, size_t index) {
    if (index >= arr->len) {
        set_error(Error_IndexOutOfBound);
        return;
    }

    size_t byte_index = index / 8;
    size_t bit_index = index % 8;

    arr->data[byte_index] |= (1 << bit_index);
}
