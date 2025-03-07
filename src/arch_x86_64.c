#include "arch.h"
#include <stdio.h>
#include <stdlib.h>

/*
   Backend para x86_64.
   Convenciones:
   - "Registro principal" es RAX.
   - Las variables globales se acceden mediante [nombre].
   - Para la comparación “mayor” se asume que la operando izquierda (L) está en RBX y la derecha (R) en RAX.
*/

/* Cargar inmediato en RAX */
static void x86_loadImmInt(long value) {
    fprintf(g_backend->out, "    mov rax, %ld\n", value);
}

/* Almacenar el contenido de RAX en la variable global 'name' */
static void x86_storeGlobal(const char *name) {
    fprintf(g_backend->out, "    mov [%s], rax\n", name);
}

/* Cargar el contenido de la variable global 'name' en RAX */
static void x86_loadGlobal(const char *name) {
    fprintf(g_backend->out, "    mov rax, [%s]\n", name);
}

/* Comparación mayor: asume L en RBX, R en RAX */
static void x86_cmpGreater(void) {
    fprintf(g_backend->out, "    cmp rbx, rax\n");
    fprintf(g_backend->out, "    setg al\n");
    fprintf(g_backend->out, "    movzb rax, al\n");
}

/* Emitir etiqueta */
static void x86_setLabel(const char *label) {
    fprintf(g_backend->out, "%s:\n", label);
}

/* Salto incondicional a 'label' */
static void x86_jump(const char *label) {
    fprintf(g_backend->out, "    jmp %s\n", label);
}

/* Salto si RAX es 0 a 'label' */
static void x86_jumpIfZero(const char *label) {
    fprintf(g_backend->out, "    cmp rax, 0\n");
    fprintf(g_backend->out, "    je %s\n", label);
}

/* --- Operaciones aritméticas --- */

/* Suma: rax = rax + rbx */
static void x86_emitAdd(void) {
    fprintf(g_backend->out, "    add rax, rbx    ; rax = L + R\n");
}

/* Resta: rax = (L - R) (se asume L en rbx, R en rax) */
static void x86_emitSub(void) {
    fprintf(g_backend->out, "    sub rbx, rax    ; rbx = L - R\n");
    fprintf(g_backend->out, "    mov rax, rbx\n");
}

/* Multiplicación: rax = rax * rbx */
static void x86_emitImul(void) {
    fprintf(g_backend->out, "    imul rax, rbx   ; rax = L * R\n");
}

/* División entera: rax = L / R.
   Se mueve R a rcx, L a rax y se hace idiv. */
static void x86_emitIDiv(void) {
    fprintf(g_backend->out, "    mov rcx, rax    ; divisor en rcx (R)\n");
    fprintf(g_backend->out, "    mov rax, rbx    ; dividendo en rax (L)\n");
    fprintf(g_backend->out, "    xor rdx, rdx\n");
    fprintf(g_backend->out, "    idiv rcx        ; rax = L / R\n");
}

/* Instancia de la vtable para x86_64 */
static ArchBackend g_x86_64Backend = {
    .out = NULL,
    .emitLoadImmInt = x86_loadImmInt,
    .emitStoreGlobal = x86_storeGlobal,
    .emitLoadGlobal = x86_loadGlobal,
    .emitCmpGreater = x86_cmpGreater,
    .emitSetLabel = x86_setLabel,
    .emitJump = x86_jump,
    .emitJumpIfZero = x86_jumpIfZero,
    .emitAdd = x86_emitAdd,
    .emitSub = x86_emitSub,
    .emitImul = x86_emitImul,
    .emitIDiv = x86_emitIDiv
};

/* Función para crear el backend x86_64. Se configura la salida (FILE *) */
ArchBackend *createX86Backend(FILE *fp) {
    g_x86_64Backend.out = fp;
    return &g_x86_64Backend;
}
