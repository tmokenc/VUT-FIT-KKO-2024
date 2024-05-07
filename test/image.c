#include "greatest.h"
#include "../src/error.h"
#include "../src/image.h"

Image IMAGE;

#define BLOCK_SIZE 16
#define IMAGE_WIDTH 1920
#define IMAGE_HEIGHT 1280

static void image_setup(void *arg) {
    IMAGE = image_new(IMAGE_WIDTH, IMAGE_HEIGHT);
    fill_random(IMAGE.data, image_size(&IMAGE));
    clear_error();
    (void)arg;
}

static void image_tear_down(void *arg) {
    image_free(&IMAGE);
    (void)arg;
}

SUITE(image);

TEST image_blocks() {
    Image tmp = image_new(IMAGE_WIDTH, IMAGE_HEIGHT);

    Image block;
    for (int i = 0; i < image_number_of_blocks(&IMAGE, BLOCK_SIZE); i++) {
        block = image_get_block(&IMAGE, i, BLOCK_SIZE);
        // if (got_error() == Error_OutOfMemory) PASS();
        image_insert_block(&tmp, &block, i, BLOCK_SIZE);
        image_free(&block);
    }

    ASSERT_MEM_EQ(IMAGE.data, tmp.data, image_size(&IMAGE));
    PASS();
}

TEST image_serialization_vertical() {
    uint8_t *tmp = image_serialization(&IMAGE, Serialization_Vertical);

    ASSERT_NEQ(memcmp(tmp, IMAGE.data, image_size(&IMAGE)), 0);

    Image revert = image_deserialization(tmp, IMAGE.width, IMAGE.height, Serialization_Vertical);

    ASSERT_MEM_EQ(revert.data, IMAGE.data, image_size(&IMAGE));
    
    free(tmp);
    image_free(&revert);
    PASS();
}

TEST image_serialization_circular() {
    uint8_t *tmp = image_serialization(&IMAGE, Serialization_Circular);

    ASSERT_NEQ(memcmp(tmp, IMAGE.data, image_size(&IMAGE)), 0);

    Image revert = image_deserialization(tmp, IMAGE.width, IMAGE.height, Serialization_Circular);

    ASSERT_MEM_EQ(revert.data, IMAGE.data, image_size(&IMAGE));
    
    free(tmp);
    image_free(&revert);
    PASS();
}

GREATEST_SUITE(image) {
    GREATEST_SET_SETUP_CB(image_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(image_tear_down, NULL);

    RUN_TEST(image_blocks);
    RUN_TEST(image_serialization_vertical);
    RUN_TEST(image_serialization_circular);
}
