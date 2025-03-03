#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/**
 * Genera código ensamblador a partir del AST y lo escribe en el archivo 'filename'.
 */
void generateCode(AstNode *root, const char *filename);

#endif
