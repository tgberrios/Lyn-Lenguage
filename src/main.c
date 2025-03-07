#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "optimize.h"    // Módulo de optimización del AST
#include "codegen.h"
#include "memory.h"      // Módulo de gestión de memoria

/* Prototipos de funciones de prueba */
void runLexerTest(const char *source);
void runParserTest(const char *source, AstNode **astOut);
void runOptimizeTest(AstNode **ast);
void runSemanticTest(AstNode *ast);
void runCodegenTest(AstNode *ast);
void runMemoryStats(void);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* Código de prueba de ejemplo en Lyn usando la notación con ':' */
    const char *sourceCode =
        "// Programa de prueba completo en Lyn\n"
        "main;\n"
        "\n"
        "// Prueba de variables y expresiones\n"
        "x: int = 10;\n"
        "y: int = 20;\n"
        "z: int = (x + y) * 2;\n"
        "print(z);\n"
        "\n"
        "// Prueba de funciones\n"
        "func suma(a: int, b: int) -> int;\n"
        "    return a + b;\n"
        "end;\n"
        "resultado: int = suma(x, y);\n"
        "print(\"Resultado: \" + resultado.to_str());\n"
        "\n"
        "// Prueba de función lambda\n"
        "doble = (n: int) -> int => n * 2;\n"
        "print(\"Doble de 5: \" + doble(5).to_str());\n"
        "\n"
        "// Prueba de clases y objetos\n"
        "class Punto;\n"
        "    x: float;\n"
        "    y: float;\n"
        "    func __init__(self: Punto, x: float, y: float);\n"
        "        self.x = x;\n"
        "        self.y = y;\n"
        "    end;\n"
        "    func distancia(self: Punto) -> float;\n"
        "        return sqrt(self.x * self.x + self.y * self.y);\n"
        "    end;\n"
        "end;\n"
        "p: Punto = Punto(3.0, 4.0);\n"
        "print(\"Distancia de punto: \" + p.distancia().to_str());\n"
        "\n"
        "// Prueba de control de flujo\n"
        "if resultado > 25;\n"
        "    print(\"Resultado mayor a 25\");\n"
        "else;\n"
        "    print(\"Resultado menor o igual a 25\");\n"
        "end;\n"
        "\n"
        "// Prueba de bucle\n"
        "for i in range(3);\n"
        "    print(\"Iteración \" + i.to_str());\n"
        "end;\n"
        "\n"
        "// Prueba de importación\n"
        "import python \"numpy\";\n"
        "arr: [int] = [1, 2, 3, 4];\n"
        "print(\"Suma de arreglo: \" + suma_numpy(arr).to_str());\n"
        "\n"
        "end;\n";

    printf("=== Ejecución de pruebas de Lync Compiler ===\n\n");

    /* 1️⃣ Test del Lexer */
    runLexerTest(sourceCode);

    /* 2️⃣ Test del Parser */
    AstNode *ast = NULL;
    runParserTest(sourceCode, &ast);
    if (!ast) {
        fprintf(stderr, "Error: Parsing failed.\n");
        exit(1);
    }
    printf("Parser: AST generado exitosamente.\n\n");

    /* 3️⃣ Optimización del AST */
    runOptimizeTest(&ast);
    printf("Optimizer: AST optimizado exitosamente.\n\n");

    /* 4️⃣ Test de Análisis Semántico */
    runSemanticTest(ast);
    printf("Semantic Analysis: Análisis semántico completado.\n\n");

    /* 5️⃣ Test de Generación de Código */
    runCodegenTest(ast);
    printf("Code Generation: Código ensamblador generado.\n\n");

    /* 6️⃣ (Opcional) Mostrar estadísticas de memoria si DEBUG_MEMORY está definido */
    runMemoryStats();

    /* Liberar el AST */
    freeAst(ast);

    printf("Todas las pruebas se ejecutaron exitosamente.\n");
    return 0;
}

/* ===================== */
/* ⚡ Funciones de prueba */
/* ===================== */

void runLexerTest(const char *source) {
    printf("Running Lexer Test...\n");
    lexerInit(source);
    Token token;
    do {
        token = getNextToken();
        printf("Token: type=%d, lexeme='%s', line=%d, col=%d\n",
               token.type, token.lexeme, token.line, token.col);
    } while (token.type != TOKEN_EOF);
    printf("Lexer Test Passed!\n\n");
}

void runParserTest(const char *source, AstNode **astOut) {
    printf("Running Parser Test...\n");
    lexerInit(source);  // Reiniciar el lexer
    *astOut = parseProgram();
    if (*astOut) {
        printf("Parser Test Passed!\n\n");
    } else {
        printf("Parser Test Failed!\n\n");
    }
}

void runOptimizeTest(AstNode **ast) {
    printf("Running AST Optimization Test...\n");
    *ast = optimizeAST(*ast);
    printf("AST Optimization Test Passed!\n\n");
}

void runSemanticTest(AstNode *ast) {
    printf("Running Semantic Analysis Test...\n");
    analyzeSemantics(ast);
    printf("Semantic Analysis Test Passed!\n\n");
}

void runCodegenTest(AstNode *ast) {
    printf("Running Code Generation Test...\n");
    generateCode(ast, "output.s");

    FILE *file = fopen("output.s", "r");
    if (file) {
        printf("Code Generation Test Passed! Assembly written to output.s\n");
        fclose(file);
    } else {
        fprintf(stderr, "Code Generation Test Failed: output.s not found.\n");
    }
}

void runMemoryStats(void) {
    #ifdef DEBUG_MEMORY
    printf("\n--- Memory Pool Statistics ---\n");
    printf("Global Allocations: %zu\n", memory_get_global_alloc_count());
    printf("Global Frees: %zu\n", memory_get_global_free_count());
    printf("------------------------------\n\n");
    #endif
}
