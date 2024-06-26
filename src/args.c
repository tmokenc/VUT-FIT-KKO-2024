/**
 * @file args.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 09/04/2024
 * @brief Implementation for `args.h`
 */

#include "args.h"
#include "error.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

Args args_parse(int argc, char **argv) {
    Args args = {0};
    args.filename = NULL;
    args.output_filename = NULL;
    args.image_adaptive = false;
    args.transformace_data = false;
    args.width = 0;
    args.block_size = 128; // Default to 128x128 per block
    args.mode = Mode_Compress; // Default mode is compress

    int opt;
    while ((opt = getopt(argc, argv, "cdmaw:i:o:b:h")) != -1) {
        switch (opt) {
            case 'c':
                args.mode = Mode_Compress;
                break;
            case 'd':
                args.mode = Mode_Decompress;
                break;
            case 'm':
                args.transformace_data = true;
                break;
            case 'a':
                args.image_adaptive = true;
                break;
            case 'w':
                args.width = atoi(optarg);
                break;
            case 'b':
                args.block_size = atoi(optarg);
                break;
            case 'i':
                args.filename = optarg;
                break;
            case 'o':
                args.output_filename = optarg;
                break;
            case 'h':
                args.is_help = true;
                return args;
            default:
                fprintf(stderr, "Unknown option: %c\n", optopt);
                set_error(Error_InvalidArgument);
        }
    }

    if (args.filename == NULL) {
        set_error(Error_InvalidArgument);
        fprintf(stderr, "Error: Input file not specified.\n");
    }

    if (args.output_filename == NULL) {
        set_error(Error_InvalidArgument);
        fprintf(stderr, "Error: Output file not specified.\n");
    }

    if (args.mode == Mode_Compress && !args.width) {
        set_error(Error_InvalidArgument);
        fprintf(stderr, "Error: Width of the image not specified.\n");
    }

    if (!args.block_size) {
        set_error(Error_InvalidArgument);
        fprintf(stderr, "Error: Invalid block size.\n");
    }

    return args;
}
