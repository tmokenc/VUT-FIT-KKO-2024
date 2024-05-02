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
 * @return BitArray A BitArray object representing the decoded original data.
 */
BitArray rle_decode(uint8_t *bytes, size_t len);

#endif
