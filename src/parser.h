#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

/**
 * Parsea el código fuente y devuelve la raíz del AST.
 */
AstNode *parseProgram(void);

/**
 * Libera el AST generado.
 */
void freeAst(AstNode *root);

#endif
