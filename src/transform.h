/**
 * @file transform.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 09/04/2024
 * @brief Transform image data to another representation
 */

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Transform the image data to another representation.
 *
 * @param bytes Pointer to the array of bytes representing the image data.
 * @param len The length of the image data array.
 */
void transform(uint8_t *bytes, size_t len);

/**
 * @brief Revert the transformed image data back to its original representation.
 *
 * @param bytes Pointer to the array of bytes representing the transformed image data.
 * @param len The length of the transformed image data array.
 */
void transform_revert(uint8_t *bytes, size_t len);

#endif
