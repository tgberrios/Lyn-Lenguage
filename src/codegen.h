#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/**
 * @brief Genera código ensamblador a partir del AST y lo escribe en el archivo especificado.
 *
 * @param root Puntero al nodo raíz del AST.
 * @param filename Nombre del archivo donde se escribirá el código ensamblador.
 */
void generateCode(AstNode *root, const char *filename);

#endif
