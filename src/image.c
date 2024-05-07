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

enum direction {
    Direction_Right,
    Direction_Left,
    Direction_Up,
    Direction_Down,
};

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

    switch (strategy) {
        case Serialization_Vertical: {
            for (size_t i = 0; i < size; i++) {
                size_t x = i / image->height;
                size_t y = i % image->height;
                size_t pixel_index = y * image->width + x;
                logfmt("Seralization: mapping %ld -> %ld", i, pixel_index);
                bytes[i] = image->data[pixel_index];
            }

            break;
        }

        case Serialization_Circular: {
            /// It is going in the direction right -> down -> left -> up
            /// After each transition, the step is -= 2
            size_t right = image->width;
            size_t down = image->height - 1;
            size_t left = image->width - 1;
            size_t up = image->height - 2;

            enum direction direction = Direction_Right;
            size_t *nof_step = &right;
            size_t processed = 1;

            /// first byte is always the same
            bytes[0] = image->data[0];

            long step = 1;
            long index = -1;

            while (processed < size) {
                for (size_t i = 0; i < *nof_step; i++) {
                    index += step;
                    logfmt("%ld -> %ld\n", processed, index);
                    bytes[processed++] = image->data[index];
                }

                *nof_step -= 2;

                switch (direction) {
                    case Direction_Right:
                        direction = Direction_Down;
                        nof_step = &down;
                        step = image->width;
                        break;
                    case Direction_Left:
                        direction = Direction_Up;
                        nof_step = &up;
                        step = image->width;
                        step = -step;
                        break;
                    case Direction_Up:
                        direction = Direction_Right;
                        nof_step = &right;
                        step = 1;
                        break;
                    case Direction_Down:
                        direction = Direction_Left;
                        nof_step = &left;
                        step = -1;
                        break;
                }
            }

            break;
        }
    }

    return bytes;
}

Image image_deserialization(uint8_t *bytes, uint32_t width, uint32_t height, Serialization strategy) {
    Image image = image_new(width, height);
    if (got_error()) return image;

    uint64_t size = image_size(&image);

    switch (strategy) {
        case Serialization_Vertical: {
            for (size_t i = 0; i < size; i++) {
                size_t x = i / width;
                size_t y = i % width;

                size_t pixel_index = y * height + x;
                logfmt("Deseralization: mapping %ld -> %ld", i, pixel_index);
                image.data[i] = bytes[pixel_index];
            }
            break;

        }

        /// This is the same as serialization, just swap the data storing part
        case Serialization_Circular: {
            size_t right = image.width;
            size_t down = image.height - 1;
            size_t left = image.width - 1;
            size_t up = image.height - 2;

            enum direction direction = Direction_Right;
            size_t *nof_step = &right;
            size_t processed = 1;

            image.data[0] = bytes[0];

            long step = 1;
            long index = -1;

            while (processed < size) {
                for (size_t i = 0; i < *nof_step; i++) {
                    index += step;
                    logfmt("%ld -> %ld\n", index, processed);
                    image.data[index] = bytes[processed++];
                }

                *nof_step -= 2;

                switch (direction) {
                    case Direction_Right:
                        direction = Direction_Down;
                        nof_step = &down;
                        step = image.width;
                        break;
                    case Direction_Left:
                        direction = Direction_Up;
                        nof_step = &up;
                        step = image.width;
                        step = -step;
                        break;
                    case Direction_Up:
                        direction = Direction_Right;
                        nof_step = &right;
                        step = 1;
                        break;
                    case Direction_Down:
                        direction = Direction_Left;
                        nof_step = &left;
                        step = -1;
                        break;
                }
            }
            // TODO
            break;
        }
    }

    return image;
}

