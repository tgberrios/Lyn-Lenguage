#include <stdio.h>
#include <assert.h>
#include "lexer.h"

int main(void) {
    const char *testSource = "func test() { print \"Hello\"; }";
    lexerInit(testSource);
    Token token;
    do {
        token = getNextToken();
        printf("Token: type=%d, lexeme='%s', line=%d, col=%d\n",
               token.type, token.lexeme, token.line, token.col);
    } while (token.type != TOKEN_EOF);
    return 0;
}
