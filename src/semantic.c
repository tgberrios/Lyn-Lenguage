#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Función interna recursiva para analizar nodos del AST.
 *
 * Recorre el AST y aplica verificaciones semánticas básicas a cada nodo.
 *
 * @param node Nodo del AST a analizar.
 */
static void analyzeNode(AstNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->program.statementCount; i++) {
                analyzeNode(node->program.statements[i]);
            }
            break;
        case AST_VAR_ASSIGN:
            analyzeNode(node->varAssign.value);
            break;
        case AST_FUNC_DEF:
            for (int i = 0; i < node->funcDef.bodyCount; i++) {
                analyzeNode(node->funcDef.body[i]);
            }
            break;
        case AST_RETURN_STMT:
            analyzeNode(node->returnStmt.expr);
            break;
        case AST_PRINT_STMT:
            analyzeNode(node->printStmt.expr);
            break;
        case AST_BINARY_OP:
            analyzeNode(node->binaryOp.left);
            analyzeNode(node->binaryOp.right);
            break;
        case AST_LAMBDA:
            analyzeNode(node->lambda.body);
            break;
        case AST_IF_STMT:
            analyzeNode(node->ifStmt.condition);
            for (int i = 0; i < node->ifStmt.thenCount; i++) {
                analyzeNode(node->ifStmt.thenBranch[i]);
            }
            for (int i = 0; i < node->ifStmt.elseCount; i++) {
                analyzeNode(node->ifStmt.elseBranch[i]);
            }
            break;
        case AST_FOR_STMT:
            analyzeNode(node->forStmt.rangeStart);
            analyzeNode(node->forStmt.rangeEnd);
            for (int i = 0; i < node->forStmt.bodyCount; i++) {
                analyzeNode(node->forStmt.body[i]);
            }
            break;
        case AST_CLASS_DEF:
            for (int i = 0; i < node->classDef.memberCount; i++) {
                analyzeNode(node->classDef.members[i]);
            }
            break;
        default:
            // Para nodos literales y de identificador no se requiere análisis adicional.
            break;
    }
}

/**
 * @brief Realiza el análisis semántico del AST.
 *
 * Llama a la función recursiva analyzeNode para analizar todo el árbol.
 *
 * @param root Puntero a la raíz del AST.
 */
void analyzeSemantics(AstNode *root) {
    analyzeNode(root);
}
