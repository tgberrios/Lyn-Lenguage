#include "arch.h"
#include <stdio.h>
#include <stdlib.h>

/*
   Backend para ARM (ARM32).
   Convenciones (simplificadas):
   - Registro principal: r0.
   - Para acceder a variables globales se utiliza r1 para cargar la dirección.
   - Para operaciones binarias se asume que el operando izquierdo (L) se coloca en r1
     y el operando derecho (R) en r0.
*/

/* Cargar inmediato en r0 */
static void arm_loadImmInt(long value) {
    fprintf(g_backend->out, "    mov r0, #%ld\n", value);
}

/* Almacenar el contenido de r0 en la variable global 'name' */
static void arm_storeGlobal(const char *name) {
    fprintf(g_backend->out, "    ldr r1, =%s\n", name);
    fprintf(g_backend->out, "    str r0, [r1]\n");
}

/* Cargar el contenido de la variable global 'name' en r0 */
static void arm_loadGlobal(const char *name) {
    fprintf(g_backend->out, "    ldr r1, =%s\n", name);
    fprintf(g_backend->out, "    ldr r0, [r1]\n");
}

/* Comparación mayor: asume L en r1, R en r0.
   Se emite una secuencia que coloca en r0 el resultado (1 si L > R, 0 en caso contrario).
*/
static void arm_cmpGreater(void) {
    fprintf(g_backend->out, "    cmp r1, r0\n");
    fprintf(g_backend->out, "    movgt r0, #1\n");
    fprintf(g_backend->out, "    movle r0, #0\n");
}

/* Emitir etiqueta */
static void arm_setLabel(const char *label) {
    fprintf(g_backend->out, "%s:\n", label);
}

/* Salto incondicional a 'label' */
static void arm_jump(const char *label) {
    fprintf(g_backend->out, "    b %s\n", label);
}

/* Salto si r0 es 0 a 'label' */
static void arm_jumpIfZero(const char *label) {
    fprintf(g_backend->out, "    cmp r0, #0\n");
    fprintf(g_backend->out, "    beq %s\n", label);
}

/* --- Operaciones aritméticas --- */

/* Suma: asume L en r1 y R en r0; resultado en r0 */
static void arm_emitAdd(void) {
    fprintf(g_backend->out, "    add r0, r1, r0    ; r0 = L + R\n");
}

/* Resta: asume L en r1 y R en r0; resultado en r0 */
static void arm_emitSub(void) {
    fprintf(g_backend->out, "    sub r0, r1, r0    ; r0 = L - R\n");
}

/* Multiplicación: asume L en r1 y R en r0; resultado en r0 */
static void arm_emitImul(void) {
    fprintf(g_backend->out, "    mul r0, r1, r0    ; r0 = L * R\n");
}

/* División entera: asume L en r1 y R en r0; resultado en r0 */
static void arm_emitIDiv(void) {
    /* Se utiliza la instrucción sdiv para división con signo */
    fprintf(g_backend->out, "    sdiv r0, r1, r0   ; r0 = L / R\n");
}

/* Instancia de la vtable para ARM */
static ArchBackend g_armBackend = {
    .out = NULL,
    .emitLoadImmInt = arm_loadImmInt,
    .emitStoreGlobal = arm_storeGlobal,
    .emitLoadGlobal = arm_loadGlobal,
    .emitCmpGreater = arm_cmpGreater,
    .emitSetLabel = arm_setLabel,
    .emitJump = arm_jump,
    .emitJumpIfZero = arm_jumpIfZero,
    .emitAdd = arm_emitAdd,
    .emitSub = arm_emitSub,
    .emitImul = arm_emitImul,
    .emitIDiv = arm_emitIDiv
};

/* Función para crear el backend ARM.
   Se asigna la salida (por ejemplo, stdout o un FILE* a un archivo .s)
   y se retorna un puntero a la vtable.
*/
ArchBackend *createARMBackend(FILE *fp) {
    g_armBackend.out = fp;
    return &g_armBackend;
}
