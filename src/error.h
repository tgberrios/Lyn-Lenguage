#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

#define REPORT_ERROR(msg, ...) \
    do { \
        fprintf(stderr, "ERROR [%s:%d]: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

#define REPORT_WARNING(msg, ...) \
    do { \
        fprintf(stderr, "WARNING [%s:%d]: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#endif /* ERROR_H */
