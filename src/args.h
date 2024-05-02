/**
 * @file args.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 09/04/2024
 * @brief Arguments parser 
 */

#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enumeration representing the mode of operation.
 */
typedef enum {
    Mode_Compress, /**< Compression mode */
    Mode_Decompress /**< Decompression mode */
} Mode;

/**
 * @brief Structure representing parsed command-line arguments.
 */
typedef struct {
    char *filename; /**< Input file name */
    char *output_filename; /**< Output file name */
    bool image_adaptive; /**< Flag indicating whether adaptive image scanning is activated */
    bool transformace_data; /**< Flag indicating whether data transformation is activated */
    uint32_t width; /**< Width of the image */
    int block_size; /**< Block size for adaptive image scanning */
    Mode mode; /**< Mode of operation (compression or decompression) */
    bool is_help; /**< Flag indicating whether the help message should be displayed */
} Args;

/**
 * @brief Parses command-line arguments and fills the Args structure.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Parsed command-line arguments in the Args structure.
 */
Args args_parse(int argc, char **argv);

#endif
