#!/bin/bash
# Script para compilar todos los targets: por defecto, ARM, RISCV y WASM

# Opciones comunes de compilación
CFLAGS_COMMON="-Wall -Wextra -std=c11 -I./src -DDEBUG_MEMORY -DUSE_GC"

echo "Compilación del target por defecto..."
make clean && make CFLAGS="$CFLAGS_COMMON" && ./compiler

# Compilación para ARM
if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "Compilación para ARM..."
    make clean && make TARGET=arm CFLAGS="$CFLAGS_COMMON" && ./compiler
else
    echo "Herramienta arm-none-eabi-gcc no encontrada. Se omitirá la compilación para ARM."
fi

# Compilación para RISCV
if command -v riscv64-unknown-elf-gcc >/dev/null 2>&1; then
    echo "Compilación para RISCV..."
    make clean && make TARGET=riscv CFLAGS="$CFLAGS_COMMON" && ./compiler
else
    echo "Herramienta riscv64-unknown-elf-gcc no encontrada. Se omitirá la compilación para RISCV."
fi

# Compilación para WASM
if command -v wasmtime >/dev/null 2>&1; then
    echo "Compilación para WASM..."
    make clean && make TARGET=wasm CFLAGS="$CFLAGS_COMMON"
    # Para WASM, el resultado es un módulo .wasm; se puede ejecutar con un runtime como wasmtime.
    echo "Ejecutando módulo WASM con wasmtime..."
    wasmtime compiler.wasm
else
    echo "Herramienta WASM (ej. wasmtime) no encontrada. Se omitirá la ejecución del módulo WASM."
fi

echo "Proceso de compilación completado."
