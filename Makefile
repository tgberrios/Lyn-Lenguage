CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src
SRC = src/main.c src/lexer.c src/parser.c src/ast.c src/semantic.c src/codegen.c src/memory.c
OBJ = $(SRC:.c=.o)
TARGET = compiler

# Enlazamos pthread para usar mutex en memory.c
LDLIBS = -lpthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
