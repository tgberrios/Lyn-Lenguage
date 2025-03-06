#include "memory.h"
#include <stdlib.h>
#include <stdio.h>

void* memory_alloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: memory_alloc failed to allocate %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void memory_free(void *ptr) {
    free(ptr);
}

void* memory_realloc(void *ptr, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (!new_ptr && new_size != 0) {
        fprintf(stderr, "Error: memory_realloc failed to reallocate to %zu bytes\n", new_size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}
