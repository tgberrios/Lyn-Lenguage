/* arch.h */
#ifndef ARCH_H
#define ARCH_H

#include <stdio.h>

typedef enum {
    ARCH_X86_64,
    ARCH_ARM32,
    ARCH_RISCV64,
    ARCH_WASM,
    ARCH_UNKNOWN
} Architecture;

typedef struct {
    FILE *out;
    void (*emitLoadImmInt)(long value);
    void (*emitStoreGlobal)(const char *name);
    void (*emitLoadGlobal)(const char *name);
    /* Nuevos campos para operaciones aritméticas */
    void (*emitAdd)(void);
    void (*emitSub)(void);
    void (*emitImul)(void);
    void (*emitIDiv)(void);
    void (*emitCmpGreater)(void);
    /* Saltos y etiquetas */
    void (*emitJumpIfZero)(const char *label);
    void (*emitJump)(const char *label);
    void (*emitSetLabel)(const char *label);
} ArchBackend;

extern ArchBackend *g_backend;
void setCurrentBackend(Architecture arch, FILE *outputFile);

#endif /* ARCH_H */
