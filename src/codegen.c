#include "codegen.h"
#include "ast.h"
#include "memory.h"
#include "arch.h"
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

/* ==========================================================
   Utilidades para etiquetas en ensamblador
   ========================================================== */
static int labelCount = 0;
static void getNewLabel(char *buffer, const char *prefix) {
    sprintf(buffer, ".%s_%d", prefix, labelCount++);
}

/* ==========================================================
   Prototipos privados
   ========================================================== */
static void generateExpression(AstNode *expr);
static void generateStatement(AstNode *stmt);

/* ==========================================================
   generateExpression
   Genera código usando el backend y deja el resultado en el registro principal.
   ========================================================== */
static void generateExpression(AstNode *expr) {
    if (!expr) {
        fprintf(g_backend->out, "    ; (expresión nula)\n");
        return;
    }
    switch (expr->type) {
    case AST_NUMBER_LITERAL: {
        long val = (long)expr->numberLiteral.value;
        g_backend->emitLoadImmInt(val);
        break;
    }
    case AST_STRING_LITERAL: {
        fprintf(g_backend->out, "    lea rax, [rip+str_%s]    ; cargar literal string\n",
                expr->stringLiteral.value);
        break;
    }
    case AST_IDENTIFIER: {
        g_backend->emitLoadGlobal(expr->identifier.name);
        break;
    }
    case AST_BINARY_OP: {
        AstNode *L = expr->binaryOp.left;
        AstNode *R = expr->binaryOp.right;
        char op = expr->binaryOp.op;
        generateExpression(L);
        fprintf(g_backend->out, "    push rax          ; stack = L\n");
        generateExpression(R);
        fprintf(g_backend->out, "    pop rbx           ; rbx = L, rax = R\n");
        switch (op) {
            case '+':
                g_backend->emitAdd();
                break;
            case '-':
                g_backend->emitSub();
                break;
            case '*':
                g_backend->emitImul();
                break;
            case '/':
                g_backend->emitIDiv();
                break;
            case '>':
                g_backend->emitCmpGreater();
                break;
            default:
                fprintf(g_backend->out, "    ; ERROR: Operador '%c' no soportado\n", op);
                break;
        }
        break;
    }
    case AST_FUNC_CALL: {
        for (int i = 0; i < expr->funcCall.argCount; i++) {
            generateExpression(expr->funcCall.arguments[i]);
            fprintf(g_backend->out, "    push rax    ; argumento %d\n", i);
        }
        fprintf(g_backend->out, "    call %s\n", expr->funcCall.name);
        break;
    }
    case AST_METHOD_CALL: {
        generateExpression(expr->methodCall.object);
        fprintf(g_backend->out, "    push rax    ; push 'this'\n");
        for (int i = 0; i < expr->methodCall.argCount; i++) {
            generateExpression(expr->methodCall.arguments[i]);
            fprintf(g_backend->out, "    push rax    ; arg %d\n", i);
        }
        fprintf(g_backend->out, "    call %s\n", expr->methodCall.method);
        break;
    }
    case AST_MEMBER_ACCESS: {
        fprintf(g_backend->out, "    ; (pendiente) Acceso a miembro: %s\n", expr->memberAccess.member);
        break;
    }
    case AST_LAMBDA: {
        fprintf(g_backend->out, "    ; (lambda expresion) => STUB, no implementado\n");
        break;
    }
    case AST_VAR_DECL: {
        fprintf(g_backend->out, "    ; (varDecl en expr) => sin acción\n");
        break;
    }
    case AST_ARRAY_LITERAL: {
        fprintf(g_backend->out, "    ; (array literal) => pendiente de implementación\n");
        break;
    }
    case AST_CLASS_DEF: {
        fprintf(g_backend->out, "    ; (class def en expr) => sin acción\n");
        break;
    }
    case AST_IMPORT: {
        fprintf(g_backend->out, "    ; (import en expr) => no-op\n");
        break;
    }
    default:
        fprintf(g_backend->out, "    ; ERROR: Expresión nodo tipo %d no soportado\n", expr->type);
        break;
    }
}

/* ==========================================================
   generateStatement
   Genera código para sentencias usando el backend.
   ========================================================== */
