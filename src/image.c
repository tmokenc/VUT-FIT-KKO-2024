/**
 * @file image.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Implementation for `image.h`
 */

#include "image.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

int coord_to_index(Image *image, int x, int y) {
    return y * image->width + x;
}

void block_offset(Image *image, int block_index, int block_size, int *x, int *y) {
    int block_per_row = image->width / block_size;
    if (image->width % block_size) block_per_row += 1;
    *x = (block_index % block_per_row) * block_size;
    *y = (block_index / block_per_row) * block_size;
}

Image image_new(uint32_t width, uint32_t height) {
    Image image = {
        .width = width,
        .height = height,
        .data = NULL
    };

    if (!width || !height) {
        fprintf(stderr, "ERR image: %dx%d is invalid size of an image\n", width, height);
        set_error(Error_InvalidImageSize);
    }

    image.data = malloc(image_size(&image));

    if (!image.data) {
        fprintf(stderr, "ERR image: Cannot create image size of %dx%d\n", width, height);
        set_error(Error_OutOfMemory);
    }

    return image;
}

Image image_from_raw(uint8_t *bytes, uint32_t width, uint32_t height) {
    Image image = {
        .width = width,
        .height = height,
        .data = bytes,
    };

    if (!width || !height) {
        fprintf(stderr, "ERR image: %dx%d is invalid size of an image\n", width, height);
        set_error(Error_InvalidImageSize);
    }

    return image;
}

void image_free(Image *image) {
    if (image->data) {
        free(image->data);
        image->data = NULL;
        image->width = 0;
        image->height = 0;
    }
}

uint64_t image_size(Image *image) {
    return (uint64_t)image->height * (uint64_t)image->width;
}

uint16_t image_number_of_blocks(Image *image, int block_size) {
    if (block_size == 0) {
        set_error(Error_InvalidBlockSize);
        return 0;
    }

    int horizontal = image->width / block_size;
    int vertical = image->height / block_size;

    if (image->width % block_size) horizontal += 1;
    if (image->height % block_size) vertical += 1;

    return horizontal * vertical;
}

Image image_get_block(Image *image, int block_index, int block_size) {
    int x, y;
    Image block;
    block_offset(image, block_index, block_size, &x, &y);

    if (got_error()) return block;

    int width = image->width - x;
    int height = image->height - y;

    if (width  > block_size) width  = block_size;
    if (height > block_size) height = block_size;

    logfmt("Block #%d start at (%d, %d) with dimension %dx%d", block_index, x, y, width, height);

    block = image_new(width, height);

    if (got_error()) {
        image_free(&block);
        return block;
    }

    for (int ny = 0; ny < height; ny++) {
        uint8_t *dst = block.data + (ny * width);
        uint8_t *src = image->data + coord_to_index(image, x, y + ny);
        memcpy(dst, src, width);
    }

    return block;
}

void image_insert_block(Image *image, Image *block, int block_index, int block_size) {
    int x, y;
    block_offset(image, block_index, block_size, &x, &y);

    for (size_t ny = 0; ny < block->height; ny++) {
        uint8_t *dst = image->data + coord_to_index(image, x, y + ny);
        uint8_t *src = block->data + (ny * block->width);
        memcpy(dst, src, block->width);
    }
}

uint8_t *image_serialization(Image *image, Serialization strategy) {
    size_t size = image_size(image);
    uint8_t *bytes = (uint8_t *)malloc(size);

    if (!bytes) {
        set_error(Error_OutOfMemory);
        return bytes;
    }

    for (size_t i = 0; i < size; i++) {
        size_t pixel_index = 0;

        switch (strategy) {
            case Serialization_Vertical: {
                int x = i / image->height;
                int y = i % image->height;

                pixel_index = y * image->width + x;
                break;
            }

            case Serialization_Zigzag: {
                // TODO
            }
        }

        logfmt("Seralization: mapping %ld -> %ld", i, pixel_index);
        bytes[i] = image->data[pixel_index];
    }

    return bytes;
}

Image image_deserialization(uint8_t *bytes, uint32_t width, uint32_t height, Serialization strategy) {
    Image image = image_new(width, height);
    if (got_error()) return image;

    uint64_t size = image_size(&image);

    for (size_t i = 0; i < size; i++) {
        size_t pixel_index = 0;

        switch (strategy) {
            case Serialization_Vertical: {
                int x = i / width;
                int y = i % width;

                pixel_index = y * height + x;
                break;
            }

            case Serialization_Zigzag: {
                // TODO
            }
        }

        logfmt("Deseralization: mapping %ld -> %ld", i, pixel_index);
        image.data[i] = bytes[pixel_index];
    }

    return image;
}
