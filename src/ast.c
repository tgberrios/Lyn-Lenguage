#include "ast.h"
#include "memory.h"      // Incluye tanto los wrappers básicos como las funciones de pool.
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Declaración interna del pool para nodos AST.
   Se asume que se utiliza exclusivamente para nodos AST. */
static void initAstPool(void);
static void ensureAstPoolInitialized(void);

/* Declaración global (privada) del pool para nodos AST */
static void *astPoolMemory = NULL;  // Este no se usará directamente, se manejará a través de memory_pool_create.
static MemoryPool *astPool = NULL;

/**
 * @brief Inicializa el pool para nodos AST, si aún no está inicializado.
 */
static void ensureAstPoolInitialized(void) {
    if (!astPool) {
        /* Se crea un pool para, por ejemplo, 1024 nodos AST, 
           con blockSize igual a sizeof(AstNode) y una alineación de 16 bytes. */
        astPool = memory_pool_create(sizeof(AstNode), 1024, 16);
        if (!astPool) {
            fprintf(stderr, "Error: Failed to initialize AST memory pool.\n");
            exit(EXIT_FAILURE);
        }
    }
}

AstNode *createAstNode(AstNodeType type) {
    ensureAstPoolInitialized();
    AstNode *node = (AstNode *)memory_pool_alloc(astPool);
    if (!node) {
        fprintf(stderr, "Error: No free AST nodes available in the pool.\n");
        exit(EXIT_FAILURE);
    }
    memset(node, 0, sizeof(AstNode));
    node->type = type;
    switch (type) {
        case AST_PROGRAM:
            node->program.statements = NULL;
            node->program.statementCount = 0;
            break;
        case AST_VAR_ASSIGN:
            node->varAssign.name[0] = '\0';
            node->varAssign.value = NULL;
            break;
        case AST_VAR_DECL:
            node->varDecl.name[0] = '\0';
            node->varDecl.type[0] = '\0';
            break;
        case AST_FUNC_DEF:
            node->funcDef.name[0] = '\0';
            node->funcDef.parameters = NULL;
            node->funcDef.paramCount = 0;
            node->funcDef.returnType[0] = '\0';
            node->funcDef.body = NULL;
            node->funcDef.bodyCount = 0;
            break;
        case AST_FUNC_CALL:
            node->funcCall.name[0] = '\0';
            node->funcCall.arguments = NULL;
            node->funcCall.argCount = 0;
            break;
        case AST_RETURN_STMT:
            node->returnStmt.expr = NULL;
            break;
        case AST_PRINT_STMT:
            node->printStmt.expr = NULL;
            break;
        case AST_LAMBDA:
            node->lambda.parameters = NULL;
            node->lambda.paramCount = 0;
            node->lambda.returnType[0] = '\0';
            node->lambda.body = NULL;
            break;
        case AST_CLASS_DEF:
            node->classDef.name[0] = '\0';
            node->classDef.members = NULL;
            node->classDef.memberCount = 0;
            break;
        case AST_IF_STMT:
            node->ifStmt.condition = NULL;
            node->ifStmt.thenBranch = NULL;
            node->ifStmt.thenCount = 0;
            node->ifStmt.elseBranch = NULL;
            node->ifStmt.elseCount = 0;
            break;
        case AST_FOR_STMT:
            node->forStmt.iterator[0] = '\0';
            node->forStmt.rangeStart = NULL;
            node->forStmt.rangeEnd = NULL;
            node->forStmt.body = NULL;
            node->forStmt.bodyCount = 0;
            break;
        case AST_IMPORT:
            node->importStmt.moduleType[0] = '\0';
            node->importStmt.moduleName[0] = '\0';
            break;
        case AST_ARRAY_LITERAL:
            node->arrayLiteral.elements = NULL;
            node->arrayLiteral.elementCount = 0;
            break;
        case AST_BINARY_OP:
            node->binaryOp.left = NULL;
            node->binaryOp.op = '\0';
            node->binaryOp.right = NULL;
            break;
        case AST_NUMBER_LITERAL:
            node->numberLiteral.value = 0.0;
            break;
        case AST_STRING_LITERAL:
            node->stringLiteral.value[0] = '\0';
            break;
        case AST_IDENTIFIER:
            node->identifier.name[0] = '\0';
            break;
        case AST_MEMBER_ACCESS:
            node->memberAccess.object = NULL;
            node->memberAccess.member[0] = '\0';
            break;
        case AST_METHOD_CALL:
            node->methodCall.object = NULL;
            node->methodCall.method[0] = '\0';
            node->methodCall.arguments = NULL;
            node->methodCall.argCount = 0;
            break;
    }
    return node;
}

