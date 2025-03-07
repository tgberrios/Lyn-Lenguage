#include "codegen.h"
#include "ast.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================
   Backend Symbol Table (variables globales)
   ========================================================== */

typedef struct Symbol {
    char name[256];
    struct Symbol *next;
} Symbol;

static Symbol *symbolTable = NULL;

/* Agrega símbolo global */
static void addSymbol(const char *name) {
    Symbol *sym = (Symbol *)memory_alloc(sizeof(Symbol));
    strncpy(sym->name, name, sizeof(sym->name) - 1);
    sym->name[sizeof(sym->name) - 1] = '\0';
    sym->next = symbolTable;
    symbolTable = sym;
}

/* Verifica si la variable global existe */
static int isSymbolInTable(const char *name) {
    Symbol *sym = symbolTable;
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return 1;
        sym = sym->next;
    }
    return 0;
}

/* Libera la tabla de símbolos al terminar la compilación */
static void freeSymbolTable() {
    Symbol *sym = symbolTable;
    while (sym) {
        Symbol *tmp = sym;
        sym = sym->next;
        memory_free(tmp);
    }
    symbolTable = NULL;
}

/* ==========================================================
   Utilidades para etiquetas en ensamblador
   ========================================================== */

/**
 * Genera un label único para if/for/bloques anidados.
 * Ejemplo: ".IF_0", ".END_0", etc.
 */
static int labelCount = 0;
static void getNewLabel(char *buffer, const char *prefix) {
    sprintf(buffer, ".%s_%d", prefix, labelCount++);
}

/* ==========================================================
   Prototipos privados
   ========================================================== */
static void generateExpression(AstNode *expr, FILE *fp);
static void generateStatement(AstNode *stmt, FILE *fp);

/* ==========================================================
   generateExpression
   Genera ASM que deja el resultado en RAX
   ========================================================== */
static void generateExpression(AstNode *expr, FILE *fp) {
    if (!expr) {
        fprintf(fp, "    ; (expresión nula)\n");
        return;
    }

    switch (expr->type) {

    case AST_NUMBER_LITERAL: {
        /* Asumimos literales como enteros. 
           Para floats reales, debería manejarse en registros xmm. */
        long val = (long)expr->numberLiteral.value;
        fprintf(fp, "    mov rax, %ld    ; cargar literal numérico\n", val);
        break;
    }

    case AST_STRING_LITERAL: {
        /* Tu parser o sistema podría crear .data con "str_<literal>:".
           Aquí suponemos que .data lo define. */
        fprintf(fp, "    lea rax, [rip+str_%s]    ; cargar literal string\n",
                expr->stringLiteral.value);
        break;
    }

    case AST_IDENTIFIER: {
        /* Cargar variable global en RAX */
        fprintf(fp, "    mov rax, [%s]    ; cargar variable global\n",
                expr->identifier.name);
        break;
    }

    case AST_BINARY_OP: {
        /* Manejo aritmético y ahora también comparaciones como '>' */
        AstNode *L = expr->binaryOp.left;
        AstNode *R = expr->binaryOp.right;
        char op = expr->binaryOp.op;

        // Genera L en RAX
        generateExpression(L, fp);
        fprintf(fp, "    push rax          ; stack = L\n");

        // Genera R en RAX
        generateExpression(R, fp);
        fprintf(fp, "    pop rbx           ; rbx = L, rax = R\n");

        switch (op) {
            case '+':
                fprintf(fp, "    add rax, rbx    ; rax = L + R\n");
                break;
            case '-':
                fprintf(fp, "    sub rbx, rax    ; rbx = L - R\n");
                fprintf(fp, "    mov rax, rbx\n");
                break;
            case '*':
                fprintf(fp, "    imul rax, rbx   ; rax = L * R\n");
                break;
            case '/':
                /* Div entero: rax=dividendo, rcx=divisor, rdx=0 */
                fprintf(fp, "    mov rcx, rax    ; divisor en rcx (R)\n");
                fprintf(fp, "    mov rax, rbx    ; dividendo en rax (L)\n");
                fprintf(fp, "    xor rdx, rdx\n");
                fprintf(fp, "    idiv rcx        ; rax = L / R\n");
                break;

            // NUEVO: Comparaciones
            case '>':
                // Queremos rax = (L > R) ? 1 : 0
                // Para ello: rbx = L, rax = R
                // 1) cmp rbx, rax => L ? R
                // 2) setg al => al=1 si L>R; sino 0
                // 3) zero-extend rax
                fprintf(fp, "    cmp rbx, rax    ; compara L con R\n");
                fprintf(fp, "    setg al         ; al=1 si L>R, 0 si no\n");
                fprintf(fp, "    movzb rax, al   ; rax = 0/1\n");
                break;

            // Añade más si quieres >=, <, <=, ==
            default:
                fprintf(fp, "    ; ERROR: Operador '%c' no soportado\n", op);
                break;
        }
        break;
    }

    case AST_FUNC_CALL: {
        /* Llamada a función:
           - Generar args, push en orden
           - call <nombre>
           - RAX = retorno */
        for (int i = 0; i < expr->funcCall.argCount; i++) {
            generateExpression(expr->funcCall.arguments[i], fp);
            fprintf(fp, "    push rax    ; argumento %d\n", i);
        }
        fprintf(fp, "    call %s\n", expr->funcCall.name);
        break;
    }

    case AST_METHOD_CALL: {
        // object.method(...)
        generateExpression(expr->methodCall.object, fp);
        fprintf(fp, "    push rax    ; push 'this'\n");
        for (int i = 0; i < expr->methodCall.argCount; i++) {
            generateExpression(expr->methodCall.arguments[i], fp);
            fprintf(fp, "    push rax    ; arg %d\n", i);
        }
        fprintf(fp, "    call %s\n", expr->methodCall.method);
        break;
    }

    case AST_MEMBER_ACCESS: {
        /* Acceso a object.member => En un compilador real, 
           se calcula la dirección con offsets. Aquí es un placeholder. */
        fprintf(fp, "    ; (pendiente) Acceso a miembro: %s\n",
                expr->memberAccess.member);
        break;
    }

    // NUEVO: Manejar AST_LAMBDA si llegara como *expresión*
    case AST_LAMBDA: {
        fprintf(fp, "    ; (lambda expresion) => STUB, no implementado\n");
        // Podrías generar un label anónimo y devolver su dirección
        // ...
        break;
    }

    // NUEVO: Manejar AST_VAR_DECL si aparece en expr context
    case AST_VAR_DECL: {
        // Normalmente no generas nada como expresión, 
        // porque "varDecl" es declaración. Dejamos un stub:
        fprintf(fp, "    ; (declaración varDecl) => sin acción en codegen expr\n");
        break;
    }

    case AST_ARRAY_LITERAL: {
        fprintf(fp, "    ; (array literal) => no implementado (stub)\n");
        break;
    }

    case AST_CLASS_DEF: {
        // Si aparece como expresión, lo ignoramos
        fprintf(fp, "    ; (class def en expr) => sin acción\n");
        break;
    }

    case AST_IMPORT: {
        fprintf(fp, "    ; (import en expr) => no-op\n");
        break;
    }

    default:
        fprintf(fp, "    ; ERROR: Expresión nodo tipo %d no soportado\n",
                expr->type);
        break;
    }
}

