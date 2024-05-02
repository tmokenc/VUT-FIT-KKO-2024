/**
 * @file bit_array.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 09/04/2024
 * @brief Data structure for manipulating array of bits
 */

#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Structure representing an array of bits.
 */
typedef struct {
    uint8_t *data; /**< Pointer to the underlying byte array */
    size_t len; /**< Length of the bit array */
    size_t cursor; /**< Current bit cursor within the bit array */
    size_t capacity; /**< Capacity in byte. */
} BitArray;

/**
 * @brief Creates a new bit array from the given byte array.
 *
 * @param bytes Pointer to the byte array.
 * @param len Length of the byte array.
 * @return BitArray structure representing the bit array.
 */
BitArray bit_array_new(uint8_t *bytes, size_t len);

/**
 * @brief Frees memory allocated for the given bit array.
 *
 * @param arr Pointer to the BitArray structure to be freed.
 */
void bit_array_free(BitArray *arr);

/**
 * @brief Returns the total number of bits in the bit array.
 *
 * @param arr Pointer to the BitArray structure.
 * @return Total number of bits in the bit array.
 */
size_t bit_array_bit_len(BitArray *arr);

/**
 * @brief Returns the total number of bytes in the bit array.
 *
 * @param arr Pointer to the BitArray structure.
 * @return Total number of bytes in the bit array.
 */
size_t bit_array_byte_len(BitArray *arr);

/**
 * @brief Appends a single bit to the bit array.
 *
 * @param arr Pointer to the BitArray structure.
 * @param is_one Boolean indicating whether to append a 1 or a 0.
 */
void bit_array_push(BitArray *arr, bool is_one);

/**
 * @brief Appends the specified number of bits to the bit array from the given data.
 *
 * @param arr Pointer to the BitArray structure.
 * @param data An integer containing the bits to be appended upto 64 bits.
 * @param n Number of bits to be appended.
 * @note only use this with n is 8 with make it behave like a Byte array.
 */
void bit_array_push_n(BitArray *arr, uint64_t data, size_t n);

/**
 * @brief Pads the bit array with zeros to the nearest byte boundary.
 *
 * @param arr Pointer to the BitArray structure.
 */
void bit_array_pad_to_byte(BitArray *arr);

/**
 * @brief Concatenates the given bit array with the current bit array.
 *
 * @param arr Pointer to the BitArray structure.
 * @param other Pointer to the BitArray structure to be concatenated.
 */
void bit_array_concat(BitArray *arr, BitArray *other);

/**
 * @brief Reads a single bit from the bit array.
 *
 * @param arr Pointer to the BitArray structure.
 * @return 0 or 1 if a bit is successfully read, or 2 if the end of the bit array is reached.
 */
int bit_array_read(BitArray *arr);

/**
 * @brief Reads up to 64 bits from the bit array.
 *
 * @param arr Pointer to the BitArray structure.
 * @param n Number of bits to read (up to 64).
 * @return Unsigned 64-bit integer containing the read bits.
 */
uint64_t bit_array_read_n(BitArray *arr, size_t n);

/**
 * @brief Set a value into the bit array at a specific index to 1
 *
 * @param arr Pointer to the BitArray structure.
 * @param index Index of the bit
 * @note This will raise an error if the index is outside of range.
 */
void bit_array_set_one_at(BitArray *arr, size_t index);

#endif
