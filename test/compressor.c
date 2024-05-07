#include "greatest.h"
#include "../src/error.h"
#include "../src/args.h"
#include "../src/image.h"
#include "../src/compressor.h"

Image _IMAGE;
Args ARGS;

#define _IMAGE_WIDTH 1920
#define _IMAGE_HEIGHT 1280

static void compressor_setup(void *arg) {
    _IMAGE = image_new(_IMAGE_WIDTH, _IMAGE_HEIGHT);
    ARGS.filename = NULL;
    ARGS.output_filename = NULL;
    ARGS.image_adaptive = false;
    ARGS.transformace_data = false;
    ARGS.block_size = 16;

    fill_random(_IMAGE.data, image_size(&_IMAGE));
    clear_error();
    (void)arg;
}

static void compressor_tear_down(void *arg) {
    image_free(&_IMAGE);
    (void)arg;
}

SUITE(compressor);

TEST compressor_simple() {
    BitArray compressed = compressor_image_compress(&_IMAGE, &ARGS);
    Image decompressed = compressor_image_decompress(compressed.data, bit_array_byte_len(&compressed), &ARGS);

    ASSERT_EQ(_IMAGE.width, decompressed.width);
    ASSERT_EQ(_IMAGE.height, decompressed.height);
    ASSERT_MEM_EQ(_IMAGE.data, decompressed.data, image_size(&_IMAGE));

    PASS();
}

TEST compressor_transform() {
    ARGS.transformace_data = true;
    Image tmp_img = image_new(_IMAGE.width, _IMAGE.height);
    memcpy(tmp_img.data, _IMAGE.data, image_size(&_IMAGE));

    BitArray compressed = compressor_image_compress(&tmp_img, &ARGS);
    Image decompressed = compressor_image_decompress(compressed.data, bit_array_byte_len(&compressed), &ARGS);

    ASSERT_EQ(_IMAGE.width, decompressed.width);
    ASSERT_EQ(_IMAGE.height, decompressed.height);
    ASSERT_MEM_EQ(_IMAGE.data, decompressed.data, image_size(&_IMAGE));
    PASS();
}

TEST compressor_serialization() {
    ARGS.image_adaptive = true;
    BitArray compressed = compressor_image_compress(&_IMAGE, &ARGS);
    Image decompressed = compressor_image_decompress(compressed.data, bit_array_byte_len(&compressed), &ARGS);

    ASSERT_EQ(_IMAGE.width, decompressed.width);
    ASSERT_EQ(_IMAGE.height, decompressed.height);
    ASSERT_MEM_EQ(_IMAGE.data, decompressed.data, image_size(&_IMAGE));
    PASS();
}

TEST compressor_serialization_transform() {
    ARGS.image_adaptive = true;
    ARGS.transformace_data = true;
    Image tmp_img = image_new(_IMAGE.width, _IMAGE.height);
    memcpy(tmp_img.data, _IMAGE.data, image_size(&_IMAGE));

    BitArray compressed = compressor_image_compress(&tmp_img, &ARGS);
    Image decompressed = compressor_image_decompress(compressed.data, bit_array_byte_len(&compressed), &ARGS);

    ASSERT_EQ(_IMAGE.width, decompressed.width);
    ASSERT_EQ(_IMAGE.height, decompressed.height);
    ASSERT_MEM_EQ(_IMAGE.data, decompressed.data, image_size(&_IMAGE));
    PASS();
}

GREATEST_SUITE(compressor) {
    GREATEST_SET_SETUP_CB(compressor_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(compressor_tear_down, NULL);

    RUN_TEST(compressor_simple);
    RUN_TEST(compressor_transform);
    RUN_TEST(compressor_serialization);
    RUN_TEST(compressor_serialization_transform);
}

