#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <stdio.h>
#include <time.h>


// ====== Defining log levels ======

#define LOG_LEVEL_INFO  1   // Controls info-level logs.
#define LOG_LEVEL_DEBUG 1   // Controls debug-level logs.
#define LOG_LEVEL_WARN  1   // Controls warn-level logs.
#define LOG_LEVEL_ERROR 1   // Controls error-level logs.

// ====== Defining ANSI colors ======

#define COLOR_RESET   "\033[0m"     // No color
#define COLOR_DEBUG   "\033[36m"    // Cyan
#define COLOR_INFO    "\033[32m"    // Green
#define COLOR_WARN    "\033[33m"    // Yellow
#define COLOR_ERROR   "\033[31m"    // Red

// ====== Defining macros ======

/**
 * Base macro. It adds timestamp and contex to the actual print.
 * @param debug_level The debug-level to print.
 * @param format_string A string that specifies the data to be printed. It may also contain a format specifier as a placeholder to print the value of any variable or value. E.g. "This is my %d message!".
 * @param __VA_ARGS__... The variable/values corresponding to the format specifier.
 */ 
#define LOG_BASE(debug_level, format_string, ...) do {                              \
    time_t seconds = time(NULL);                                                    \
    struct tm timeStruct;                                                           \
    localtime_r(&seconds, &timeStruct);                                             \
    fprintf(stderr, "[%02d:%02d:%02d][%s] %s:%d:%s(): " format_string,              \
            timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec, debug_level,  \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                           \
} while (0)

// Info-level macro
#if LOG_LEVEL_INFO
#  define LOG_INFO(format_string, ...)  LOG_BASE(COLOR_INFO "INFO" COLOR_RESET, format_string, ##__VA_ARGS__)
#else
#  define LOG_INFO(format_string, ...)
#endif

// Debug-level macro
#if LOG_LEVEL_DEBUG
#  define LOG_DEBUG(format_string, ...) LOG_BASE(COLOR_DEBUG "DEBUG" COLOR_RESET, format_string, ##__VA_ARGS__)
#else
#  define LOG_DEBUG(format_string, ...)
#endif

// Warn-level macro
#if LOG_LEVEL_WARN
#  define LOG_WARN(format_string, ...)  LOG_BASE(COLOR_WARN "WARN" COLOR_RESET, format_string, ##__VA_ARGS__)
#else
#  define LOG_WARN(format_string, ...)
#endif

// Error-level macro
#if LOG_LEVEL_ERROR
#  define LOG_ERROR(format_string, ...) LOG_BASE(COLOR_ERROR "ERROR" COLOR_RESET, format_string, ##__VA_ARGS__)
#else
#  define LOG_ERROR(format_string, ...)
#endif

/**
 * Custom debug-level macro for structs. It uses a custom function defined to print a struct. 
 * @param print_fn The print function defined for the struct. This function should take `structPointer` as input.
 * @param structPointer The pointer to the struct to print throught `print_fn` function.
 */
#define LOG_STRUCT_DEBUG(print_fn, structPointer) do {                              \
    LOG_DEBUG("%s","");                                                             \
    print_fn(structPointer);                                                        \
} while (0)

#endif