#include "ast.h"
#include <stdlib.h>
#include <string.h>

AstNode *createAstNode(AstNodeType type) {
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node) return NULL;
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
            // Inicialización para método (soporte agregado)
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
                free(node->program.statements);
            }
            break;
        case AST_VAR_ASSIGN:
            freeAstNode(node->varAssign.value);
            break;
        case AST_VAR_DECL:
            break;
        case AST_FUNC_DEF:
            if (node->funcDef.parameters) {
                for (int i = 0; i < node->funcDef.paramCount; i++)
                    freeAstNode(node->funcDef.parameters[i]);
                free(node->funcDef.parameters);
            }
            if (node->funcDef.body) {
                for (int i = 0; i < node->funcDef.bodyCount; i++)
                    freeAstNode(node->funcDef.body[i]);
                free(node->funcDef.body);
            }
            break;
        case AST_FUNC_CALL:
            if (node->funcCall.arguments) {
                for (int i = 0; i < node->funcCall.argCount; i++)
                    freeAstNode(node->funcCall.arguments[i]);
                free(node->funcCall.arguments);
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
                for (int i = 0; i < node->lambda.paramCount; i++)
                    freeAstNode(node->lambda.parameters[i]);
                free(node->lambda.parameters);
            }
            freeAstNode(node->lambda.body);
            break;
        case AST_CLASS_DEF:
            if (node->classDef.members) {
                for (int i = 0; i < node->classDef.memberCount; i++)
                    freeAstNode(node->classDef.members[i]);
                free(node->classDef.members);
            }
            break;
        case AST_IF_STMT:
            freeAstNode(node->ifStmt.condition);
            if (node->ifStmt.thenBranch) {
                for (int i = 0; i < node->ifStmt.thenCount; i++)
                    freeAstNode(node->ifStmt.thenBranch[i]);
                free(node->ifStmt.thenBranch);
            }
            if (node->ifStmt.elseBranch) {
                for (int i = 0; i < node->ifStmt.elseCount; i++)
                    freeAstNode(node->ifStmt.elseBranch[i]);
                free(node->ifStmt.elseBranch);
            }
            break;
        case AST_FOR_STMT:
            freeAstNode(node->forStmt.rangeStart);
            freeAstNode(node->forStmt.rangeEnd);
            if (node->forStmt.body) {
                for (int i = 0; i < node->forStmt.bodyCount; i++)
                    freeAstNode(node->forStmt.body[i]);
                free(node->forStmt.body);
            }
            break;
        case AST_IMPORT:
            break;
        case AST_ARRAY_LITERAL:
            if (node->arrayLiteral.elements) {
                for (int i = 0; i < node->arrayLiteral.elementCount; i++)
                    freeAstNode(node->arrayLiteral.elements[i]);
                free(node->arrayLiteral.elements);
            }
            break;
        case AST_BINARY_OP:
            freeAstNode(node->binaryOp.left);
            freeAstNode(node->binaryOp.right);
            break;
        case AST_METHOD_CALL:
            freeAstNode(node->methodCall.object);
            if (node->methodCall.arguments) {
                for (int i = 0; i < node->methodCall.argCount; i++)
                    freeAstNode(node->methodCall.arguments[i]);
                free(node->methodCall.arguments);
            }
            break;
        default:
            break;
    }
    free(node);
}
