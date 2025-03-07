#define _POSIX_C_SOURCE 200112L
#include "memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* ============================
   Wrappers Básicos de Memoria
   ============================ */

static size_t globalAllocCount = 0;
static size_t globalFreeCount  = 0;

void* memory_alloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: memory_alloc failed to allocate %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG_MEMORY
    globalAllocCount++;
    fprintf(stderr, "[memory_alloc] ptr=%p size=%zu (globalAllocCount=%zu)\n",
            ptr, size, globalAllocCount);
    #endif
    return ptr;
}

void memory_free(void *ptr) {
    #ifdef DEBUG_MEMORY
    if (ptr) {
        globalFreeCount++;
        fprintf(stderr, "[memory_free] ptr=%p (globalFreeCount=%zu)\n",
                ptr, globalFreeCount);
    }
    #endif
    free(ptr);
}

void* memory_realloc(void *ptr, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (!new_ptr && new_size != 0) {
        fprintf(stderr, "Error: memory_realloc failed to reallocate to %zu bytes\n", new_size);
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG_MEMORY
    fprintf(stderr, "[memory_realloc] old_ptr=%p new_ptr=%p new_size=%zu\n",
            ptr, new_ptr, new_size);
    #endif
    return new_ptr;
}

/* ============================
   Implementación del Memory Pooling
   ============================ */

/* Definición interna de la estructura MemoryPool */
struct MemoryPool {
    size_t blockSize;         /* Tamaño de cada bloque */
    size_t poolSize;          /* Número total de bloques */
    void *poolMemory;         /* Bloque contiguo de memoria asignado */
    void *freeList;           /* Lista enlazada de bloques libres */
    size_t totalAllocs;       /* Número total de asignaciones realizadas */
    size_t totalFrees;        /* Número total de liberaciones realizadas */
    pthread_mutex_t mutex;    /* Mutex para thread-safety */
};

/* Cada bloque libre usa los primeros bytes para apuntar al siguiente bloque */
typedef struct FreeBlock {
    struct FreeBlock *next;
} FreeBlock;

MemoryPool *memory_pool_create(size_t blockSize, size_t poolSize, size_t alignment) {
    if (blockSize < sizeof(FreeBlock *))
        blockSize = sizeof(FreeBlock *);

    MemoryPool *pool = (MemoryPool *)memory_alloc(sizeof(MemoryPool));
    pool->blockSize = blockSize;
    pool->poolSize = poolSize;
    pool->totalAllocs = 0;
    pool->totalFrees = 0;

    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize mutex in memory pool.\n");
        memory_free(pool);
        return NULL;
    }

    int res = posix_memalign(&pool->poolMemory, alignment, blockSize * poolSize);
    if (res != 0) {
        fprintf(stderr, "Error: posix_memalign failed with error %d\n", res);
        pthread_mutex_destroy(&pool->mutex);
        memory_free(pool);
        return NULL;
    }
    memset(pool->poolMemory, 0, blockSize * poolSize);

    pool->freeList = NULL;
    for (size_t i = 0; i < poolSize; i++) {
        void *block = (char *)pool->poolMemory + i * blockSize;
        ((FreeBlock *)block)->next = pool->freeList;
        pool->freeList = block;
    }
    #ifdef DEBUG_MEMORY
    fprintf(stderr, "[memory_pool_create] pool=%p blockSize=%zu poolSize=%zu alignment=%zu\n",
            pool, blockSize, poolSize, alignment);
    #endif
    return pool;
}

void *memory_pool_alloc(MemoryPool *pool) {
    void *block = NULL;
    pthread_mutex_lock(&pool->mutex);
    if (pool->freeList) {
        block = pool->freeList;
        pool->freeList = ((FreeBlock *)block)->next;
        pool->totalAllocs++;
        #ifdef DEBUG_MEMORY
        fprintf(stderr, "[memory_pool_alloc] pool=%p block=%p (totalAllocs=%zu)\n",
                pool, block, pool->totalAllocs);
        #endif
    }
    pthread_mutex_unlock(&pool->mutex);
    return block;
}

void memory_pool_free(MemoryPool *pool, void *ptr) {
    if (!ptr) return;
    pthread_mutex_lock(&pool->mutex);
    ((FreeBlock *)ptr)->next = pool->freeList;
    pool->freeList = ptr;
    pool->totalFrees++;
    #ifdef DEBUG_MEMORY
    fprintf(stderr, "[memory_pool_free] pool=%p block=%p (totalFrees=%zu)\n",
            pool, ptr, pool->totalFrees);
    #endif
    pthread_mutex_unlock(&pool->mutex);
}

void memory_pool_destroy(MemoryPool *pool) {
    if (!pool) return;
    pthread_mutex_destroy(&pool->mutex);
    memory_free(pool->poolMemory);
    memory_free(pool);
}

size_t memory_pool_get_total_allocs(MemoryPool *pool) {
    return pool ? pool->totalAllocs : 0;
}

size_t memory_pool_get_total_frees(MemoryPool *pool) {
    return pool ? pool->totalFrees : 0;
}

void memory_pool_dumpStats(MemoryPool *pool) {
    if (!pool) return;
    printf("Memory Pool Stats:\n");
    printf("  Block size   : %zu\n", pool->blockSize);
    printf("  Pool size    : %zu\n", pool->poolSize);
    printf("  Allocs       : %zu\n", pool->totalAllocs);
    printf("  Frees        : %zu\n", pool->totalFrees);
    size_t inUse = (pool->totalAllocs > pool->totalFrees)
                    ? (pool->totalAllocs - pool->totalFrees)
                    : 0;
    printf("  Blocks in use: %zu\n", inUse);
    printf("  Pool pointer : %p\n", (void*)pool);
}

/* ============================
   Funciones para Tracking Global de Memoria
   ============================ */

size_t memory_get_global_alloc_count(void) {
    return globalAllocCount;
}

size_t memory_get_global_free_count(void) {
    return globalFreeCount;
}
