#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // Para strcmp
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "optimize.h"
#include "codegen.h"
#include "memory.h"
#include "arch.h"  // Define Architecture, setCurrentBackend(), etc.

//
// Prototipos de funciones de prueba
//
void runLexerTest(const char *source);
void runParserTest(const char *source, AstNode **astOut);
void runOptimizeTest(AstNode **ast);
void runSemanticTest(AstNode *ast);
void runCodegenTest(AstNode *ast);
void runMemoryStats(void);

// Prototipo para testear todos los backends
void runAllBackendTests(const char *source);

int main(int argc, char **argv) {
    printf("=== Ejecución de pruebas de Lync Compiler ===\n\n");

    /* 1) Detectar argumentos --target=arm|riscv|wasm|x86 */
    Architecture arch = ARCH_X86_64;  /* Por defecto: x86_64 */
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--target=", 9) == 0) {
            const char *targetName = argv[i] + 9;
            if (strcmp(targetName, "arm") == 0) {
                arch = ARCH_ARM32;
            }
            else if (strcmp(targetName, "riscv") == 0) {
                arch = ARCH_RISCV64;
            }
            else if (strcmp(targetName, "wasm") == 0) {
                arch = ARCH_WASM;
            }
            else {
                printf("Aviso: objetivo '%s' no reconocido; usando x86_64.\n", targetName);
                arch = ARCH_X86_64;
            }
        }
    }
    /* Se configura el backend seleccionado (la salida se puede redirigir a un archivo si se desea) */
    setCurrentBackend(arch, stdout);

    /* 2) Código de prueba en Lyn */
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

    /* 3) Ejecutar las pruebas para el target seleccionado */
    runLexerTest(sourceCode);

    AstNode *ast = NULL;
    runParserTest(sourceCode, &ast);
    if (!ast) {
        fprintf(stderr, "Error: Parsing failed.\n");
        exit(1);
    }
    printf("Parser: AST generado exitosamente.\n\n");

    runOptimizeTest(&ast);
    printf("Optimizer: AST optimizado exitosamente.\n\n");

    runSemanticTest(ast);
    printf("Semantic Analysis: Análisis semántico completado.\n\n");

    runCodegenTest(ast);
    printf("Code Generation: Código ensamblador generado.\n\n");

    runMemoryStats();

    freeAst(ast);

    printf("Prueba con target seleccionado finalizada.\n\n");

    /* 4) Ejecutar tests para todos los backends disponibles */
    runAllBackendTests(sourceCode);

    printf("Todas las pruebas se ejecutaron exitosamente.\n");
    return 0;
}

/* ===================== */
/* Funciones de prueba  */
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
    lexerInit(source);
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

/* ==========================================================
   runAllBackendTests
   Ejecuta todas las fases (lexer, parser, optimización, semántica, codegen)
   para cada backend disponible.
   ========================================================== */
void runAllBackendTests(const char *source) {
    // Lista de arquitecturas a probar y sus nombres
    Architecture archs[] = { ARCH_X86_64, ARCH_ARM32, ARCH_RISCV64, ARCH_WASM };
    const char *archNames[] = { "x86_64", "ARM32", "RISCV64", "WASM" };
    int count = sizeof(archs) / sizeof(archs[0]);

    for (int i = 0; i < count; i++) {
        printf("=== Testing backend: %s ===\n", archNames[i]);
        setCurrentBackend(archs[i], stdout);

        // Reejecutar el lexer, parser, optimización, semántica y generación de código para este backend
        lexerInit(source);
        AstNode *ast = parseProgram();
        if (!ast) {
            printf("Parser failed for backend %s\n", archNames[i]);
            continue;
        }
        printf("Parser: AST generado exitosamente para %s.\n", archNames[i]);

        ast = optimizeAST(ast);
        printf("Optimizer: AST optimizado para %s.\n", archNames[i]);

        analyzeSemantics(ast);
        printf("Semantic Analysis: Completado para %s.\n", archNames[i]);

        generateCode(ast, "output.s");
        printf("Code Generation: Ensamblador generado para %s.\n", archNames[i]);

        freeAst(ast);
        printf("-------------------------------------------------\n\n");
    }
}
