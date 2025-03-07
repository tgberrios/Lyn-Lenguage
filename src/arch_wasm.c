#include "arch.h"
#include <stdio.h>
#include <stdlib.h>

/*
   Backend para WebAssembly (WAT textual).
   Este backend simula operaciones aritméticas y de control de flujo
   en WebAssembly usando instrucciones básicas.
   Se asume que los enteros son de 32 bits.
*/

/* Cargar inmediato en la pila: push i32.const value */
static void wasm_loadImmInt(long value) {
    fprintf(g_backend->out, "    i32.const %ld\n", value);
}

/* Almacenar el valor tope de la pila en la variable global '$name' */
static void wasm_storeGlobal(const char *name) {
    fprintf(g_backend->out, "    global.set $%s\n", name);
}

/* Cargar el valor de la variable global '$name' y ponerlo en la pila */
static void wasm_loadGlobal(const char *name) {
    fprintf(g_backend->out, "    global.get $%s\n", name);
}

/* Comparación mayor: en WASM se usa i32.gt_s para enteros con signo.
   Se asume que los dos operandos ya están en la pila.
   i32.gt_s consumirá los dos operandos y empujará 1 si L > R, 0 en caso contrario.
*/
static void wasm_cmpGreater(void) {
    fprintf(g_backend->out, "    i32.gt_s\n");
}

/* Emite una "etiqueta" como comentario */
static void wasm_setLabel(const char *label) {
    fprintf(g_backend->out, "    ;; label %s\n", label);
}

/* Salto incondicional: en WASM se utiliza 'br' para saltar a un bloque etiquetado */
static void wasm_jump(const char *label) {
    fprintf(g_backend->out, "    br %s\n", label);
}

/* Salto condicional: usa 'i32.eqz' para comparar con cero y 'br_if' para saltar si es cierto */
static void wasm_jumpIfZero(const char *label) {
    fprintf(g_backend->out, "    i32.eqz\n");
    fprintf(g_backend->out, "    br_if %s\n", label);
}

/* --- Operaciones aritméticas --- */

/* Suma: i32.add */
static void wasm_emitAdd(void) {
    fprintf(g_backend->out, "    i32.add\n");
}

/* Resta: i32.sub */
static void wasm_emitSub(void) {
    fprintf(g_backend->out, "    i32.sub\n");
}

/* Multiplicación: i32.mul */
static void wasm_emitImul(void) {
    fprintf(g_backend->out, "    i32.mul\n");
}

/* División entera con signo: i32.div_s */
static void wasm_emitIDiv(void) {
    fprintf(g_backend->out, "    i32.div_s\n");
}

/* Instanciamos la vtable para WebAssembly */
static ArchBackend g_wasmBackend = {
    .out = NULL,
    .emitLoadImmInt = wasm_loadImmInt,
    .emitStoreGlobal = wasm_storeGlobal,
    .emitLoadGlobal = wasm_loadGlobal,
    .emitCmpGreater = wasm_cmpGreater,
    .emitSetLabel = wasm_setLabel,
    .emitJump = wasm_jump,
    .emitJumpIfZero = wasm_jumpIfZero,
    .emitAdd = wasm_emitAdd,
    .emitSub = wasm_emitSub,
    .emitImul = wasm_emitImul,
    .emitIDiv = wasm_emitIDiv
};

/* Función para crear el backend WebAssembly. Se configura la salida (FILE *) */
ArchBackend *createWasmBackend(FILE *fp) {
    g_wasmBackend.out = fp;
    return &g_wasmBackend;
}
