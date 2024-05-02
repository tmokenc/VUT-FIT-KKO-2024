/**
 * @file image.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Defines data structure and functions to work with image buffer.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdio.h>
#include "bit_array.h"

typedef enum {
    Transposition_Vertical,
    Transposition_Zigzag,
} Transposition;

typedef enum {
    Serialization_Vertical,
    Serialization_Zigzag,
} Serialization;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} Image;

Image image_new(uint32_t width, uint32_t height);

Image image_from_raw(uint8_t *bytes, uint32_t width, uint32_t height);

void image_free(Image *image);

uint64_t image_size(Image *image);

uint16_t image_number_of_blocks(Image *image, int block_size);

Image image_get_block(Image *image, int block_index, int block_size);

void image_insert_block(Image *image, Image *block, int block_index, int block_size);

/// The length is the size of image
uint8_t *image_serialization(Image *image, Serialization strategy);

Image image_deserialization(uint8_t *bytes, uint32_t width, uint32_t height, Serialization strategy);

#endif
