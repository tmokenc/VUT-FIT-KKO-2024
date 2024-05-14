/**
 * @file error.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 03/23/2024
 * @brief Definitions and functions related to error handling and debugging.
 */

#ifndef ERROR_H
#define ERROR_H

#ifdef DEBUG_F

/**
 * @brief macro to print out debug string if NDEBUG flag is disabled
 **/
#define log(s) fprintf(stderr, "- "__FILE__":%u: "s"\n", __LINE__)

/**
 * @brief macro to print out debug string formatted (like printf) if NDEBUG flag is disabled
 **/
#define logfmt(s, ...) fprintf(stderr, "- "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

#define logbytes(str, bytes, size) \
    fprintf(stderr, "%s: ", str); \
    for (size_t i = 0; i < size; i++) fprintf(stderr, "%02X ", bytes[i]); \
    fprintf(stderr, "\n");

#else

#define log(s)
#define logfmt(s, ...) 
#define logbytes(str, bytes, size) 

#endif

typedef enum {
    Error_None,
    Error_OutOfMemory,
    Error_IndexOutOfBound,
    Error_FileNotFound,
    Error_InvalidArgument,
    Error_InvalidImageSize,
    Error_InvalidBlockSize,
    Error_InternalError,
} Error;

/**
 * @brief Set the current error type.
 * @param type The error type to set.
 */
void set_error(Error type);

/**
 * @brief Clear the current error.
 */
void clear_error();

/**
 * @brief Get the type of the last error occurred.
 * @return The type of the last error occurred.
 */
Error got_error();

#endif
