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
#include "error.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
    CompressionType_None,
    CompressionType_Vertical,
    CompressionType_Horizontal,
    CompressionType_Circular,
} CompressionType;


BitArray prehuffman_compress(uint8_t *bytes, size_t size, bool should_transform) {
    BitArray tmp = bit_array_new(bytes, size);
    if (got_error()) return tmp;

    if (should_transform) {
        transform(tmp.data, size);
    }

    BitArray result = rle_encode(tmp.data, size);
    bit_array_free(&tmp);
    return result;
}

size_t posthuffman_decompress(uint8_t *bytes, size_t size, bool should_transform, Image *output) {
    size_t img_size = image_size(output);
    size_t len = rle_decode(bytes, size, output->data, img_size);

    if (should_transform) {
        transform_revert(output->data, img_size);
    }

    return len;
}

void compress_block(Image *block, bool should_transform, BitArray *output, BitArray *metadata) {
     uint64_t size = image_size(block);
     uint8_t *vertical = image_serialization(block, Serialization_Vertical);
     BitArray vertical_data = prehuffman_compress(vertical, size, should_transform);
     free(vertical);

     uint8_t *circular = image_serialization(block, Serialization_Circular);
     BitArray circular_data = prehuffman_compress(circular, size, should_transform);
     free(circular);

     BitArray horizontal_data = prehuffman_compress(block->data, size, should_transform);

     CompressionType type = CompressionType_None;
     BitArray res;
     size_t min_val = image_size(block);

     logbytes("Data", block->data, image_size(block));

     if (vertical_data.len < min_val) {
         min_val = vertical_data.len;
         type = CompressionType_Vertical;
     }

     if (horizontal_data.len < min_val) {
         min_val = horizontal_data.len;
         type = CompressionType_Horizontal;
     }

     if (circular_data.len < min_val) {
         min_val = circular_data.len;
         type = CompressionType_Circular;
     }

     switch (type) {
         case CompressionType_None:
             res = bit_array_new(block->data, image_size(block));
             bit_array_free(&vertical_data);
             bit_array_free(&horizontal_data);
             bit_array_free(&circular_data);
             break;
         case CompressionType_Horizontal:
             res = horizontal_data;
             bit_array_free(&vertical_data);
             bit_array_free(&circular_data);
             break;
         case CompressionType_Vertical:
             res = vertical_data;
             bit_array_free(&circular_data);
             bit_array_free(&horizontal_data);
             break;
         case CompressionType_Circular:
             res = circular_data;
             bit_array_free(&vertical_data);
             bit_array_free(&horizontal_data);
             break;
     }

     logfmt("Compressed Type %d", type);
     logbytes("Compressed data", res.data, bit_array_byte_len(&res));
     // -> write type
     bit_array_push_n(metadata, type, 2);
     bit_array_concat(output, &res);
     bit_array_free(&res);
}

BitArray compressor_image_compress(Image *image, Args *args) {
    BitArray result = bit_array_new(NULL, 0);

    bit_array_push_n(&result, (unsigned)image->width - 1, 16);
    bit_array_push_n(&result, (unsigned)image->height - 1, 16);

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

Image compressor_image_decompress(uint8_t *bytes, size_t len, Args *args) {
    #define DECOMPRESS_ERROR_GUARD(func) func; \
        if (got_error()) {\
            bit_array_free(&bits); \
            image_free(&image); \
            bit_array_free(&block_metadata); \
            return image; \
        }

    BitArray bits = huffman_decompress(bytes, len);

    uint16_t width  = bit_array_read_n(&bits, 16) + 1;
    uint16_t height = bit_array_read_n(&bits, 16) + 1;

    /// 4 bytes of width and height of the image
    uint8_t *data = bits.data + 4;
    size_t data_len = bit_array_byte_len(&bits) - 4;
    Image image = image_new(width, height);
    if (got_error()) {
        bit_array_free(&bits);
        return image;
    }
    
    if (args->image_adaptive) {
        /// Reading block metadata
        uint16_t nof_blocks = image_number_of_blocks(&image, args->block_size);
        size_t block_metadata_size = nof_blocks * 2; // Each block has 2 bits of metadata
        int n = block_metadata_size;
        BitArray block_metadata = bit_array_new(NULL, 0);

        while (n > 0) {
            uint8_t byte = *data++;
            DECOMPRESS_ERROR_GUARD();
            DECOMPRESS_ERROR_GUARD(bit_array_push_n(&block_metadata, byte, 8));
            n -= 8;
        }

        logbytes("Current data", data, data_len);

        for (int i = 0; i < nof_blocks; i++) {
            CompressionType type = bit_array_read_n(&block_metadata, 2);
            DECOMPRESS_ERROR_GUARD();

            logfmt("Comrpessing block %d", i);
            Image block = image_get_block(&image, i, args->block_size);
            size_t block_size = image_size(&block);
            size_t len = block_size;

            logbytes("Data", data, image_size(&block));

            switch (type) {
                case CompressionType_None:
                    log("Compressed none");
                    memcpy(block.data, data, block_size);
                    break;
                case CompressionType_Horizontal:
                    log("Compressed horizontal");
                    len = posthuffman_decompress(data, data_len, args->transformace_data, &block);
                    break;
                case CompressionType_Vertical: {
                    log("Compressed vertical");
                    len = posthuffman_decompress(data, data_len, args->transformace_data, &block);
                    Image tmp = image_deserialization(block.data, block.width, block.height, Serialization_Vertical);
                    image_free(&block);
                    block = tmp;
                    break;
                }
                case CompressionType_Circular:
                    log("Compressed circular");
                    len = posthuffman_decompress(data, data_len, args->transformace_data, &block);
                    Image tmp = image_deserialization(block.data, block.width, block.height, Serialization_Circular);
                    image_free(&block);
                    block = tmp;
                    break;
            }

            data += len;
            data_len -= len;

            logfmt("Decompressed Type %d", type);
            logbytes("Decompressed data", block.data, image_size(&block));
            logfmt("Type %d", type);
            image_insert_block(&image, &block, i, args->block_size);
            log("Inserted block into the image");
        }

        bit_array_free(&block_metadata);
    } else {
        posthuffman_decompress(data, data_len, args->transformace_data, &image);
    }

    bit_array_free(&bits);

    return image;
}
