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
    Direction_Down,
    Direction_Left,
    Direction_Up,
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

    if (got_error()) return block;

    for (int ny = 0; ny < height; ny++) {
        uint8_t *dst = block.data + (ny * width);
        uint8_t *src = image->data + coord_to_index(image, x, y + ny);
        memcpy(dst, src, width);
    }

    log("Getting block done");

    return block;
}

void image_insert_block(Image *image, Image *block, int block_index, int block_size) {
    int x, y;
    block_offset(image, block_index, block_size, &x, &y);
    logfmt("Insert block %d with size %d into %dx%d", block_index, block_size, x, y);

    for (size_t ny = 0; ny < block->height; ny++) {
        uint8_t *dst = image->data + coord_to_index(image, x, y + ny);
        uint8_t *src = block->data + (ny * block->width);
        memcpy(dst, src, block->width);
    }

    log("Insert block done");
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
            int width = image->width;
            int top = 0, bottom = image->height - 1;
            int left = 0, right = image->width - 1;
            enum direction dir = Direction_Right; 
            int index = 0;
        
            while (top <= bottom && left <= right) {
                switch (dir) {
                    case Direction_Right:
                        memcpy(bytes + index, image->data + top * width + left, right - left + 1);
                        index += right - left + 1;
                        top++;

                        /// Naivee version of the above
                        // for (int i = left; i <= right; i++) {
                        //     bytes[index++] = image->data[top * width + i];
                        // }
                        break;
                    case Direction_Down:
                        for (int i = top; i <= bottom; i++) {
                            bytes[index++] = image->data[i * width + right];
                        }

                        right--;
                        break;
                    case Direction_Left:
                        for (int i = right; i >= left; i--) {
                            bytes[index++] = image->data[bottom * width + i];
                        }

                        bottom--;
                        break;
                    case Direction_Up:
                        for (int i = bottom; i >= top; i--) {
                            bytes[index++] = image->data[i * width + left];
                        }

                        left++;
                        break;
                }

                // 0: go right, 1: go down, 2: go left, 3: go up
                dir = (dir + 1) % 4;
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
            int top = 0, bottom = height - 1;
            int left = 0, right = width - 1;
            enum direction dir = Direction_Right; 
            int index = 0;
        
            while (top <= bottom && left <= right) {
                switch (dir) {
                    case Direction_Right:
                        memcpy(image.data + top * width + left, bytes + index, right - left + 1);
                        index += right - left + 1;
                        top++;
                        break;
                    case Direction_Left:
                        for (int i = right; i >= left; i--) {
                            image.data[bottom * width + i] = bytes[index++];
                        }

                        bottom--;
                        break;
                    case Direction_Down:
                        for (int i = top; i <= bottom; i++) {
                            image.data[i * width + right] = bytes[index++];
                        }

                        right--;
                        break;
                    case Direction_Up:
                        for (int i = bottom; i >= top; i--) {
                            image.data[i * width + left] = bytes[index++];
                        }

                        left++;
                        break;
                }
                dir = (dir + 1) % 4;
            }

            break;
        }
    }

    return image;
}

