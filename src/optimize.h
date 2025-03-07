#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "ast.h"

/**
 * @brief Optimiza el AST aplicando optimizaciones como eliminación de código muerto,
 *        propagación de constantes y simplificación de expresiones.
 *
 * @param root Puntero a la raíz del AST.
 * @return AstNode* Puntero al AST optimizado.
 */
AstNode *optimizeAST(AstNode *root);

#endif /* OPTIMIZE_H */