/* ==========================================================
   generateStatement
   Genera ASM para sentencias: if, for, asignaciones, etc.
   ========================================================== */
static void generateStatement(AstNode *stmt, FILE *fp) {
    fprintf(fp, "    ; ---- Inicio Sentencia ----\n");

    if (!stmt) {
        fprintf(fp, "    ; (sentencia nula)\n");
        fprintf(fp, "    ; ---- Fin Sentencia ----\n\n");
        return;
    }

    switch (stmt->type) {

    case AST_VAR_ASSIGN: {
        /* Registrar variable si no está */
        if (!isSymbolInTable(stmt->varAssign.name)) {
            addSymbol(stmt->varAssign.name);
        }
        /* Generar expresión y asignarla */
        generateExpression(stmt->varAssign.initializer, fp);
        fprintf(fp, "    mov [%s], rax    ; asignación\n", 
                stmt->varAssign.name);
        break;
    }

    // NUEVO: Soportar declaración de variable (AST_VAR_DECL)
    case AST_VAR_DECL: {
        // Si trae un initializer, asignarlo
        if (!isSymbolInTable(stmt->varDecl.name)) {
            addSymbol(stmt->varDecl.name);
        }
        if (stmt->varDecl.initializer) {
            generateExpression(stmt->varDecl.initializer, fp);
            fprintf(fp, "    mov [%s], rax ; inicializar varDecl\n",
                    stmt->varDecl.name);
        } else {
            // Sin init => se deja en 0
            fprintf(fp, "    ; varDecl '%s' sin init => nada\n",
                    stmt->varDecl.name);
        }
        break;
    }

    case AST_PRINT_STMT: {
        generateExpression(stmt->printStmt.expr, fp);
        fprintf(fp, "    mov rsi, rax    ; param arg\n");
        fprintf(fp, "    lea rdi, [rip+fmt] ; \"Result: %%ld\\n\"\n");
        fprintf(fp, "    xor eax, eax\n");
        fprintf(fp, "    call printf\n");
        break;
    }

    case AST_FUNC_DEF: {
        fprintf(fp, "\n.global %s\n", stmt->funcDef.name);
        fprintf(fp, "%s:\n", stmt->funcDef.name);
        for (int i = 0; i < stmt->funcDef.bodyCount; i++) {
            generateStatement(stmt->funcDef.body[i], fp);
        }
        fprintf(fp, "    ret    ; fin de funcion %s\n", stmt->funcDef.name);
        break;
    }

    case AST_RETURN_STMT: {
        generateExpression(stmt->returnStmt.expr, fp);
        fprintf(fp, "    ret\n");
        break;
    }

    case AST_IF_STMT: {
        char labelElse[32], labelEnd[32];
        getNewLabel(labelElse, "ELSE");
        getNewLabel(labelEnd, "ENDIF");

        generateExpression(stmt->ifStmt.condition, fp);
        fprintf(fp, "    cmp rax, 0\n");
        fprintf(fp, "    je %s\n", labelElse);

        for (int i = 0; i < stmt->ifStmt.thenCount; i++) {
            generateStatement(stmt->ifStmt.thenBranch[i], fp);
        }
        fprintf(fp, "    jmp %s\n", labelEnd);

        fprintf(fp, "%s:\n", labelElse);
        for (int i = 0; i < stmt->ifStmt.elseCount; i++) {
            generateStatement(stmt->ifStmt.elseBranch[i], fp);
        }

        fprintf(fp, "%s:\n", labelEnd);
        break;
    }

    case AST_FOR_STMT: {
        if (!isSymbolInTable(stmt->forStmt.iterator)) {
            addSymbol(stmt->forStmt.iterator);
        }
        char labelLoop[32], labelEnd[32];
        getNewLabel(labelLoop, "LOOP");
        getNewLabel(labelEnd, "LOOPEND");

        generateExpression(stmt->forStmt.rangeStart, fp);
        fprintf(fp, "    mov [%s], rax\n", stmt->forStmt.iterator);

        fprintf(fp, "%s:\n", labelLoop);
        generateExpression(stmt->forStmt.rangeEnd, fp);
        fprintf(fp, "    mov rbx, rax    ; rbx = end\n");
        fprintf(fp, "    mov rax, [%s]   ; rax = i\n", stmt->forStmt.iterator);
        fprintf(fp, "    cmp rax, rbx\n");
        fprintf(fp, "    jge %s\n", labelEnd);

        for (int i = 0; i < stmt->forStmt.bodyCount; i++) {
            generateStatement(stmt->forStmt.body[i], fp);
        }

        fprintf(fp, "    mov rax, [%s]\n", stmt->forStmt.iterator);
        fprintf(fp, "    add rax, 1\n");
        fprintf(fp, "    mov [%s], rax\n", stmt->forStmt.iterator);
        fprintf(fp, "    jmp %s\n", labelLoop);

        fprintf(fp, "%s:\n", labelEnd);
        break;
    }

    case AST_IMPORT: {
        fprintf(fp, "    ; (import %s %s) => sin efecto real\n",
                stmt->importStmt.moduleType,
                stmt->importStmt.moduleName);
        break;
    }

    case AST_CLASS_DEF: {
        fprintf(fp, "    ; (class %s) => pendiente de implementación real\n",
                stmt->classDef.name);
        break;
    }

    case AST_LAMBDA: {
        fprintf(fp, "    ; (lambda) => pendiente de implementación real\n");
        break;
    }

    case AST_ARRAY_LITERAL: {
        fprintf(fp, "    ; (array literal) => pendiente de implementación real\n");
        break;
    }

    // Si no coincide con nada, podría ser una "expresión suelta"
    default:
        generateExpression(stmt, fp);
        break;
    }

    fprintf(fp, "    ; ---- Fin Sentencia ----\n\n");
}

