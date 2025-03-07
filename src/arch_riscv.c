#include "arch.h"
#include <stdio.h>
#include <stdlib.h>

/* 
   Backend para RISC-V.
   Convenciones (simplificadas):
   - "Registro principal" es a0.
   - Se usarán t0 y t1 como registros temporales para operaciones binop.
*/

/* Cargar inmediato en a0 */
static void riscv_loadImmInt(long value) {
    /* Suponemos que el valor cabe en 32 bits */
    fprintf(g_backend->out, "    li a0, %ld\n", value);
}

/* Almacenar el contenido de a0 en la variable global 'name'.
   Se asume que 'name' es la etiqueta de una variable global ya definida.
*/
static void riscv_storeGlobal(const char *name) {
    fprintf(g_backend->out, "    la t0, %s\n", name);
    fprintf(g_backend->out, "    sw a0, 0(t0)\n");
}

/* Cargar el contenido de la variable global 'name' en a0 */
static void riscv_loadGlobal(const char *name) {
    fprintf(g_backend->out, "    la t0, %s\n", name);
    fprintf(g_backend->out, "    lw a0, 0(t0)\n");
}

/* Comparación mayor: Se asume que la operando izquierda (L) está en t1 y la derecha (R) en a0.
   Se utiliza la instrucción sgt para obtener 1 si (t1 > a0), 0 en caso contrario.
*/
static void riscv_cmpGreater(void) {
    fprintf(g_backend->out, "    sgt a0, t1, a0    ; a0 = (t1 > a0)? 1 : 0\n");
}

/* Emitir una etiqueta */
static void riscv_setLabel(const char *label) {
    fprintf(g_backend->out, "%s:\n", label);
}

/* Salto incondicional a 'label' */
static void riscv_jump(const char *label) {
    fprintf(g_backend->out, "    j %s\n", label);
}

/* Salto condicional: si a0 es 0, salta a 'label' */
static void riscv_jumpIfZero(const char *label) {
    fprintf(g_backend->out, "    beqz a0, %s\n", label);
}

/* --- Operaciones aritméticas --- */

/* Suma: se asume que L ya quedó en t0 y R en a0, luego:
   a0 = t0 + a0 */
static void riscv_emitAdd(void) {
    fprintf(g_backend->out, "    add a0, t0, a0    ; a0 = L + R\n");
}

/* Resta: se asume L en t0, R en a0; se calcula L - R */
static void riscv_emitSub(void) {
    fprintf(g_backend->out, "    sub a0, t0, a0    ; a0 = L - R\n");
}

/* Multiplicación: se asume L en t0, R en a0; se calcula L * R */
static void riscv_emitImul(void) {
    fprintf(g_backend->out, "    mul a0, t0, a0    ; a0 = L * R\n");
}

/* División entera: se asume L en t0, R en a0; se calcula L / R.
   La instrucción div de RISC-V realiza división entera.
*/
static void riscv_emitIDiv(void) {
    fprintf(g_backend->out, "    div a0, t0, a0    ; a0 = L / R\n");
}

/* Instanciamos la vtable para RISC-V */
static ArchBackend g_riscvBackend = {
    .out = NULL,
    .emitLoadImmInt = riscv_loadImmInt,
    .emitStoreGlobal = riscv_storeGlobal,
    .emitLoadGlobal = riscv_loadGlobal,
    .emitCmpGreater = riscv_cmpGreater,
    .emitSetLabel = riscv_setLabel,
    .emitJump = riscv_jump,
    .emitJumpIfZero = riscv_jumpIfZero,
    .emitAdd = riscv_emitAdd,
    .emitSub = riscv_emitSub,
    .emitImul = riscv_emitImul,
    .emitIDiv = riscv_emitIDiv
};

/* Función para crear el backend RISC-V.
   Se configura la salida (por ejemplo, stdout o un FILE* a un archivo .s).
*/
ArchBackend *createRiscvBackend(FILE *fp) {
    g_riscvBackend.out = fp;
    return &g_riscvBackend;
}
