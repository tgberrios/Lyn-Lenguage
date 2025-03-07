/* optimize.c */
#include "optimize.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Función auxiliar para crear un nodo literal numérico a partir de un valor double */
static AstNode *makeNumberLiteral(double value) {
    AstNode *node = createAstNode(AST_NUMBER_LITERAL);
    node->numberLiteral.value = value;
    return node;
}

/* Optimización de expresiones binarias: constant folding */
static AstNode *optimizeBinaryOp(AstNode *node) {
    if (!node || node->type != AST_BINARY_OP)
        return node;

    /* Optimiza primero los operandos recursivamente */
    node->binaryOp.left = optimizeAST(node->binaryOp.left);
    node->binaryOp.right = optimizeAST(node->binaryOp.right);

    /* Si ambos operandos son literales numéricos, evaluamos la operación */
    if (node->binaryOp.left->type == AST_NUMBER_LITERAL &&
        node->binaryOp.right->type == AST_NUMBER_LITERAL) {
        double leftVal = node->binaryOp.left->numberLiteral.value;
        double rightVal = node->binaryOp.right->numberLiteral.value;
        double result = 0.0;
        switch (node->binaryOp.op) {
            case '+': result = leftVal + rightVal; break;
            case '-': result = leftVal - rightVal; break;
            case '*': result = leftVal * rightVal; break;
            case '/': 
                if (rightVal == 0) {
                    fprintf(stderr, "Runtime error: Division by zero in constant folding.\n");
                    exit(1);
                }
                result = leftVal / rightVal; 
                break;
            default:
                /* Si la operación no es soportada, retornamos el nodo sin cambios */
                return node;
        }
        /* Libera los nodos hijos del operador */
        freeAstNode(node->binaryOp.left);
        freeAstNode(node->binaryOp.right);
        /* Libera el nodo actual y retorna un nuevo nodo literal con el resultado */
        freeAstNode(node);
        return makeNumberLiteral(result);
    }
    return node;
}

/* Optimización de sentencias if: eliminación de código muerto */
static AstNode *optimizeIfStmt(AstNode *node) {
    if (!node || node->type != AST_IF_STMT)
        return node;

    node->ifStmt.condition = optimizeAST(node->ifStmt.condition);

    /* Si la condición es un literal numérico, evaluamos la condición */
    if (node->ifStmt.condition->type == AST_NUMBER_LITERAL) {
        double condVal = node->ifStmt.condition->numberLiteral.value;
        int condTrue = (condVal != 0);  /* Se interpreta 0 como false */

        /* Elimina la rama no ejecutada */
        if (condTrue) {
            /* Liberamos la rama else si existe */
            for (int i = 0; i < node->ifStmt.elseCount; i++) {
                freeAstNode(node->ifStmt.elseBranch[i]);
            }
            free(node->ifStmt.elseBranch);
            node->ifStmt.elseBranch = NULL;
            node->ifStmt.elseCount = 0;
        } else {
            /* Liberamos la rama then */
            for (int i = 0; i < node->ifStmt.thenCount; i++) {
                freeAstNode(node->ifStmt.thenBranch[i]);
            }
            free(node->ifStmt.thenBranch);
            node->ifStmt.thenBranch = NULL;
            node->ifStmt.thenCount = 0;
        }
    } else {
        /* Si la condición no es constante, optimizamos ambas ramas */
        for (int i = 0; i < node->ifStmt.thenCount; i++) {
            node->ifStmt.thenBranch[i] = optimizeAST(node->ifStmt.thenBranch[i]);
        }
        for (int i = 0; i < node->ifStmt.elseCount; i++) {
            node->ifStmt.elseBranch[i] = optimizeAST(node->ifStmt.elseBranch[i]);
        }
    }
    return node;
}

/* Función principal de optimización del AST */
AstNode *optimizeAST(AstNode *root) {
    if (!root) return NULL;

    switch (root->type) {
        case AST_PROGRAM:
            for (int i = 0; i < root->program.statementCount; i++) {
                root->program.statements[i] = optimizeAST(root->program.statements[i]);
            }
            break;
        case AST_VAR_ASSIGN:
            /* Actualizado: usar initializer en lugar de value */
            root->varAssign.initializer = optimizeAST(root->varAssign.initializer);
            break;
        case AST_VAR_DECL:
            /* Opcional: optimizar el inicializador si existe */
            if (root->varDecl.initializer)
                root->varDecl.initializer = optimizeAST(root->varDecl.initializer);
            break;
        case AST_FUNC_DEF:
            for (int i = 0; i < root->funcDef.bodyCount; i++) {
                root->funcDef.body[i] = optimizeAST(root->funcDef.body[i]);
            }
            break;
        case AST_RETURN_STMT:
            root->returnStmt.expr = optimizeAST(root->returnStmt.expr);
            break;
        case AST_PRINT_STMT:
            root->printStmt.expr = optimizeAST(root->printStmt.expr);
            break;
        case AST_BINARY_OP:
            return optimizeBinaryOp(root);
        case AST_LAMBDA:
            root->lambda.body = optimizeAST(root->lambda.body);
            break;
        case AST_IF_STMT:
            return optimizeIfStmt(root);
        case AST_FOR_STMT:
            root->forStmt.rangeStart = optimizeAST(root->forStmt.rangeStart);
            root->forStmt.rangeEnd = optimizeAST(root->forStmt.rangeEnd);
            for (int i = 0; i < root->forStmt.bodyCount; i++) {
                root->forStmt.body[i] = optimizeAST(root->forStmt.body[i]);
            }
            break;
        case AST_CLASS_DEF:
            for (int i = 0; i < root->classDef.memberCount; i++) {
                root->classDef.members[i] = optimizeAST(root->classDef.members[i]);
            }
            break;
        default:
            /* Para literales, identificadores y otros nodos, no se realiza optimización */
            break;
    }
    return root;
}
