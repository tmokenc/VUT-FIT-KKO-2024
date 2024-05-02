#include "greatest.h"
#include "../src/error.h"
#include "../src/huffman.h"

SUITE(huffman);

#define DATA_SIZE (1280 * 1920)
uint8_t *DATA;

static void huffman_setup(void *arg) {
    clear_error();
    DATA = malloc(DATA_SIZE);
    fill_random(DATA, DATA_SIZE);
    (void)arg;
}

static void huffman_teardown(void *arg) {
    clear_error();
    free(DATA);
    (void)arg;
}

TEST huffman_correctness() {
    BitArray compressed = huffman_compress(DATA, DATA_SIZE);
    BitArray decompressed = huffman_decompress(compressed.data, bit_array_byte_len(&compressed));

    ASSERT_FALSE(got_error());
    // printf("Len %ld\n", bit_array_byte_len(&decompressed));
    // ASSERT_EQ(DATA_SIZE, bit_array_byte_len(&decompressed));
    ASSERT_MEM_EQ(DATA, decompressed.data, bit_array_byte_len(&decompressed));
    ASSERT_MEM_EQ(DATA, decompressed.data, DATA_SIZE);

    bit_array_free(&compressed);
    bit_array_free(&decompressed);

    PASS();
}

GREATEST_SUITE(huffman) {
    GREATEST_SET_SETUP_CB(huffman_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(huffman_teardown, NULL);

    RUN_TEST(huffman_correctness);
}
