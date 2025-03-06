#include "codegen.h"
#include "ast.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definición de la tabla de símbolos para el backend */
typedef struct Symbol {
    char name[256];
    struct Symbol *next;
} Symbol;

static Symbol *symbolTable = NULL;

/**
 * @brief Agrega un símbolo a la tabla.
 *
 * Usa memory_alloc para asignar memoria y agrega el símbolo al inicio de la lista.
 *
 * @param name Nombre del símbolo.
 */
static void addSymbol(const char *name) {
    Symbol *sym = (Symbol *)memory_alloc(sizeof(Symbol));
    strncpy(sym->name, name, sizeof(sym->name) - 1);
    sym->name[sizeof(sym->name) - 1] = '\0';
    sym->next = symbolTable;
    symbolTable = sym;
}

/**
 * @brief Verifica si un símbolo se encuentra en la tabla.
 *
 * @param name Nombre del símbolo.
 * @return int 1 si se encuentra, 0 en caso contrario.
 */
static int isSymbolInTable(const char *name) {
    Symbol *sym = symbolTable;
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return 1;
        sym = sym->next;
    }
    return 0;
}

/**
 * @brief Libera toda la tabla de símbolos.
 */
static void freeSymbolTable() {
    Symbol *sym = symbolTable;
    while (sym) {
        Symbol *tmp = sym;
        sym = sym->next;
        memory_free(tmp);
    }
    symbolTable = NULL;
}

/**
 * @brief Genera código ensamblador para una expresión.
 *
 * Esta función maneja casos básicos y realiza constant folding para expresiones
 * en las que ambos operandos son literales numéricos.
 *
 * @param expr Puntero al nodo AST de la expresión.
 * @param fp Puntero al archivo donde se escribe el ensamblador.
 */
static void generateExpression(AstNode *expr, FILE *fp) {
    switch(expr->type) {
        case AST_NUMBER_LITERAL:
            fprintf(fp, "    mov rax, %f\n", expr->numberLiteral.value);
            break;
        case AST_STRING_LITERAL:
            fprintf(fp, "    lea rax, [rip+str_%s]\n", expr->stringLiteral.value);
            break;
        case AST_IDENTIFIER:
            fprintf(fp, "    mov rax, [%s]\n", expr->identifier.name);
            break;
        case AST_BINARY_OP: {
            /* Constant folding: si ambos operandos son literales numéricos, evaluamos en compilación. */
            if (expr->binaryOp.left->type == AST_NUMBER_LITERAL &&
                expr->binaryOp.right->type == AST_NUMBER_LITERAL) {
                double leftVal = expr->binaryOp.left->numberLiteral.value;
                double rightVal = expr->binaryOp.right->numberLiteral.value;
                double res = 0.0;
                switch (expr->binaryOp.op) {
                    case '+': res = leftVal + rightVal; break;
                    case '-': res = leftVal - rightVal; break;
                    case '*': res = leftVal * rightVal; break;
                    case '/': res = leftVal / rightVal; break;
                    default: break;
                }
                fprintf(fp, "    mov rax, %f\n", res);
            } else {
                generateExpression(expr->binaryOp.left, fp);
                fprintf(fp, "    push rax\n");
                generateExpression(expr->binaryOp.right, fp);
                fprintf(fp, "    pop rbx\n");
                if (expr->binaryOp.op == '+') {
                    fprintf(fp, "    add rax, rbx\n");
                } else if (expr->binaryOp.op == '-') {
                    fprintf(fp, "    sub rbx, rax\n");
                    fprintf(fp, "    mov rax, rbx\n");
                } else if (expr->binaryOp.op == '*') {
                    fprintf(fp, "    imul rax, rbx\n");
                } else if (expr->binaryOp.op == '/') {
                    fprintf(fp, "    mov rcx, rax\n");
                    fprintf(fp, "    pop rbx\n");
                    fprintf(fp, "    mov rax, rbx\n");
                    fprintf(fp, "    xor rdx, rdx\n");
                    fprintf(fp, "    idiv rcx\n");
                }
            }
            break;
        }
        default:
            /* En caso de nodo no soportado, se genera una advertencia en el ensamblador. */
            fprintf(fp, "    ; ERROR: Node type %d not supported in codegen\n", expr->type);
    }
}