void freeAstNode(AstNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            if (node->program.statements) {
                for (int i = 0; i < node->program.statementCount; i++) {
                    freeAstNode(node->program.statements[i]);
                }
                memory_free(node->program.statements);
            }
            break;
        case AST_VAR_ASSIGN:
            freeAstNode(node->varAssign.value);
            break;
        case AST_VAR_DECL:
            break;
        case AST_FUNC_DEF:
            if (node->funcDef.parameters) {
                for (int i = 0; i < node->funcDef.paramCount; i++) {
                    freeAstNode(node->funcDef.parameters[i]);
                }
                memory_free(node->funcDef.parameters);
            }
            if (node->funcDef.body) {
                for (int i = 0; i < node->funcDef.bodyCount; i++) {
                    freeAstNode(node->funcDef.body[i]);
                }
                memory_free(node->funcDef.body);
            }
            break;
        case AST_FUNC_CALL:
            if (node->funcCall.arguments) {
                for (int i = 0; i < node->funcCall.argCount; i++) {
                    freeAstNode(node->funcCall.arguments[i]);
                }
                memory_free(node->funcCall.arguments);
            }
            break;
        case AST_RETURN_STMT:
            freeAstNode(node->returnStmt.expr);
            break;
        case AST_PRINT_STMT:
            freeAstNode(node->printStmt.expr);
            break;
        case AST_LAMBDA:
            if (node->lambda.parameters) {
                for (int i = 0; i < node->lambda.paramCount; i++) {
                    freeAstNode(node->lambda.parameters[i]);
                }
                memory_free(node->lambda.parameters);
            }
            freeAstNode(node->lambda.body);
            break;
        case AST_CLASS_DEF:
            if (node->classDef.members) {
                for (int i = 0; i < node->classDef.memberCount; i++) {
                    freeAstNode(node->classDef.members[i]);
                }
                memory_free(node->classDef.members);
            }
            break;
        case AST_IF_STMT:
            freeAstNode(node->ifStmt.condition);
            if (node->ifStmt.thenBranch) {
                for (int i = 0; i < node->ifStmt.thenCount; i++) {
                    freeAstNode(node->ifStmt.thenBranch[i]);
                }
                memory_free(node->ifStmt.thenBranch);
            }
            if (node->ifStmt.elseBranch) {
                for (int i = 0; i < node->ifStmt.elseCount; i++) {
                    freeAstNode(node->ifStmt.elseBranch[i]);
                }
                memory_free(node->ifStmt.elseBranch);
            }
            break;
        case AST_FOR_STMT:
            freeAstNode(node->forStmt.rangeStart);
            freeAstNode(node->forStmt.rangeEnd);
            if (node->forStmt.body) {
                for (int i = 0; i < node->forStmt.bodyCount; i++) {
                    freeAstNode(node->forStmt.body[i]);
                }
                memory_free(node->forStmt.body);
            }
            break;
        case AST_IMPORT:
            break;
        case AST_ARRAY_LITERAL:
            if (node->arrayLiteral.elements) {
                for (int i = 0; i < node->arrayLiteral.elementCount; i++) {
                    freeAstNode(node->arrayLiteral.elements[i]);
                }
                memory_free(node->arrayLiteral.elements);
            }
            break;
        case AST_BINARY_OP:
            freeAstNode(node->binaryOp.left);
            freeAstNode(node->binaryOp.right);
            break;
        case AST_METHOD_CALL:
            freeAstNode(node->methodCall.object);
            if (node->methodCall.arguments) {
                for (int i = 0; i < node->methodCall.argCount; i++) {
                    freeAstNode(node->methodCall.arguments[i]);
                }
                memory_free(node->methodCall.arguments);
            }
            break;
        default:
            break;
    }
    /* Retorna el nodo al pool en lugar de liberarlo con memory_free */
    memory_pool_free(astPool, node);
}
