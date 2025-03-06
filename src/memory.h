#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================
   Wrappers Básicos de Memoria
   ============================ */

/**
 * @brief Asigna memoria del tamaño especificado.
 *
 * Envuelve a malloc. En caso de fallo, imprime un mensaje de error y termina la ejecución.
 *
 * @param size Tamaño en bytes a asignar.
 * @return void* Puntero a la memoria asignada.
 */
void* memory_alloc(size_t size);

/**
 * @brief Libera la memoria asignada.
 *
 * Envuelve a free.
 *
 * @param ptr Puntero a la memoria a liberar.
 */
void memory_free(void *ptr);

/**
 * @brief Reasigna un bloque de memoria.
 *
 * Envuelve a realloc. Si falla, imprime un mensaje de error y termina la ejecución.
 *
 * @param ptr Puntero a la memoria previamente asignada.
 * @param new_size Nuevo tamaño en bytes.
 * @return void* Puntero a la memoria reasignada.
 */
void* memory_realloc(void *ptr, size_t new_size);

/* ============================
   Memory Pooling para Objetos Fijos
   ============================ */

/**
 * @brief Estructura que representa un pool de memoria para objetos de tamaño fijo.
 *
 * El pool reserva un bloque contiguo de memoria, lo divide en bloques fijos,
 * mantiene una lista de bloques libres para reutilización y registra estadísticas.
 */
typedef struct MemoryPool MemoryPool;

/**
 * @brief Crea un pool de memoria para objetos de tamaño fijo.
 *
 * @param blockSize Tamaño de cada bloque en bytes.
 * @param poolSize Número total de bloques a reservar.
 * @param alignment Alineación requerida (por ejemplo, 16, 32 o 64 bytes).
 * @return MemoryPool* Puntero al pool creado o NULL en caso de error.
 */
MemoryPool *memory_pool_create(size_t blockSize, size_t poolSize, size_t alignment);

/**
 * @brief Asigna un bloque de memoria desde el pool.
 *
 * Retorna un bloque libre o NULL si no hay bloques disponibles.
 *
 * @param pool Puntero al pool.
 * @return void* Puntero al bloque asignado.
 */
void *memory_pool_alloc(MemoryPool *pool);

/**
 * @brief Libera un bloque, devolviéndolo al pool.
 *
 * @param pool Puntero al pool.
 * @param ptr Puntero al bloque a liberar.
 */
void memory_pool_free(MemoryPool *pool, void *ptr);

/**
 * @brief Destruye el pool de memoria y libera todos sus recursos.
 *
 * @param pool Puntero al pool a destruir.
 */
void memory_pool_destroy(MemoryPool *pool);

/**
 * @brief Obtiene el número total de asignaciones realizadas desde el pool.
 *
 * @param pool Puntero al pool.
 * @return size_t Número de asignaciones.
 */
size_t memory_pool_get_total_allocs(MemoryPool *pool);

/**
 * @brief Obtiene el número total de liberaciones realizadas en el pool.
 *
 * @param pool Puntero al pool.
 * @return size_t Número de liberaciones.
 */
size_t memory_pool_get_total_frees(MemoryPool *pool);

/**
 * @brief Imprime estadísticas del pool de memoria.
 *
 * Muestra información sobre el tamaño de bloque, número de bloques, asignaciones y liberaciones.
 *
 * @param pool Puntero al pool.
 */
void memory_pool_dumpStats(MemoryPool *pool);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H */
