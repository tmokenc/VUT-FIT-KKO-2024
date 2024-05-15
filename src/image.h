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

/**
 * @brief Enumeration for different serialization strategies.
 */
typedef enum {
    Serialization_Vertical,
    Serialization_Circular,
} Serialization;

/**
 * @brief Structure representing an image.
 */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} Image;

/**
 * @brief Creates a new image with the specified width and height.
 * 
 * @param width Width of the new image.
 * @param height Height of the new image.
 * @return Image The new image.
 */
Image image_new(uint32_t width, uint32_t height);

/**
 * @brief Creates an image from raw byte data with the specified width and height.
 * 
 * @param bytes Pointer to the raw byte data.
 * @param width Width of the image.
 * @param height Height of the image.
 * @return Image The new image.
 */
Image image_from_raw(uint8_t *bytes, uint32_t width, uint32_t height);

/**
 * @brief Frees the memory allocated for an image.
 * 
 * @param image Pointer to the image to be freed.
 */
void image_free(Image *image);

/**
 * @brief Calculates the size of an image in bytes.
 * 
 * @param image Pointer to the image.
 * @return uint64_t Size of the image in bytes.
 */
uint64_t image_size(Image *image);

/**
 * @brief Calculates the number of blocks in an image for a given block size.
 * 
 * @param image Pointer to the image.
 * @param block_size Size of each block.
 * @return uint16_t Number of blocks.
 */
uint16_t image_number_of_blocks(Image *image, int block_size);

/**
 * @brief Retrieves a block from an image.
 * 
 * @param image Pointer to the image.
 * @param block_index Index of the block.
 * @param block_size Size of the block.
 * @return Image The extracted block.
 */
Image image_get_block(Image *image, int block_index, int block_size);

/**
 * @brief Inserts a block into an image at the specified index.
 * 
 * @param image Pointer to the image.
 * @param block Pointer to the block to be inserted.
 * @param block_index Index at which to insert the block.
 * @param block_size Size of the block.
 */
void image_insert_block(Image *image, Image *block, int block_index, int block_size);

/**
 * @brief Serializes an image using the specified strategy.
 * 
 * @param image Pointer to the image.
 * @param strategy Serialization strategy.
 * @return uint8_t* Pointer to the serialized image data.
 */
uint8_t *image_serialization(Image *image, Serialization strategy);

/**
 * @brief Deserializes image data into an Image structure.
 * 
 * @param bytes Pointer to the serialized image data.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param strategy Serialization strategy used.
 * @return Image The deserialized image.
 */
Image image_deserialization(uint8_t *bytes, uint32_t width, uint32_t height, Serialization strategy);

#endif
