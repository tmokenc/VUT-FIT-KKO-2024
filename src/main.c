#include <stdlib.h>
#include <stdio.h>
#include "image.h"
#include "args.h"
#include "error.h"
#include "compressor.h"

size_t load_file(const char *filename, uint8_t **output);
void save_file(const char *filename, uint8_t *bytes, size_t len);

int main(int argc, char **argv) {
    Args args = args_parse(argc, argv);

    if (got_error()) return got_error();
    if (args.is_help) {
        printf("Usage: huff_codec -[cdmawibo:h]\n"
               "  -w <width_value>    Specify the width of the image\n"
               "  -i <ifile>          Input file name\n"
               "  -o <ofile>          Output file name\n"
               "  -c                  Compress mode\n"
               "  -d                  Decompress mode\n"
               "  -m                  Activate model and RLE for preprocessing input data\n"
               "                      [Default: false]\n"
               "  -a                  Activate adaptive image scanning mode\n"
               "                      [Default: false]\n"
               "  -b <number>         Specify the block size for adaptive image\n"
               "                      [Default: 16]\n"
               "  -h                  Print this help message\n");

        return 0;
    }

    uint8_t *bytes;
    size_t filesize = load_file(args.filename, &bytes);

    if (got_error()) return got_error();
    
    switch (args.mode) {
        case Mode_Compress: {
            uint32_t height = filesize / args.width;
            Image image = image_from_raw(bytes, args.width, height);
            BitArray result = compressor_image_compress(&image, &args);
            if (got_error()) break;

            save_file(args.output_filename, result.data, bit_array_byte_len(&result));
            bit_array_free(&result);
            break;
        }

        case Mode_Decompress: {
            Image image = compressor_image_decompress(bytes, filesize, &args);
            if (got_error()) break;

            save_file(args.output_filename, image.data, image_size(&image));
            image_free(&image);
            break;
        }
    }

    free(bytes);
    
    return got_error();
}

size_t load_file(const char *filename, uint8_t **output) {
    FILE *file = fopen(filename, "rb"); // Open file for reading in binary mode
    if (file == NULL) {
        set_error(Error_InternalError);
        fprintf(stderr, "Error opening file: %s\n", filename);
        return 0; // Return 0 to indicate failure
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for file content
    *output = (uint8_t*)malloc(file_size);
    if (*output == NULL) {
        set_error(Error_OutOfMemory);
        fprintf(stderr, "Error allocating memory for file content.\n");
        fclose(file);
        return 0;
    }

    // Read file content into allocated memory
    size_t bytes_read = fread(*output, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        set_error(Error_InternalError);
        fprintf(stderr, "Error reading file: %s\n", filename);
        free(*output);
        return 0;
    }

    return bytes_read;
}

void save_file(const char *filename, uint8_t *bytes, size_t len) {
    // The flag means open file for writing in binary mode, create it if not exist
    FILE *file = fopen(filename, "w+b");
    if (file == NULL) {
        set_error(Error_InternalError);
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    // Write bytes to the file
    size_t bytes_written = fwrite(bytes, 1, len, file);
    fclose(file);

    if (bytes_written != len) {
        set_error(Error_InternalError);
        fprintf(stderr, "Error writing to file: %s\n", filename);
    }
}
