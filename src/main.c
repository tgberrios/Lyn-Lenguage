#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "codegen.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    const char *sourceCode =
        "main;\n"
        "    x = 10;\n"
        "    y = 20;\n"
        "\n"
        "    func suma(a int, b int) -> int;\n"
        "        return a + b;\n"
        "    end;\n"
        "\n"
        "    resultado = suma(x, y);\n"
        "    print(\"Resultado: \" + resultado.to_str());\n"
        "\n"
        "    doble = (x int, y int) -> int => x + y;\n"
        "    log(\"Doble de 5 y 10 es: \" + doble(5, 10).to_str());\n"
        "\n"
        "    class Punto;\n"
        "        x float;\n"
        "        y float;\n"
        "\n"
        "        func __init__(self, x float, y float);\n"
        "            self.x = x;\n"
        "            self.y = y;\n"
        "        end;\n"
        "\n"
        "        func distancia(self) -> float;\n"
        "            return sqrt(self.x * self.x + self.y * self.y);\n"
        "        end;\n"
        "    end;\n"
        "\n"
        "    p = Punto(3.0, 4.0);\n"
        "    print(\"Distancia de punto: \" + p.distancia().to_str());\n"
        "\n"
        "    if resultado > 25;\n"
        "        print(\"Resultado mayor a 25\");\n"
        "    else;\n"
        "        print(\"Resultado menor o igual a 25\");\n"
        "    end;\n"
        "\n"
        "    for i in range(3);\n"
        "        print(\"Iteración \" + i.to_str());\n"
        "    end;\n"
        "\n"
        "    ui \"index.html\";\n"
        "    css \"styles.css\";\n"
        "\n"
        "    func onButtonClick();\n"
        "        print(\"Botón presionado\");\n"
        "    end;\n"
        "\n"
        "    register_event(\"btnSubmit\", \"click\", onButtonClick);\n"
        "\n"
        "    import python \"numpy\";\n"
        "\n"
        "    arr = [1, 2, 3, 4];\n"
        "    print(\"Suma de arreglo: \" + suma_numpy(arr).to_str());\n"
        "end;\n";

    // Iniciar lexer
    lexerInit(sourceCode);

    // Debug: Imprimir todos los tokens
    printf("Tokenizing source code:\n");
    Token token;
    do {
        token = getNextToken();
        printf("Token: type=%d, lexeme='%s', line=%d, col=%d\n",
               token.type, token.lexeme, token.line, token.col);
    } while (token.type != TOKEN_EOF);
    lexerInit(sourceCode); // Reiniciar lexer para parsing

    // Parsear
    AstNode *ast = parseProgram();
    if (!ast) {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }
    // Análisis semántico
    analyzeSemantics(ast);
    printf("Semantic analysis completed successfully.\n");
    // Generación de código
    generateCode(ast, "output.s");
    printf("Code generation completed successfully. Assembly written to output.s\n");
    // Liberar AST
    freeAst(ast);
    return 0;
}