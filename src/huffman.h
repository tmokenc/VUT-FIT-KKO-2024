/**
 * @file huffman.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Header file for Canonical Huffman coding.
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "bit_array.h"

/**
 * @brief Compresses data using Canonical Huffman coding.
 * @param bytes Pointer to the byte array to be compressed.
 * @param len Length of the byte array.
 * @return BitArray The compressed data.
 */
BitArray huffman_compress(uint8_t *bytes, size_t len);

/**
 * @brief Decompresses data compressed using Canonical Huffman coding.
 * @param bytes Pointer to the compressed byte array.
 * @param len Length of the compressed byte array.
 * @return BitArray The decompressed data.
 */
BitArray huffman_decompress(uint8_t *bytes, size_t len);

#endif
