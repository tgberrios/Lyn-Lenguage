/* codegen.c */
#include "codegen.h"
#include "ast.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tabla de símbolos para el backend (usada para variables globales) */
typedef struct Symbol {
    char name[256];
    struct Symbol *next;
} Symbol;

static Symbol *symbolTable = NULL;

/* Funciones para la tabla de símbolos */
static void addSymbol(const char *name) {
    Symbol *sym = (Symbol *)memory_alloc(sizeof(Symbol));
    strncpy(sym->name, name, sizeof(sym->name) - 1);
    sym->name[sizeof(sym->name) - 1] = '\0';
    sym->next = symbolTable;
    symbolTable = sym;
}

static int isSymbolInTable(const char *name) {
    Symbol *sym = symbolTable;
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return 1;
        sym = sym->next;
    }
    return 0;
}

static void freeSymbolTable() {
    Symbol *sym = symbolTable;
    while (sym) {
        Symbol *tmp = sym;
        sym = sym->next;
        memory_free(tmp);
    }
    symbolTable = NULL;
}

/* -------------------------------------------------------------------------- */
/*         Generación de Código con Optimización de Instrucciones             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Genera código ensamblador para una expresión.
 *
 * Se aplican optimizaciones como el uso de instrucciones inmediatas cuando uno de los
 * operandos es un literal numérico. Además, se añaden comentarios de depuración.
 *
 * @param expr Puntero al nodo AST de la expresión.
 * @param fp Archivo donde se escribe el ensamblador.
 */
static void generateExpression(AstNode *expr, FILE *fp) {
    switch(expr->type) {
        case AST_NUMBER_LITERAL:
            /* Cargar un literal numérico en rax */
            fprintf(fp, "    mov rax, %f    ; cargar literal numérico\n", expr->numberLiteral.value);
            break;
        case AST_STRING_LITERAL:
            /* Cargar la dirección de una cadena literal */
            fprintf(fp, "    lea rax, [rip+str_%s]    ; cargar literal de cadena\n", expr->stringLiteral.value);
            break;
        case AST_IDENTIFIER:
            /* Cargar el valor de una variable (asumimos que su dirección es conocida) */
            fprintf(fp, "    mov rax, [%s]    ; cargar valor de variable\n", expr->identifier.name);
            break;
        case AST_BINARY_OP: {
            /* Optimización de operaciones inmediatas:
               Si el operando derecho es un literal, se utiliza una instrucción inmediata. */
            if (expr->binaryOp.right->type == AST_NUMBER_LITERAL) {
                generateExpression(expr->binaryOp.left, fp);  // Resultado en rax
                double imm = expr->binaryOp.right->numberLiteral.value;
                switch (expr->binaryOp.op) {
                    case '+':
                        fprintf(fp, "    add rax, %f    ; suma inmediata\n", imm);
                        break;
                    case '-':
                        fprintf(fp, "    sub rax, %f    ; resta inmediata\n", imm);
                        break;
                    case '*':
                        fprintf(fp, "    imul rax, %f   ; multiplicación inmediata\n", imm);
                        break;
                    case '/':
                        /* Para la división, cargamos el divisor en rcx y usamos idiv */
                        fprintf(fp, "    mov rcx, %f    ; divisor inmediato\n", imm);
                        fprintf(fp, "    xor rdx, rdx   ; limpiar rdx\n");
                        fprintf(fp, "    idiv rcx       ; división\n");
                        break;
                    default:
                        fprintf(fp, "    ; ERROR: Operador '%c' no soportado\n", expr->binaryOp.op);
                        break;
                }
            } else {
                /* Caso general: ambos operandos son expresiones complejas.
                   Se usa el mecanismo push/pop para salvar el valor de la izquierda. */
                generateExpression(expr->binaryOp.left, fp);
                fprintf(fp, "    push rax          ; guardar operando izquierdo\n");
                generateExpression(expr->binaryOp.right, fp);
                fprintf(fp, "    pop rbx           ; recuperar operando izquierdo\n");
                switch (expr->binaryOp.op) {
                    case '+':
                        fprintf(fp, "    add rax, rbx    ; suma\n");
                        break;
                    case '-':
                        fprintf(fp, "    sub rbx, rax    ; resta\n");
                        fprintf(fp, "    mov rax, rbx\n");
                        break;
                    case '*':
                        fprintf(fp, "    imul rax, rbx   ; multiplicación\n");
                        break;
                    case '/':
                        fprintf(fp, "    mov rcx, rax    ; preparar divisor\n");
                        fprintf(fp, "    pop rbx         ; recuperar operando izquierdo\n");
                        fprintf(fp, "    mov rax, rbx    ; mover operando a rax\n");
                        fprintf(fp, "    xor rdx, rdx    ; limpiar rdx\n");
                        fprintf(fp, "    idiv rcx        ; división\n");
                        break;
                    default:
                        fprintf(fp, "    ; ERROR: Operador '%c' no soportado\n", expr->binaryOp.op);
                        break;
                }
            }
            break;
        }
        default:
            fprintf(fp, "    ; ERROR: Nodo tipo %d no soportado en generación de código\n", expr->type);
            break;
    }
}

