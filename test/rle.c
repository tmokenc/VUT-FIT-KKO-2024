#include "greatest.h"
#include "../src/error.h"
#include "../src/rle.h"

SUITE(rle);

#define RLE_DATA_SIZE (1280 * 1920)
uint8_t *RLE_DATA;

static void rle_setup(void *arg) {
    clear_error();
    RLE_DATA = malloc(RLE_DATA_SIZE);
    fill_random(RLE_DATA, RLE_DATA_SIZE);
    (void)arg;
}

static void rle_teardown(void *arg) {
    clear_error();
    free(RLE_DATA);
    (void)arg;
}

TEST rle_correctness() {
    BitArray compressed = rle_encode(RLE_DATA, RLE_DATA_SIZE);
    BitArray decompressed = rle_decode(compressed.data, bit_array_byte_len(&compressed));

    ASSERT_FALSE(got_error());
    ASSERT_EQ(RLE_DATA_SIZE, bit_array_byte_len(&decompressed));
    ASSERT_MEM_EQ(RLE_DATA, decompressed.data, RLE_DATA_SIZE);

    bit_array_free(&compressed);
    bit_array_free(&decompressed);

    PASS();
}

GREATEST_SUITE(rle) {
    GREATEST_SET_SETUP_CB(rle_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(rle_teardown, NULL);

    RUN_TEST(rle_correctness);
}