/* ==========================================================
   generateCode
   Punto de entrada principal para generar el .s
   ========================================================== */
void generateCode(AstNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error al abrir archivo de salida.\n");
        exit(1);
    }

    /* Si la raíz es un AST_PROGRAM, revisa variables globales */
    if (root->type == AST_PROGRAM) {
        for (int i = 0; i < root->program.statementCount; i++) {
            AstNode *st = root->program.statements[i];
            if (st->type == AST_VAR_ASSIGN) {
                if (!isSymbolInTable(st->varAssign.name)) {
                    addSymbol(st->varAssign.name);
                }
            }
            // NUEVO: Si hay varDecl como global, también se registra
            else if (st->type == AST_VAR_DECL) {
                if (!isSymbolInTable(st->varDecl.name)) {
                    addSymbol(st->varDecl.name);
                }
            }
        }
    }

    /* Sección .data */
    fprintf(fp, ".intel_syntax noprefix\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "fmt: .asciz \"Result: %%ld\\n\"\n\n");

    // Generar espacios en .data para cada variable global
    Symbol *sym = symbolTable;
    while (sym) {
        fprintf(fp, "%s: .quad 0\n", sym->name);
        sym = sym->next;
    }

    /* Sección .text */
    fprintf(fp, "\n.text\n.global main\n");

    if (root->type == AST_PROGRAM) {
        fprintf(fp, "main:\n");
        for (int i = 0; i < root->program.statementCount; i++) {
            generateStatement(root->program.statementCount > 0
                                ? root->program.statements[i]
                                : NULL,
                              fp);
        }
        /* exit syscall */
        fprintf(fp, "    mov rax, 60    ; exit\n");
        fprintf(fp, "    xor rdi, rdi   ; status=0\n");
        fprintf(fp, "    syscall\n");
    } else {
        /* main "implícito" */
        fprintf(fp, "main:\n");
        generateStatement(root, fp);
        fprintf(fp, "    mov rax, 60\n");
        fprintf(fp, "    xor rdi, rdi\n");
        fprintf(fp, "    syscall\n");
    }

    fclose(fp);
    freeSymbolTable();
}
