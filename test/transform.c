#include "greatest.h"
#include "../src/error.h"
#include "../src/transform.h"
#include <string.h>

#define TRANSFORM_DATA_SIZE (1280 * 1920)
uint8_t *TRANSFORM_DATA;

static void transform_setup(void *arg) {
    clear_error();
    TRANSFORM_DATA = malloc(TRANSFORM_DATA_SIZE);
    fill_random(TRANSFORM_DATA, TRANSFORM_DATA_SIZE);
    (void)arg;
}

static void transform_teardown(void *arg) {
    clear_error();
    free(TRANSFORM_DATA);
    (void)arg;
}

SUITE(_transform);

TEST transform_correctness() {
    uint8_t res[TRANSFORM_DATA_SIZE] = {0};

    memcpy(res, TRANSFORM_DATA, TRANSFORM_DATA_SIZE);

    ASSERT_FALSE(got_error());

    transform(res, TRANSFORM_DATA_SIZE);

    ASSERT_NEQ(memcmp(res, TRANSFORM_DATA, TRANSFORM_DATA_SIZE), 0);

    transform_revert(res, TRANSFORM_DATA_SIZE);

    ASSERT_MEM_EQ(res, TRANSFORM_DATA, TRANSFORM_DATA_SIZE);
    PASS();
}

GREATEST_SUITE(_transform) {
    GREATEST_SET_SETUP_CB(transform_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(transform_teardown, NULL);

    RUN_TEST(transform_correctness);
}

