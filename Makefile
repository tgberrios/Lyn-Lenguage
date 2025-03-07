CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src
SRC = src/main.c src/lexer.c src/parser.c src/ast.c src/semantic.c src/optimize.c \
      src/codegen.c src/memory.c \
      src/arch_select.c src/arch_x86_64.c src/arch_arm.c src/arch_riscv.c src/arch_wasm.c
OBJ = $(SRC:.c=.o)
TARGET = compiler

LDLIBS = -lpthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
