#include "greatest.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

void fill_random(uint8_t *output, int length) {
    int urandom = open("/dev/urandom", O_RDONLY);
    read(urandom, output, length);
    close(urandom);
}

#include "huffman.c"
#include "bit_array.c"
#include "transform.c"
#include "rle.c"
#include "image.c"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(bit_array);
    RUN_SUITE(huffman);
    RUN_SUITE(_transform);
    RUN_SUITE(rle);
    RUN_SUITE(image);

    GREATEST_MAIN_END();
}
