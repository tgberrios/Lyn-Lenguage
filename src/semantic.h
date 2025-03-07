#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* Tipos de datos semánticos */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CLASS,    // Para tipos definidos por el usuario (clases)
    TYPE_UNKNOWN
} DataType;

/**
 * @brief Realiza el análisis semántico sobre el AST.
 *
 * Recorre el árbol de sintaxis abstracta, verifica consistencia de tipos en 
 * operaciones, asignaciones y declaraciones, y gestiona el alcance de las 
 * variables usando una pila de tablas de símbolos.
 *
 * @param root Puntero a la raíz del AST.
 */
void analyzeSemantics(AstNode *root);

#endif /* SEMANTIC_H */