static void generateStatement(AstNode *stmt) {
    fprintf(g_backend->out, "    ; ---- Inicio Sentencia ----\n");
    if (!stmt) {
        fprintf(g_backend->out, "    ; (sentencia nula)\n");
        fprintf(g_backend->out, "    ; ---- Fin Sentencia ----\n\n");
        return;
    }
    switch (stmt->type) {
    case AST_VAR_ASSIGN: {
        if (!isSymbolInTable(stmt->varAssign.name))
            addSymbol(stmt->varAssign.name);
        generateExpression(stmt->varAssign.initializer);
        g_backend->emitStoreGlobal(stmt->varAssign.name);
        break;
    }
    case AST_VAR_DECL: {
        if (!isSymbolInTable(stmt->varDecl.name))
            addSymbol(stmt->varDecl.name);
        if (stmt->varDecl.initializer) {
            generateExpression(stmt->varDecl.initializer);
            fprintf(g_backend->out, "    mov [%s], rax ; inicializar varDecl\n", stmt->varDecl.name);
        } else {
            fprintf(g_backend->out, "    ; varDecl '%s' sin init => 0\n", stmt->varDecl.name);
        }
        break;
    }
    case AST_PRINT_STMT: {
        generateExpression(stmt->printStmt.expr);
        fprintf(g_backend->out, "    mov rsi, rax\n");
        fprintf(g_backend->out, "    lea rdi, [rip+fmt]\n");
        fprintf(g_backend->out, "    xor eax, eax\n");
        fprintf(g_backend->out, "    call printf\n");
        break;
    }
    case AST_FUNC_DEF: {
        fprintf(g_backend->out, "\n.global %s\n%s:\n", stmt->funcDef.name, stmt->funcDef.name);
        for (int i = 0; i < stmt->funcDef.bodyCount; i++) {
            generateStatement(stmt->funcDef.body[i]);
        }
        fprintf(g_backend->out, "    ret    ; fin de función %s\n", stmt->funcDef.name);
        break;
    }
    case AST_RETURN_STMT: {
        generateExpression(stmt->returnStmt.expr);
        fprintf(g_backend->out, "    ret\n");
        break;
    }
    case AST_IF_STMT: {
        char labelElse[32], labelEnd[32];
        getNewLabel(labelElse, "ELSE");
        getNewLabel(labelEnd, "ENDIF");
        generateExpression(stmt->ifStmt.condition);
        fprintf(g_backend->out, "    cmp rax, 0\n");
        g_backend->emitJumpIfZero(labelElse);
        for (int i = 0; i < stmt->ifStmt.thenCount; i++) {
            generateStatement(stmt->ifStmt.thenBranch[i]);
        }
        g_backend->emitJump(labelEnd);
        fprintf(g_backend->out, "%s:\n", labelElse);
        for (int i = 0; i < stmt->ifStmt.elseCount; i++) {
            generateStatement(stmt->ifStmt.elseBranch[i]);
        }
        fprintf(g_backend->out, "%s:\n", labelEnd);
        break;
    }
    case AST_FOR_STMT: {
        if (!isSymbolInTable(stmt->forStmt.iterator))
            addSymbol(stmt->forStmt.iterator);
        char labelLoop[32], labelEnd[32];
        getNewLabel(labelLoop, "LOOP");
        getNewLabel(labelEnd, "LOOPEND");
        generateExpression(stmt->forStmt.rangeStart);
        fprintf(g_backend->out, "    mov [%s], rax\n", stmt->forStmt.iterator);
        fprintf(g_backend->out, "%s:\n", labelLoop);
        generateExpression(stmt->forStmt.rangeEnd);
        fprintf(g_backend->out, "    mov rbx, rax    ; rbx = end\n");
        fprintf(g_backend->out, "    mov rax, [%s]   ; rax = i\n", stmt->forStmt.iterator);
        fprintf(g_backend->out, "    cmp rax, rbx\n");
        g_backend->emitJumpIfZero(labelEnd);
        for (int i = 0; i < stmt->forStmt.bodyCount; i++) {
            generateStatement(stmt->forStmt.body[i]);
        }
        fprintf(g_backend->out, "    mov rax, [%s]\n", stmt->forStmt.iterator);
        fprintf(g_backend->out, "    add rax, 1\n");
        fprintf(g_backend->out, "    mov [%s], rax\n", stmt->forStmt.iterator);
        g_backend->emitJump(labelLoop);
        fprintf(g_backend->out, "%s:\n", labelEnd);
        break;
    }
    case AST_IMPORT: {
        fprintf(g_backend->out, "    ; (import %s %s) => sin efecto real\n",
                stmt->importStmt.moduleType, stmt->importStmt.moduleName);
        break;
    }
    case AST_CLASS_DEF: {
        fprintf(g_backend->out, "    ; (class %s) => pendiente de implementación real\n", stmt->classDef.name);
        break;
    }
    case AST_LAMBDA: {
        fprintf(g_backend->out, "    ; (lambda) => pendiente de implementación real\n");
        break;
    }
    case AST_ARRAY_LITERAL: {
        fprintf(g_backend->out, "    ; (array literal) => pendiente de implementación\n");
        break;
    }
    default:
        generateExpression(stmt);
        break;
    }
    fprintf(g_backend->out, "    ; ---- Fin Sentencia ----\n\n");
}

/* ==========================================================
   generateCode
   Función principal para generar el ensamblador final.
   ========================================================== */
void generateCode(AstNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error al abrir archivo de salida.\n");
        exit(1);
    }
    /* Registrar variables globales */
    if (root->type == AST_PROGRAM) {
        for (int i = 0; i < root->program.statementCount; i++) {
            AstNode *st = root->program.statements[i];
            if (st->type == AST_VAR_ASSIGN) {
                if (!isSymbolInTable(st->varAssign.name))
                    addSymbol(st->varAssign.name);
            } else if (st->type == AST_VAR_DECL) {
                if (!isSymbolInTable(st->varDecl.name))
                    addSymbol(st->varDecl.name);
            }
        }
    }
    /* Sección .data */
    fprintf(fp, ".intel_syntax noprefix\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "fmt: .asciz \"Result: %%ld\\n\"\n\n");
    Symbol *sym = symbolTable;
    while (sym) {
        fprintf(fp, "%s: .quad 0\n", sym->name);
        sym = sym->next;
    }
    /* Sección .text */
    fprintf(fp, "\n.text\n.global main\n");
    /* Aseguramos que la salida del backend sea el mismo archivo */
    g_backend->out = fp;
    if (root->type == AST_PROGRAM) {
        fprintf(fp, "main:\n");
        for (int i = 0; i < root->program.statementCount; i++) {
            generateStatement(root->program.statements[i]);
        }
        /* Exit syscall */
        fprintf(g_backend->out, "    mov rax, 60    ; exit\n");
        fprintf(g_backend->out, "    xor rdi, rdi   ; status=0\n");
        fprintf(g_backend->out, "    syscall\n");
    } else {
        fprintf(fp, "main:\n");
        generateStatement(root);
        fprintf(g_backend->out, "    mov rax, 60\n");
        fprintf(g_backend->out, "    xor rdi, rdi\n");
        fprintf(g_backend->out, "    syscall\n");
    }
    fclose(fp);
    freeSymbolTable();
}
