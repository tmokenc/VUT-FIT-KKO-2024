/**
 * @file compressor.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 */

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "bit_array.h"
#include "args.h"
#include "image.h"

BitArray compressor_image_compress(Image *image, Args *args);

Image compressor_image_decompress(BitArray *bits, Args *args);

#endif
