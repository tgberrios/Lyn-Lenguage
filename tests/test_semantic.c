#include <stdio.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"

int main(void) {
    // Caso válido: variable declarada y luego usada.
    const char *validSource = 
        "x: int = 10\n"
        "print x\n";
        
    lexerInit(validSource);
    AstNode *ast = parseProgram();
    assert(ast != NULL);
    
    // Debería pasar el análisis semántico sin errores.
    analyzeSemantics(ast);
    printf("Semantic analysis test passed for valid input.\n");
    freeAst(ast);
    
    /*
    // Caso inválido (descomenta para probar):
    // Uso de variable no declarada. Este test debería generar un error semántico y terminar el programa.
    const char *invalidSource = "print y\n";
    lexerInit(invalidSource);
    ast = parseProgram();
    // Se espera que analyzeSemantics detecte el error y llame a exit(1).
    analyzeSemantics(ast);
    freeAst(ast);
    */
    
    return 0;
}
