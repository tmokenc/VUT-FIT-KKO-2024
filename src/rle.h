/**
 * @file rle.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Defines Run-Length Encoding (RLE) functions
 */

#ifndef RLE_H
#define RLE_H

#include "bit_array.h"

/**
 * @brief Encodes the input data using Run-Length Encoding (RLE).
 *
 * @param bytes Pointer to the array of bytes representing the input data.
 * @param len The length of the input data array.
 * @return BitArray A BitArray object representing the RLE-encoded data.
 */
BitArray rle_encode(uint8_t *bytes, size_t len);

/**
 * @brief Decodes the input RLE-encoded data back to its original form.
 *
 * @param bytes Pointer to the array of bytes representing the RLE-encoded data.
 * @param len The length of the RLE-encoded data array.
 * @param output Output data, the address should be large enough to store the decoded data.
 * @param output_len Length of the expected output data.
 * @return The length of decoded data
 */
size_t rle_decode(uint8_t *bytes, size_t len, uint8_t *output, size_t output_len);

#endif