/**
 * @brief Genera código para una sentencia, añadiendo comentarios de depuración.
 *
 * Se agregan comentarios que delimitan el inicio y fin de cada sentencia.
 *
 * @param node Nodo AST de la sentencia.
 * @param fp Archivo de salida para el ensamblador.
 */
static void generateStatement(AstNode *node, FILE *fp) {
    fprintf(fp, "    ; ---- Inicio Sentencia ----\n");
    switch (node->type) {
        case AST_VAR_ASSIGN:
            /* Antes de generar el código, registrar la variable en la tabla de símbolos global */
            if (!isSymbolInTable(node->varAssign.name)) {
                addSymbol(node->varAssign.name);
            }
            /* Usar el campo 'initializer' en lugar de 'value' */
            generateExpression(node->varAssign.initializer, fp);
            fprintf(fp, "    mov [%s], rax    ; asignación de variable\n", node->varAssign.name);
            break;
        case AST_PRINT_STMT:
            generateExpression(node->printStmt.expr, fp);
            fprintf(fp, "    mov rsi, rax    ; prepara argumento para printf\n");
            fprintf(fp, "    lea rdi, [rip+fmt]    ; formato de impresión\n");
            fprintf(fp, "    xor eax, eax\n");
            fprintf(fp, "    call printf\n");
            break;
        case AST_FUNC_DEF:
            fprintf(fp, "\n.global %s\n%s:\n", node->funcDef.name, node->funcDef.name);
            for (int i = 0; i < node->funcDef.bodyCount; i++) {
                generateStatement(node->funcDef.body[i], fp);
            }
            fprintf(fp, "    ret    ; fin de función\n");
            break;
        case AST_RETURN_STMT:
            generateExpression(node->returnStmt.expr, fp);
            fprintf(fp, "    ret    ; return\n");
            break;
        default:
            generateExpression(node, fp);
            break;
    }
    fprintf(fp, "    ; ---- Fin Sentencia ----\n\n");
}

/**
 * @brief Genera el código ensamblador final a partir del AST.
 *
 * Recorre el AST, construye la tabla de símbolos global (para variables globales)
 * y genera las secciones de datos y texto del ensamblador. También incluye comentarios
 * para facilitar la depuración.
 *
 * @param root Puntero al nodo raíz del AST.
 * @param filename Nombre del archivo de salida.
 */
void generateCode(AstNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error al abrir el archivo para generación de código.\n");
        exit(1);
    }

    /* Construir la tabla de símbolos global revisando asignaciones */
    for (int i = 0; i < root->program.statementCount; i++) {
        AstNode *stmt = root->program.statements[i];
        if (stmt->type == AST_VAR_ASSIGN) {
            if (!isSymbolInTable(stmt->varAssign.name))
                addSymbol(stmt->varAssign.name);
        }
    }

    /* Sección de datos */
    fprintf(fp, ".intel_syntax noprefix\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "fmt: .asciz \"Result: %%ld\\n\"\n");
    /* Aquí se podrían añadir otras variables globales o literales */

    /* Sección de texto */
    fprintf(fp, ".text\n.global main\n");
    fprintf(fp, "main:\n");

    /* Generar código para cada sentencia (excepto definiciones de funciones que se
       generan de forma separada) */
    for (int i = 0; i < root->program.statementCount; i++) {
        AstNode *stmt = root->program.statements[i];
        if (stmt->type != AST_FUNC_DEF)
            generateStatement(stmt, fp);
    }

    /* Código para salir del programa */
    fprintf(fp, "    mov rax, 60    ; syscall: exit\n");
    fprintf(fp, "    xor rdi, rdi   ; status 0\n");
    fprintf(fp, "    syscall\n");

    fclose(fp);
    freeSymbolTable();
}
