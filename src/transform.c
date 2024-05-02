/**
 * @file transform.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 09/04/2024
 * @brief Implementation of `transform.h`
 */

#include "transform.h"

void transform(uint8_t *bytes, size_t len) {
    if (len < 2) return;

    uint8_t last = 0;
    
    for (size_t i = 0; i < len; i++) {
        uint8_t tmp = bytes[i];
        bytes[i] -=  last;
        last = tmp;
    }
}

void transform_revert(uint8_t *bytes, size_t len) {
    if (len < 2) return;

    for (size_t i = 1; i < len; i++) {
        bytes[i] += bytes[i - 1];
    }
}

