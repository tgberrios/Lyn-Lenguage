#ifndef AST_H
#define AST_H

#include <stddef.h>

/* Enumeración de los tipos de nodos del AST */
typedef enum {
    AST_PROGRAM,
    AST_VAR_ASSIGN,
    AST_VAR_DECL,
    AST_FUNC_DEF,
    AST_FUNC_CALL,
    AST_RETURN_STMT,
    AST_PRINT_STMT,
    AST_LAMBDA,
    AST_CLASS_DEF,
    AST_IF_STMT,
    AST_FOR_STMT,
    AST_IMPORT,
    AST_ARRAY_LITERAL,
    AST_BINARY_OP,
    AST_NUMBER_LITERAL,
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_MEMBER_ACCESS,
    AST_METHOD_CALL
} AstNodeType;

/* Declaración adelantada para usar en MethodCallNode */
typedef struct AstNode AstNode;

/* Nodo para llamadas a métodos */
typedef struct {
    AstNode *object;
    char method[256];
    AstNode **arguments;
    int argCount;
} MethodCallNode;

/* Definición del nodo AST */
struct AstNode {
    AstNodeType type;
    union {
        struct {
            AstNode **statements;
            int statementCount;
        } program;
        struct {
            char name[256];
            AstNode *initializer;
        } varAssign;
        struct {
            char name[256];
            char type[64];
            AstNode *initializer;
        } varDecl;
        struct {
            char name[256];
            AstNode **parameters;
            int paramCount;
            char returnType[64];
            AstNode **body;
            int bodyCount;
        } funcDef;
        struct {
            char name[256];
            AstNode **arguments;
            int argCount;
        } funcCall;
        struct {
            AstNode *expr;
        } returnStmt;
        struct {
            AstNode *expr;
        } printStmt;
        struct {
            AstNode **parameters;
            int paramCount;
            char returnType[64];
            AstNode *body;
        } lambda;
        struct {
            char name[256];
            AstNode **members;
            int memberCount;
        } classDef;
        struct {
            AstNode *condition;
            AstNode **thenBranch;
            int thenCount;
            AstNode **elseBranch;
            int elseCount;
        } ifStmt;
        struct {
            char iterator[256];
            AstNode *rangeStart;
            AstNode *rangeEnd;
            AstNode **body;
            int bodyCount;
        } forStmt;
        struct {
            char moduleType[64];
            char moduleName[256];
        } importStmt;
        struct {
            AstNode **elements;
            int elementCount;
        } arrayLiteral;
        struct {
            AstNode *left;
            char op;
            AstNode *right;
        } binaryOp;
        struct {
            double value;
        } numberLiteral;
        struct {
            char value[256];
        } stringLiteral;
        struct {
            char name[256];
        } identifier;
        struct {
            AstNode *object;
            char member[256];
        } memberAccess;
        MethodCallNode methodCall;
    };
};

/**
 * @brief Crea un nuevo nodo AST del tipo especificado.
 *
 * @param type Tipo del nodo AST.
 * @return AstNode* Puntero al nodo AST creado.
 */
AstNode *createAstNode(AstNodeType type);

/**
 * @brief Libera la memoria utilizada por un nodo AST y sus descendientes.
 *
 * @param node Puntero al nodo AST a liberar.
 */
void freeAstNode(AstNode *node);

#endif /* AST_H */
