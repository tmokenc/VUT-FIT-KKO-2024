/**
 * @file error.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 03/23/2024
 * @brief Implementation for `error.h`
 */

#include "error.h"

Error ERROR = Error_None;

void set_error(Error err) {
    ERROR = err;
}

Error got_error() {
    return ERROR;
}

void clear_error() {
    ERROR = Error_None;
}