/**
 * @brief Genera código para una sentencia.
 *
 * Dependiendo del tipo de nodo AST, esta función genera el código adecuado.
 *
 * @param node Nodo AST de la sentencia.
 * @param fp Puntero al archivo donde se escribe el código ensamblador.
 */
static void generateStatement(AstNode *node, FILE *fp) {
    if (node->type == AST_VAR_ASSIGN) {
        /* Para asignaciones: se agrega la variable a la tabla de símbolos si no existe. */
        if (!isSymbolInTable(node->varAssign.name)) {
            addSymbol(node->varAssign.name);
        }
        /* Aquí podrías agregar código para evaluar y almacenar el valor en memoria. */
    } else if (node->type == AST_PRINT_STMT) {
        generateExpression(node->printStmt.expr, fp);
        fprintf(fp, "    mov rsi, rax\n");
        fprintf(fp, "    lea rdi, [rip+fmt]\n");
        fprintf(fp, "    xor eax, eax\n");
        fprintf(fp, "    call printf\n");
    } else if (node->type == AST_FUNC_DEF) {
        fprintf(fp, "\n.global %s\n%s:\n", node->funcDef.name, node->funcDef.name);
        for (int i = 0; i < node->funcDef.bodyCount; i++) {
            generateStatement(node->funcDef.body[i], fp);
        }
        fprintf(fp, "    ret\n");
    } else if (node->type == AST_RETURN_STMT) {
        generateExpression(node->returnStmt.expr, fp);
        fprintf(fp, "    ret\n");
    } else {
        generateExpression(node, fp);
    }
}

/**
 * @brief Genera código ensamblador a partir del AST y lo escribe en un archivo.
 *
 * Esta función recorre el AST, construye la tabla de símbolos y genera las secciones
 * de datos y texto del ensamblador.
 *
 * @param root Puntero al nodo raíz del AST.
 * @param filename Nombre del archivo de salida.
 */
void generateCode(AstNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error opening file for code generation.\n");
        exit(1);
    }
    /* Recorrer el AST para construir la tabla de símbolos */
    for (int i = 0; i < root->program.statementCount; i++) {
        AstNode *stmt = root->program.statements[i];
        if (stmt->type == AST_VAR_ASSIGN) {
            if (!isSymbolInTable(stmt->varAssign.name))
                addSymbol(stmt->varAssign.name);
        }
    }
    fprintf(fp, ".intel_syntax noprefix\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "fmt: .asciz \"Result: %%ld\\n\"\n");
    Symbol *sym = symbolTable;
    while (sym) {
        int value = 0;
        for (int i = 0; i < root->program.statementCount; i++) {
            AstNode *stmt = root->program.statements[i];
            if (stmt->type == AST_VAR_ASSIGN && strcmp(stmt->varAssign.name, sym->name) == 0) {
                if (stmt->varAssign.value && stmt->varAssign.value->type == AST_NUMBER_LITERAL)
                    value = (int)stmt->varAssign.value->numberLiteral.value;
                break;
            }
        }
        fprintf(fp, "%s: .quad %d\n", sym->name, value);
        sym = sym->next;
    }
    fprintf(fp, ".text\n.global main\n");
    fprintf(fp, "main:\n");
    for (int i = 0; i < root->program.statementCount; i++) {
        AstNode *stmt = root->program.statements[i];
        if (stmt->type != AST_FUNC_DEF)
            generateStatement(stmt, fp);
    }
    fprintf(fp, "    mov rax, 60\n");
    fprintf(fp, "    xor rdi, rdi\n");
    fprintf(fp, "    syscall\n");
    fclose(fp);
    freeSymbolTable();
}
