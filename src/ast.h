#ifndef AST_H
#define AST_H

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

typedef struct AstNode AstNode; // Declaración adelantada para usar AstNode * en MethodCallNode

typedef struct {
    AstNode *object;         // Puntero a AstNode
    char method[256];
    AstNode **arguments;     // Array de punteros a AstNode
    int argCount;
} MethodCallNode;

struct AstNode {
    AstNodeType type;
    union {
        struct {
            AstNode **statements;
            int statementCount;
        } program;
        struct {
            char name[256];
            AstNode *value;
        } varAssign;
        struct {
            char name[256];
            char type[64];
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

AstNode *createAstNode(AstNodeType type);
void freeAstNode(AstNode *node);

#endif