#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/**
 * @brief Asigna memoria del tamaño especificado.
 *
 * Esta función envuelve a malloc, y en caso de fallo imprime un mensaje de error
 * y termina la ejecución del programa.
 *
 * @param size Tamaño en bytes de la memoria a asignar.
 * @return void* Puntero a la memoria asignada.
 */
void* memory_alloc(size_t size);

/**
 * @brief Libera la memoria asignada previamente.
 *
 * Envuelve la función free.
 *
 * @param ptr Puntero a la memoria a liberar.
 */
void memory_free(void *ptr);

/**
 * @brief Reasigna un bloque de memoria.
 *
 * Envuelve la función realloc. Si falla, imprime un mensaje de error y termina.
 *
 * @param ptr Puntero a la memoria previamente asignada.
 * @param new_size Nuevo tamaño en bytes para la memoria.
 * @return void* Puntero a la nueva memoria reasignada.
 */
void* memory_realloc(void *ptr, size_t new_size);

#endif /* MEMORY_H */
