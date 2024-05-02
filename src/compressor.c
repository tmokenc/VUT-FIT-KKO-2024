/**
 * @file compressor.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Implementation for `compressor.h`
 */

#include "compressor.h"
#include "transform.h"
#include "rle.h"
#include "huffman.h"
#include <stdlib.h>

typedef enum {
    CompressionType_None,
    CompressionType_Vertical,
    CompressionType_Horizontal,
    CompressionType_Zigzag,
} CompressionType;

void transform(uint8_t *bytes, size_t len);
void revert_transform(uint8_t *bytes, size_t len);

BitArray prehuffman_compress(uint8_t *bytes, size_t size, bool should_transform) {
    if (should_transform) {
        transform(bytes, size);
    }

    return rle_encode(bytes, size);
}

void compress_block(Image *block, bool should_transform, BitArray *output, BitArray *metadata) {
     uint64_t size = image_size(block);
     uint8_t *vertical = image_serialization(block, Serialization_Vertical);
     BitArray vertical_data = prehuffman_compress(vertical, size, should_transform);
     free(vertical);

     uint8_t *zigzag = image_serialization(block, Serialization_Zigzag);
     BitArray zigzag_data = prehuffman_compress(zigzag, size, should_transform);
     free(zigzag);

     BitArray horizontal_data = prehuffman_compress(block->data, size, should_transform);

     CompressionType type = CompressionType_None;
     BitArray res;
     size_t min_val = image_size(block);

     if (vertical_data.len < min_val) {
         min_val = vertical_data.len;
         type = CompressionType_Vertical;
     }

     if (horizontal_data.len < min_val) {
         min_val = horizontal_data.len;
         type = CompressionType_Horizontal;
     }

     if (zigzag_data.len < min_val) {
         min_val = zigzag_data.len;
         type = CompressionType_Zigzag;
     }

     switch (type) {
         case CompressionType_None:
             res = bit_array_new(block->data, image_size(block));
             bit_array_free(&vertical_data);
             bit_array_free(&horizontal_data);
             bit_array_free(&zigzag_data);
             break;
         case CompressionType_Horizontal:
             res = horizontal_data;
             bit_array_free(&vertical_data);
             bit_array_free(&zigzag_data);
             break;
         case CompressionType_Vertical:
             res = vertical_data;
             bit_array_free(&zigzag_data);
             bit_array_free(&horizontal_data);
             break;
         case CompressionType_Zigzag:
             res = zigzag_data;
             bit_array_free(&vertical_data);
             bit_array_free(&horizontal_data);
             break;
     }

     // -> write type
     bit_array_push_n(metadata, type, 2);
     bit_array_concat(output, &res);
     bit_array_free(&res);
}

BitArray compressor_image_compress(Image *image, Args *args) {
    BitArray result = bit_array_new(NULL, 0);

    bit_array_push_n(&result, (unsigned)image->width, 16);
    bit_array_push_n(&result, (unsigned)image->height, 16);

    if (args->image_adaptive) {
        BitArray blocks_metadata = bit_array_new(NULL, 0);
        BitArray blocks_data = bit_array_new(NULL, 0);
        uint16_t nof_blocks = image_number_of_blocks(image, args->block_size);
        for (uint16_t i = 0; i < nof_blocks; i++) {
            Image block = image_get_block(image, i, args->block_size);
            compress_block(&block, args->transformace_data, &blocks_data, &blocks_metadata);
            image_free(&block);
        }

        bit_array_pad_to_byte(&blocks_metadata);

        bit_array_concat(&result, &blocks_metadata);
        bit_array_concat(&result, &blocks_data);
        bit_array_free(&blocks_metadata);
        bit_array_free(&blocks_data);
    } else {
        BitArray data = prehuffman_compress(image->data, image_size(image), args->transformace_data);
        bit_array_concat(&result, &data);
        bit_array_free(&data);
    }

    BitArray huffman = huffman_compress(result.data, bit_array_byte_len(&result));
    bit_array_free(&result);

    return huffman;
}

Image compressor_image_decompress(BitArray *bits, Args *args) {
    uint16_t width  = bit_array_read_n(bits, 16);
    uint16_t height = bit_array_read_n(bits, 16);

    Image image = image_new(width, height);

    if (args->image_adaptive) {
        // TODO
    } else {
        // TODO

        int size = image_size(&image);
        int index = 0;

        while (index < size - 1) {
            // RLE decompress


        }
    }

    return image;
}
