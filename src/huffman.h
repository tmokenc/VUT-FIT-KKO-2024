/**
 * @file huffman.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Defines Canoncial Huffman coding
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "bit_array.h"

BitArray huffman_compress(uint8_t *bytes, size_t len);

BitArray huffman_decompress(uint8_t *bytes, size_t len);

#endif
