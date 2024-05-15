/**
 * @file compressor.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Header file for image compression and decompression functions.
 */

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "bit_array.h"
#include "args.h"
#include "image.h"

/**
 * @brief Compresses an image using specified encoding techniques.
 * 
 * This function compresses the given image based on the options specified
 * in the arguments. It utilizes various preprocessing techniques and encodes
 * the data using Huffman coding.
 * 
 * @param image Pointer to the Image structure to be compressed.
 * @param args Pointer to the Args structure containing compression options.
 * @return BitArray The compressed image data.
 */
BitArray compressor_image_compress(Image *image, Args *args);

/**
 * @brief Decompresses image data into an Image structure.
 * 
 * This function decompresses the provided byte array into an Image structure
 * based on the options specified in the arguments. It decodes the data using
 * Huffman coding and applies any required post-processing.
 * 
 * @param bytes Pointer to the compressed byte array.
 * @param len Length of the compressed byte array.
 * @param args Pointer to the Args structure containing decompression options.
 * @return Image The decompressed image.
 */
Image compressor_image_decompress(uint8_t *bytes, size_t len, Args *args);

#endif
