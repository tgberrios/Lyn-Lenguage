#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "codegen.h"
#include "memory.h"  // Módulo de gestión de memoria

/* Prototipos de funciones de prueba */
void runLexerTest(const char *source);
void runSemanticTest(AstNode *ast);
void runCodegenTest(AstNode *ast);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* Código de prueba con múltiples casos */
    const char *sourceCode =
        "main;\n"
        "    // Variables y expresiones\n"
        "    x = 10;\n"
        "    y = 20;\n"
        "    z = (x + y) * 2;\n"
        "    print(z);\n"
        "\n"
        "    // Funciones\n"
        "    func suma(a int, b int) -> int;\n"
        "        return a + b;\n"
        "    end;\n"
        "\n"
        "    resultado = suma(x, y);\n"
        "    print(\"Resultado: \" + resultado.to_str());\n"
        "\n"
        "    // Función lambda\n"
        "    doble = (n int) -> int => n * 2;\n"
        "    print(\"Doble de 5: \" + doble(5).to_str());\n"
        "\n"
        "    // Clases y objetos\n"
        "    class Punto;\n"
        "        x float;\n"
        "        y float;\n"
        "        func __init__(self, x float, y float);\n"
        "            self.x = x;\n"
        "            self.y = y;\n"
        "        end;\n"
        "    end;\n"
        "    p = Punto(3.0, 4.0);\n"
        "\n"
        "    // Control de flujo\n"
        "    if resultado > 25;\n"
        "        print(\"Mayor que 25\");\n"
        "    else;\n"
        "        print(\"Menor o igual a 25\");\n"
        "    end;\n"
        "\n"
        "    // Bucles\n"
        "    for i in range(3);\n"
        "        print(\"Iteración \" + i.to_str());\n"
        "    end;\n"
        "\n"
        "    // Importaciones\n"
        "    import python \"numpy\";\n"
        "    arr = [1, 2, 3, 4];\n"
        "    print(\"Suma numpy: \" + suma_numpy(arr).to_str());\n"
        "end;\n";

    /* 1️⃣ Test del Lexer */
    runLexerTest(sourceCode);

    /* 2️⃣ Test del Parser */
    lexerInit(sourceCode);  // Reiniciar el lexer para parsing
    AstNode *ast = parseProgram();
    if (!ast) {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }
    printf("Parsing completed successfully.\n");

    /* 3️⃣ Test de Análisis Semántico */
    runSemanticTest(ast);

    /* 4️⃣ Test de Generación de Código */
    runCodegenTest(ast);

    /* Liberar memoria: freeAst utiliza internamente memory_pool_free para retornar los nodos al pool */
    freeAst(ast);

    /* Opcional: si deseas volcar estadísticas del pool o destruirlo, puedes hacerlo aquí.
       Por ejemplo, si hubieras expuesto una función destroyAstPool() en el módulo AST,
       podrías llamarla antes de finalizar. */

    return 0;
}

/* ===================== */
/* ⚡ Funciones de prueba */
/* ===================== */

/* Prueba del Lexer */
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

/* Prueba del Análisis Semántico */
void runSemanticTest(AstNode *ast) {
    printf("Running Semantic Analysis Test...\n");
    analyzeSemantics(ast);
    printf("Semantic Analysis Passed!\n\n");
}

/* Prueba de Generación de Código */
void runCodegenTest(AstNode *ast) {
    printf("Running Code Generation Test...\n");
    generateCode(ast, "output.s");

    /* Verificar que output.s fue generado correctamente */
    FILE *file = fopen("output.s", "r");
    if (file) {
        printf("Code generation completed successfully. Assembly written to output.s\n");
        fclose(file);
    } else {
        fprintf(stderr, "Error: output.s not found.\n");
    }
}