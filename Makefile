# Define el target de la arquitectura. Por defecto se compila para x86_64.
TARGET ?= x86_64

# Compilador por defecto (x86_64)
CC = gcc

# Flags generales
CFLAGS += -Wall -Wextra -std=c11 -I./src -DDEBUG_MEMORY
LDFLAGS +=

# Configuración específica para cada target
ifeq ($(TARGET),arm)
    # Supone que tienes instalado un toolchain de ARM (por ejemplo, arm-none-eabi-gcc)
    CC = arm-none-eabi-gcc
    CFLAGS += -mcpu=cortex-m3 -mthumb
    LDFLAGS += -T linker_arm.ld  # Si tienes un script de linker para ARM
endif

ifeq ($(TARGET),riscv)
    # Toolchain para RISCV (ajusta el nombre según el instalado en tu sistema)
    CC = riscv64-unknown-elf-gcc
    CFLAGS += -march=rv32imac -mabi=ilp32
    LDFLAGS += -T linker_riscv.ld  # Script de linker para RISCV
endif

ifeq ($(TARGET),wasm)
    # Para WebAssembly, se puede usar clang con target wasm32
    CC = clang
    CFLAGS += --target=wasm32
    # LDFLAGS se ajusta según cómo se desee generar el binario WASM
endif

# Lista de archivos objeto
OBJS = src/main.o src/lexer.o src/parser.o src/ast.o src/semantic.o src/optimize.o src/codegen.o src/memory.o src/arch_select.o src/arch_x86_64.o src/arch_arm.o src/arch_riscv.o src/arch_wasm.o

# Regla principal
all: compiler

compiler: $(OBJS)
	$(CC) $(CFLAGS) -o compiler $(OBJS) $(LDFLAGS)

# Regla para compilar cada archivo fuente
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) compiler
