#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/**
 * @brief Realiza el análisis semántico sobre el AST.
 *
 * Esta función recorre el árbol de sintaxis abstracta y verifica que las
 * construcciones sean coherentes desde el punto de vista semántico.
 * En esta versión básica se realiza un recorrido recursivo; se puede ampliar
 * para incluir verificación de tipos, alcance de variables y otras reglas semánticas.
 *
 * @param root Puntero a la raíz del AST.
 */
void analyzeSemantics(AstNode *root);

#endif /* SEMANTIC_H */
