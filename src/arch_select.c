#include "arch.h"
#include <stdio.h>
#include <stdlib.h>

/* Declaración de las funciones creadoras de cada backend */
extern ArchBackend *createX86Backend(FILE *fp);
extern ArchBackend *createARMBackend(FILE *fp);
extern ArchBackend *createRiscvBackend(FILE *fp);
extern ArchBackend *createWasmBackend(FILE *fp);

/* Variable global que será usada por el codegen */
ArchBackend *g_backend = NULL;

/**
 * @brief Selecciona e inicializa el backend según la arquitectura objetivo.
 * 
 * @param arch Arquitectura objetivo (ARCH_X86_64, ARCH_ARM32, ARCH_RISCV64, ARCH_WASM).
 * @param outputFile Archivo de salida (por ejemplo, stdout o un FILE* a un .s).
 */
void setCurrentBackend(Architecture arch, FILE *outputFile) {
    switch (arch) {
        case ARCH_X86_64:
            g_backend = createX86Backend(outputFile);
            break;
        case ARCH_ARM32:
            g_backend = createARMBackend(outputFile);
            break;
        case ARCH_RISCV64:
            g_backend = createRiscvBackend(outputFile);
            break;
        case ARCH_WASM:
            g_backend = createWasmBackend(outputFile);
            break;
        default:
            fprintf(stderr, "Error: Arquitectura no soportada.\n");
            exit(1);
    }
}
