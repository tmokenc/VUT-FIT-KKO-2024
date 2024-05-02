#include "greatest.h"
#include "../src/error.h"
#include "../src/bit_array.h"

BitArray BIT_ARRAY;

static void bit_array_setup(void *arg) {
    BIT_ARRAY = bit_array_new(NULL, 0);
    clear_error();
    (void)arg;
}

static void bit_array_tear_down(void *arg) {
    bit_array_free(&BIT_ARRAY);
    (void)arg;
}

SUITE(bit_array);

TEST _bit_array_new() {
    BIT_ARRAY = bit_array_new(NULL, 0);

    ASSERT_EQ(BIT_ARRAY.len, 0);
    bit_array_free(&BIT_ARRAY);

    uint8_t bytes[] = {0, 1, 2, 3};

    BIT_ARRAY = bit_array_new(bytes, 4);

    ASSERT_EQ(BIT_ARRAY.len, 8 * 4);
    ASSERT_EQ(BIT_ARRAY.cursor, 0);
    ASSERT_EQ(BIT_ARRAY.capacity, 4);

    ASSERT_NEQ(BIT_ARRAY.data, bytes);
    
    PASS();
}


TEST _bit_array_bit_len() {
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 0);
    bit_array_push(&BIT_ARRAY, true);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 1);
    bit_array_push(&BIT_ARRAY, true);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 2);

    bit_array_push_n(&BIT_ARRAY, 123, 4);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 6);
    bit_array_push_n(&BIT_ARRAY, 123, 4);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 10);
    PASS();
}

TEST _bit_array_byte_len() {
    ASSERT_EQ(bit_array_byte_len(&BIT_ARRAY), 0);
    bit_array_push(&BIT_ARRAY, true);
    ASSERT_EQ(bit_array_byte_len(&BIT_ARRAY), 1);
    bit_array_push(&BIT_ARRAY, true);
    ASSERT_EQ(bit_array_byte_len(&BIT_ARRAY), 1);
    bit_array_push_n(&BIT_ARRAY, 123, 4);
    ASSERT_EQ(bit_array_byte_len(&BIT_ARRAY), 1);
    bit_array_push_n(&BIT_ARRAY, 123, 4);
    ASSERT_EQ(bit_array_byte_len(&BIT_ARRAY), 2);
    PASS();
}

TEST _bit_array_read_write() {
    bit_array_read(&BIT_ARRAY);
    ASSERT(got_error());

    clear_error();

    bit_array_push(&BIT_ARRAY, true);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);
    ASSERT_FALSE(got_error());
    bit_array_read(&BIT_ARRAY);
    ASSERT(got_error());

    clear_error();

    bit_array_push(&BIT_ARRAY, true);
    bit_array_push(&BIT_ARRAY, false);
    bit_array_push(&BIT_ARRAY, false);
    bit_array_push(&BIT_ARRAY, true);
    bit_array_push(&BIT_ARRAY, false);

    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);

    bit_array_read(&BIT_ARRAY);
    ASSERT(got_error());

    PASS();
}

TEST _bit_array_read_write_n() {

    bit_array_push_n(&BIT_ARRAY, 3, 2);
    bit_array_push(&BIT_ARRAY, true);

    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 3);

    bit_array_push_n(&BIT_ARRAY, 0xFA, 8);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 11);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 3), 7);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 8), 0xFA);
    PASS();
}

TEST _bit_array_pad_to_byte() {
    bit_array_pad_to_byte(&BIT_ARRAY);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 0);

    bit_array_push_n(&BIT_ARRAY, 2, 2);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 2);

    bit_array_pad_to_byte(&BIT_ARRAY);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 8);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 2), 2);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 6), 0);

    bit_array_push_n(&BIT_ARRAY, 6, 4);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 12);
    bit_array_pad_to_byte(&BIT_ARRAY);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 16);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 4), 6);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 4), 0);

    bit_array_pad_to_byte(&BIT_ARRAY);
    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 16);

    bit_array_push(&BIT_ARRAY, false);
    bit_array_push(&BIT_ARRAY, false);
    bit_array_push(&BIT_ARRAY, false);
    bit_array_push(&BIT_ARRAY, true);
    bit_array_push(&BIT_ARRAY, true);

    bit_array_pad_to_byte(&BIT_ARRAY);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);

    bit_array_read_n(&BIT_ARRAY, 3);
    ASSERT_FALSE(got_error());

    bit_array_read(&BIT_ARRAY);
    ASSERT(got_error());

    PASS();
}

TEST _bit_array_concat() {
    BitArray rhs = bit_array_new(NULL, 0);
    bit_array_push_n(&rhs, 0xFA, 8);

    bit_array_concat(&BIT_ARRAY, &rhs);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 8), 0xFA);

    bit_array_read(&BIT_ARRAY);

    ASSERT(got_error());
    clear_error();

    bit_array_free(&rhs);
    bit_array_push_n(&rhs, 5, 3);

    bit_array_concat(&BIT_ARRAY, &rhs);

    ASSERT_EQ(bit_array_bit_len(&BIT_ARRAY), 11);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 3), 5);

    bit_array_free(&rhs);

    bit_array_push_n(&BIT_ARRAY, 0x43fcFAAF, 64);
    bit_array_push_n(&BIT_ARRAY, 0x43fcAFFA, 64);
    bit_array_push_n(&BIT_ARRAY, 0x43fc8aa8, 64);
    bit_array_push_n(&BIT_ARRAY, 0x43fc8372, 64);
    bit_array_push_n(&BIT_ARRAY, 0x43fc0930, 64);
    bit_array_push_n(&BIT_ARRAY, 0x43fc7329, 64);
    bit_array_push_n(&BIT_ARRAY, 0x43fca292, 64);

    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fcFAAF);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fcAFFA);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fc8aa8);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fc8372);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fc0930);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fc7329);
    ASSERT_EQ(bit_array_read_n(&BIT_ARRAY, 64), 0x43fca292);

    PASS();
}

TEST _bit_array_multi_bytes() {
    uint32_t data = 0xFAAF8679;

    bit_array_push_n(&BIT_ARRAY, data, 32);

    uint32_t read = bit_array_read_n(&BIT_ARRAY, 32);

    ASSERT_EQ(data, read);

    PASS();
}

TEST _bit_array_set_one_at() {
    bit_array_push_n(&BIT_ARRAY, 0x00, 8);
    bit_array_push_n(&BIT_ARRAY, 0x00, 8);

    bit_array_set_one_at(&BIT_ARRAY, 3);
    bit_array_set_one_at(&BIT_ARRAY, 12);

    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), true);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);
    ASSERT_EQ(bit_array_read(&BIT_ARRAY), false);

    ASSERT_FALSE(got_error());

    bit_array_read(&BIT_ARRAY);

    ASSERT(got_error());

    PASS();
}

GREATEST_SUITE(bit_array) {
    GREATEST_SET_SETUP_CB(bit_array_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(bit_array_tear_down, NULL);

    RUN_TEST(_bit_array_new);
    RUN_TEST(_bit_array_bit_len);
    RUN_TEST(_bit_array_byte_len);
    RUN_TEST(_bit_array_pad_to_byte);
    RUN_TEST(_bit_array_concat);
    RUN_TEST(_bit_array_read_write);
    RUN_TEST(_bit_array_read_write_n);
    RUN_TEST(_bit_array_multi_bytes);
    RUN_TEST(_bit_array_set_one_at);
}
