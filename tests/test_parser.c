#include <stdio.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

int main(void) {
    const char *sourceCode = "x: int = 42\nprint x\n";
    lexerInit(sourceCode);
    AstNode *ast = parseProgram();
    assert(ast != NULL);
    printf("Parser test passed.\n");
    freeAst(ast);
    return 0;
}
