#include "parser.h"
#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Variable global para el token actual */
static Token currentToken;

/* Prototipos internos */
static void advanceToken(void);
static void parserError(const char *message);
static int isLambdaLookahead(void);
static AstNode *parsePostfix(AstNode *node);
static AstNode *parseStatement(void);
static AstNode *parseExpression(void);
static AstNode *parseTerm(void);
static AstNode *parseFactor(void);
static AstNode *parseFuncDef(void);
static AstNode *parseReturn(void);
static AstNode *parseIfStmt(void);
static AstNode *parseForStmt(void);
static AstNode *parseClassDef(void);
static AstNode *parseLambda(void);
static AstNode *parseArrayLiteral(void);
static void skipStatementSeparators(void);

/* Avanza al siguiente token */
static void advanceToken(void) {
    currentToken = getNextToken();
    printf("advanceToken: type=%d, lexeme='%s'\n", currentToken.type, currentToken.lexeme);
}

/* Reporta un error de parseo y termina */
static void parserError(const char *message) {
    fprintf(stderr, "\n========== LYN PARSER ERROR ==========\n");
    fprintf(stderr, "Ubicación: Línea %d, Columna %d\n", currentToken.line, currentToken.col);
    fprintf(stderr, "Error: %s\n", message);
    fprintf(stderr, "Token actual: '%s' (Tipo: %d)\n", currentToken.lexeme, currentToken.type);
    fprintf(stderr, "========================================\n\n");
    exit(1);
}

/* isLambdaLookahead: Comprueba sin alterar el estado global
   si la secuencia actual corresponde a la sintaxis de una lambda:
   ( [paramName paramType {, paramName paramType}] ) -> returnType => ... */
static int isLambdaLookahead(void) {
    LexerState saved = lexSaveState();

    // Se asume que currentToken es '('.
    Token tok1 = getNextToken(); // Primer token dentro de los paréntesis.
    if (tok1.type == TOKEN_RPAREN) {
        // Lista vacía: esperamos '->'
        Token tok2 = getNextToken();
        if (tok2.type != TOKEN_ARROW) { lexRestoreState(saved); return 0; }
        Token tok3 = getNextToken();
        if (!(tok3.type == TOKEN_INT || tok3.type == TOKEN_FLOAT ||
             (tok3.type == TOKEN_IDENTIFIER && (strcmp(tok3.lexeme, "int") == 0 || strcmp(tok3.lexeme, "float") == 0)))) {
            lexRestoreState(saved);
            return 0;
        }
        Token tok4 = getNextToken();
        if (tok4.type != TOKEN_FAT_ARROW) { lexRestoreState(saved); return 0; }
        lexRestoreState(saved);
        return 1;
    } else {
        // Debe comenzar con un identificador.
        if (tok1.type != TOKEN_IDENTIFIER) { lexRestoreState(saved); return 0; }
        Token tok2 = getNextToken(); // Tipo
        if (!(tok2.type == TOKEN_INT || tok2.type == TOKEN_FLOAT ||
             (tok2.type == TOKEN_IDENTIFIER && (strcmp(tok2.lexeme, "int") == 0 || strcmp(tok2.lexeme, "float") == 0)))) {
            lexRestoreState(saved);
            return 0;
        }
        // Posible lista separada por comas.
        Token tok = getNextToken();
        while (tok.type == TOKEN_COMMA) {
            Token tokParam = getNextToken();
            if (tokParam.type != TOKEN_IDENTIFIER) { lexRestoreState(saved); return 0; }
            Token tokType = getNextToken();
            if (!(tokType.type == TOKEN_INT || tokType.type == TOKEN_FLOAT ||
                  (tokType.type == TOKEN_IDENTIFIER && (strcmp(tokType.lexeme, "int") == 0 || strcmp(tokType.lexeme, "float") == 0)))) {
                lexRestoreState(saved);
                return 0;
            }
            tok = getNextToken();
        }
        if (tok.type != TOKEN_RPAREN) { lexRestoreState(saved); return 0; }
        Token tokAfterParen = getNextToken();
        if (tokAfterParen.type != TOKEN_ARROW) { lexRestoreState(saved); return 0; }
        Token tokReturnType = getNextToken();
        if (!(tokReturnType.type == TOKEN_INT || tokReturnType.type == TOKEN_FLOAT ||
             (tokReturnType.type == TOKEN_IDENTIFIER && (strcmp(tokReturnType.lexeme, "int") == 0 || strcmp(tokReturnType.lexeme, "float") == 0)))) {
            lexRestoreState(saved);
            return 0;
        }
        Token tokFatArrow = getNextToken();
        if (tokFatArrow.type != TOKEN_FAT_ARROW) { lexRestoreState(saved); return 0; }
        lexRestoreState(saved);
        return 1;
    }
}

/* parsePostfix: Maneja encadenamiento de '.' y '()' */
static AstNode *parsePostfix(AstNode *node) {
    printf("parsePostfix: starting with node type=%d, current: type=%d, lexeme='%s'\n",
           node->type, currentToken.type, currentToken.lexeme);

    if (currentToken.type == TOKEN_DOT) {
        advanceToken(); // consume '.'
        printf("parsePostfix: consumed '.', current type=%d, lexeme='%s'\n",
               currentToken.type, currentToken.lexeme);
        if (currentToken.type != TOKEN_IDENTIFIER)
            parserError("Expected identifier after '.'");
        AstNode *memberNode = createAstNode(AST_MEMBER_ACCESS);
        memberNode->memberAccess.object = node;
        strncpy(memberNode->memberAccess.member, currentToken.lexeme, sizeof(memberNode->memberAccess.member));
        advanceToken(); // consume el identificador
        printf("parsePostfix: consumed identifier '%s', current type=%d, lexeme='%s'\n",
               memberNode->memberAccess.member, currentToken.type, currentToken.lexeme);

        if (currentToken.type == TOKEN_LPAREN) {
            advanceToken(); // consume '('
            printf("parsePostfix: consumed '(', current type=%d, lexeme='%s'\n",
                   currentToken.type, currentToken.lexeme);
            AstNode *funcCall = createAstNode(AST_FUNC_CALL);
            strncpy(funcCall->funcCall.name, memberNode->memberAccess.member, sizeof(funcCall->funcCall.name));
            funcCall->funcCall.arguments = realloc(funcCall->funcCall.arguments, sizeof(AstNode *));
            funcCall->funcCall.arguments[0] = memberNode->memberAccess.object;
            funcCall->funcCall.argCount = 1;
            freeAstNode(memberNode);
            while (currentToken.type != TOKEN_RPAREN) {
                AstNode *arg = parseExpression();
                funcCall->funcCall.argCount++;
                funcCall->funcCall.arguments = realloc(funcCall->funcCall.arguments,
                                                       funcCall->funcCall.argCount * sizeof(AstNode *));
                funcCall->funcCall.arguments[funcCall->funcCall.argCount - 1] = arg;
                printf("parsePostfix: parsed argument, current type=%d, lexeme='%s'\n",
                       currentToken.type, currentToken.lexeme);
                if (currentToken.type == TOKEN_COMMA)
                    advanceToken();
                else if (currentToken.type != TOKEN_RPAREN)
                    parserError("Expected ',' or ')' in argument list");
            }
            advanceToken(); // consume ')'
            printf("parsePostfix: consumed ')', current type=%d, lexeme='%s'\n",
                   currentToken.type, currentToken.lexeme);
            node = funcCall;
        } else {
            node = memberNode;
        }
        return parsePostfix(node);
    } else if (currentToken.type == TOKEN_LPAREN && node->type == AST_IDENTIFIER) {
        advanceToken(); // consume '('
        printf("parsePostfix: consumed '(', current type=%d, lexeme='%s'\n",
               currentToken.type, currentToken.lexeme);
        AstNode *funcCall = createAstNode(AST_FUNC_CALL);
        strncpy(funcCall->funcCall.name, node->identifier.name, sizeof(funcCall->funcCall.name));
        funcCall->funcCall.arguments = NULL;
        funcCall->funcCall.argCount = 0;
        freeAstNode(node);
        while (currentToken.type != TOKEN_RPAREN) {
            AstNode *arg = parseExpression();
            funcCall->funcCall.argCount++;
            funcCall->funcCall.arguments = realloc(funcCall->funcCall.arguments,
                                                   funcCall->funcCall.argCount * sizeof(AstNode *));
            funcCall->funcCall.arguments[funcCall->funcCall.argCount - 1] = arg;
            printf("parsePostfix: parsed argument, current type=%d, lexeme='%s'\n",
                   currentToken.type, currentToken.lexeme);
            if (currentToken.type == TOKEN_COMMA)
                advanceToken();
            else if (currentToken.type != TOKEN_RPAREN)
                parserError("Expected ',' or ')' in argument list");
        }
        advanceToken(); // consume ')'
        printf("parsePostfix: consumed ')', current type=%d, lexeme='%s'\n",
               currentToken.type, currentToken.lexeme);
        node = funcCall;
        return parsePostfix(node);
    }

    printf("parsePostfix: returning, current type=%d, lexeme='%s'\n",
           currentToken.type, currentToken.lexeme);
    return node;
}

/* parseProgram: Se espera que el programa inicie con "main" */
AstNode *parseProgram(void) {
    AstNode *programNode = createAstNode(AST_PROGRAM);
    programNode->program.statements = NULL;
    programNode->program.statementCount = 0;
    advanceToken();  // Obtener el primer token

    if (currentToken.type != TOKEN_IDENTIFIER || strcmp(currentToken.lexeme, "main") != 0)
        parserError("Program must start with 'main'");
    advanceToken(); // consume "main"
    if (currentToken.type == TOKEN_SEMICOLON)
        advanceToken(); // consume separador

    while (currentToken.type != TOKEN_EOF &&
           !(currentToken.type == TOKEN_END && strcmp(currentToken.lexeme, "end") == 0)) {
        AstNode *stmt = parseStatement();
        printf("Parsed statement, next token: type=%d, lexeme='%s'\n", currentToken.type, currentToken.lexeme);
        skipStatementSeparators();
        programNode->program.statementCount++;
        programNode->program.statements = realloc(programNode->program.statements,
                                                  programNode->program.statementCount * sizeof(AstNode *));
        programNode->program.statements[programNode->program.statementCount - 1] = stmt;
    }
    if (currentToken.type == TOKEN_END)
        advanceToken(); // consume final "end"
    return programNode;
}

/* parseStatement: Determina el tipo de sentencia según currentToken */
static AstNode *parseStatement(void) {
    if (currentToken.type == TOKEN_FUNC) {
        return parseFuncDef();
    } else if (currentToken.type == TOKEN_RETURN) {
        return parseReturn();
    } else if (currentToken.type == TOKEN_PRINT) {
        advanceToken(); // consume "print"
        if (currentToken.type != TOKEN_LPAREN)
            parserError("Expected '(' after 'print'");
        advanceToken(); // consume '('
        AstNode *expr = parseExpression();
        if (currentToken.type != TOKEN_RPAREN)
            parserError("Expected ')' after print expression");
        advanceToken(); // consume ')'
        AstNode *printNode = createAstNode(AST_PRINT_STMT);
        printNode->printStmt.expr = expr;
        return printNode;
    } else if (currentToken.type == TOKEN_IF) {
        return parseIfStmt();
    } else if (currentToken.type == TOKEN_FOR) {
        return parseForStmt();
    } else if (currentToken.type == TOKEN_CLASS) {
        return parseClassDef();
    } else if (currentToken.type == TOKEN_IMPORT) {
        AstNode *importNode = createAstNode(AST_IMPORT);
        advanceToken(); // consume "import"
        if (currentToken.type != TOKEN_IDENTIFIER)
            parserError("Expected module type after import");
        strncpy(importNode->importStmt.moduleType, currentToken.lexeme, sizeof(importNode->importStmt.moduleType));
        advanceToken();
        if (currentToken.type != TOKEN_STRING)
            parserError("Expected module name string after module type");
        strncpy(importNode->importStmt.moduleName, currentToken.lexeme, sizeof(importNode->importStmt.moduleName));
        advanceToken();
        return importNode;
    } else if (currentToken.type == TOKEN_UI) {
        AstNode *uiNode = createAstNode(AST_IMPORT);
        strncpy(uiNode->importStmt.moduleType, "ui", sizeof(uiNode->importStmt.moduleType));
        advanceToken(); // consume "ui"
        if (currentToken.type != TOKEN_STRING)
            parserError("Expected string after 'ui'");
        strncpy(uiNode->importStmt.moduleName, currentToken.lexeme, sizeof(uiNode->importStmt.moduleName));
        advanceToken();
        return uiNode;
    } else if (currentToken.type == TOKEN_CSS) {
        AstNode *cssNode = createAstNode(AST_IMPORT);
        strncpy(cssNode->importStmt.moduleType, "css", sizeof(cssNode->importStmt.moduleType));
        advanceToken(); // consume "css"
        if (currentToken.type != TOKEN_STRING)
            parserError("Expected string after 'css'");
        strncpy(cssNode->importStmt.moduleName, currentToken.lexeme, sizeof(cssNode->importStmt.moduleName));
        advanceToken();
        return cssNode;
    } else if (currentToken.type == TOKEN_REGISTER_EVENT) {
        advanceToken(); // consume "register_event"
        if (currentToken.type != TOKEN_LPAREN)
            parserError("Expected '(' after register_event");
        advanceToken(); // consume '('
        AstNode *regCall = createAstNode(AST_FUNC_CALL);
        strncpy(regCall->funcCall.name, "register_event", sizeof(regCall->funcCall.name));
        regCall->funcCall.argCount = 0;
        regCall->funcCall.arguments = NULL;
        while (currentToken.type != TOKEN_RPAREN) {
            AstNode *arg = parseExpression();
            regCall->funcCall.argCount++;
            regCall->funcCall.arguments = realloc(regCall->funcCall.arguments,
                                                  regCall->funcCall.argCount * sizeof(AstNode *));
            regCall->funcCall.arguments[regCall->funcCall.argCount - 1] = arg;
            if (currentToken.type == TOKEN_COMMA)
                advanceToken();
            else if (currentToken.type != TOKEN_RPAREN)
                parserError("Expected ',' or ')' in register_event argument list");
        }
        advanceToken(); // consume ')'
        return regCall;
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        Token temp = currentToken;
        LexerState saved = lexSaveState();
        advanceToken();
        if (currentToken.type == TOKEN_DOT) {
            advanceToken(); // consume '.'
            if (currentToken.type != TOKEN_IDENTIFIER)
                parserError("Expected identifier after '.'");
            AstNode *memberNode = createAstNode(AST_MEMBER_ACCESS);
            memberNode->memberAccess.object = createAstNode(AST_IDENTIFIER);
            strncpy(memberNode->memberAccess.object->identifier.name, temp.lexeme,
                    sizeof(memberNode->memberAccess.object->identifier.name));
            strncpy(memberNode->memberAccess.member, currentToken.lexeme,
                    sizeof(memberNode->memberAccess.member));
            advanceToken(); // consume identificador del miembro
            if (currentToken.type == TOKEN_ASSIGN) {
                advanceToken(); // consume '='
                AstNode *value = NULL;
                LexerState saved2 = lexSaveState();
                if (currentToken.type == TOKEN_LPAREN && isLambdaLookahead()) {
                    lexRestoreState(saved2);
                    value = parseLambda();
                } else {
                    lexRestoreState(saved2);
                    value = parseExpression();
                }
                AstNode *assignNode = createAstNode(AST_VAR_ASSIGN);
                snprintf(assignNode->varAssign.name, sizeof(assignNode->varAssign.name),
                         "%s.%s", temp.lexeme, memberNode->memberAccess.member);
                assignNode->varAssign.value = value;
                freeAstNode(memberNode);
                return assignNode;
            } else {
                return parsePostfix(memberNode);
            }
        }
        if (currentToken.type == TOKEN_ASSIGN) {
            advanceToken(); // consume '='
            AstNode *value;
            LexerState saved2 = lexSaveState();
            if (currentToken.type == TOKEN_LPAREN && isLambdaLookahead()) {
                lexRestoreState(saved2);
                value = parseLambda();
            } else {
                lexRestoreState(saved2);
                value = parseExpression();
            }
            AstNode *assignNode = createAstNode(AST_VAR_ASSIGN);
            strncpy(assignNode->varAssign.name, temp.lexeme, sizeof(assignNode->varAssign.name));
            assignNode->varAssign.value = value;
            return assignNode;
        }
        else if (currentToken.type == TOKEN_INT ||
                 currentToken.type == TOKEN_FLOAT ||
                 (currentToken.type == TOKEN_IDENTIFIER &&
                  (strcmp(currentToken.lexeme, "int") == 0 || strcmp(currentToken.lexeme, "float") == 0))) {
            AstNode *declNode = createAstNode(AST_VAR_DECL);
            strncpy(declNode->varDecl.name, temp.lexeme, sizeof(declNode->varDecl.name));
            strncpy(declNode->varDecl.type, currentToken.lexeme, sizeof(declNode->varDecl.type));
            advanceToken(); // consume el tipo
            return declNode;
        }
        else if (currentToken.type == TOKEN_LPAREN) {
            advanceToken(); // consume '('
            AstNode *funcCall = createAstNode(AST_FUNC_CALL);
            strncpy(funcCall->funcCall.name, temp.lexeme, sizeof(funcCall->funcCall.name));
            funcCall->funcCall.arguments = NULL;
            funcCall->funcCall.argCount = 0;
            while (currentToken.type != TOKEN_RPAREN) {
                AstNode *arg = parseExpression();
                funcCall->funcCall.argCount++;
                funcCall->funcCall.arguments = realloc(funcCall->funcCall.arguments,
                                                       funcCall->funcCall.argCount * sizeof(AstNode *));
                funcCall->funcCall.arguments[funcCall->funcCall.argCount - 1] = arg;
                if (currentToken.type == TOKEN_COMMA)
                    advanceToken();
                else if (currentToken.type != TOKEN_RPAREN)
                    parserError("Expected ',' or ')' in function call argument list");
            }
            advanceToken(); // consume ')'
            AstNode *callNode = parsePostfix(funcCall);
            return callNode;
        } else {
            lexRestoreState(saved);
            return parseExpression();
        }
    } else {
        return parseExpression();
    }
}

/* parseExpression: Maneja operadores '+', '-', comparaciones */
static AstNode *parseExpression(void) {
    AstNode *node = parseTerm();
    printf("parseExpression: initial term parsed, current: type=%d, lexeme='%s'\n", currentToken.type, currentToken.lexeme);

    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS ||
           currentToken.type == TOKEN_GT || currentToken.type == TOKEN_LT ||
           currentToken.type == TOKEN_GTE || currentToken.type == TOKEN_LTE ||
           currentToken.type == TOKEN_EQ || currentToken.type == TOKEN_NEQ) {
        char op;
        switch (currentToken.type) {
            case TOKEN_PLUS: op = '+'; break;
            case TOKEN_MINUS: op = '-'; break;
            case TOKEN_GT: op = '>'; break;
            case TOKEN_LT: op = '<'; break;
            case TOKEN_GTE: op = 'G'; break;
            case TOKEN_LTE: op = 'L'; break;
            case TOKEN_EQ: op = 'E'; break;
            case TOKEN_NEQ: op = 'N'; break;
            default: op = currentToken.lexeme[0];
        }
        advanceToken();
        AstNode *right = parseTerm();
        printf("parseExpression: right term parsed, current: type=%d, lexeme='%s'\n", currentToken.type, currentToken.lexeme);
        AstNode *binOp = createAstNode(AST_BINARY_OP);
        binOp->binaryOp.left = node;
        binOp->binaryOp.op = op;
        binOp->binaryOp.right = right;
        node = binOp;
    }
    return node;
}

/* parseTerm: Maneja operadores '*' y '/' */
static AstNode *parseTerm(void) {
    AstNode *left = parseFactor();
    while (currentToken.type == TOKEN_ASTERISK || currentToken.type == TOKEN_SLASH) {
        char op = currentToken.lexeme[0];
        advanceToken();
        AstNode *right = parseFactor();
        AstNode *binOp = createAstNode(AST_BINARY_OP);
        binOp->binaryOp.left = left;
        binOp->binaryOp.op = op;
        binOp->binaryOp.right = right;
        left = binOp;
    }
    return left;
}

/* parseFactor: Números, cadenas, identificadores, agrupación */
static AstNode *parseFactor(void) {
    AstNode *node = NULL;
    if (currentToken.type == TOKEN_NUMBER) {
        node = createAstNode(AST_NUMBER_LITERAL);
        node->numberLiteral.value = atof(currentToken.lexeme);
        advanceToken();
    } else if (currentToken.type == TOKEN_STRING) {
        node = createAstNode(AST_STRING_LITERAL);
        strncpy(node->stringLiteral.value, currentToken.lexeme, sizeof(node->stringLiteral.value));
        advanceToken();
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        node = createAstNode(AST_IDENTIFIER);
        strncpy(node->identifier.name, currentToken.lexeme, sizeof(node->identifier.name));
        advanceToken();
        node = parsePostfix(node);
    } else if (currentToken.type == TOKEN_LPAREN) {
        advanceToken(); // consume '('
        node = parseExpression();
        if (currentToken.type != TOKEN_RPAREN)
            parserError("Expected ')' after expression");
        advanceToken(); // consume ')'
    } else if (currentToken.type == TOKEN_LBRACKET) {
        node = parseArrayLiteral();
    } else {
        parserError("Unexpected token in expression");
    }
    return node;
}

/* parseFuncDef: Parsea una definición de función */
static AstNode *parseFuncDef(void) {
    advanceToken(); // consume 'func'
    if (currentToken.type != TOKEN_IDENTIFIER)
        parserError("Expected function name after 'func'");
    char funcName[256];
    strncpy(funcName, currentToken.lexeme, sizeof(funcName));
    advanceToken();

    if (currentToken.type != TOKEN_LPAREN)
        parserError("Expected '(' after function name");
    advanceToken(); // consume '('

    AstNode **parameters = NULL;
    int paramCount = 0;
    while (currentToken.type != TOKEN_RPAREN) {
        if (currentToken.type != TOKEN_IDENTIFIER)
            parserError("Expected parameter name in function definition");
        char paramName[256];
        strncpy(paramName, currentToken.lexeme, sizeof(paramName));
        AstNode *param = createAstNode(AST_IDENTIFIER);
        strncpy(param->identifier.name, currentToken.lexeme, sizeof(param->identifier.name));
        parameters = realloc(parameters, (paramCount + 1) * sizeof(AstNode *));
        parameters[paramCount++] = param;
        advanceToken();

        if (strcmp(paramName, "self") == 0) {
            if (currentToken.type == TOKEN_COMMA) {
                advanceToken();
                continue;
            } else if (currentToken.type != TOKEN_RPAREN) {
                parserError("Expected ',' or ')' after 'self'");
            }
            continue;
        }

        if (currentToken.type != TOKEN_INT && currentToken.type != TOKEN_FLOAT &&
            !(currentToken.type == TOKEN_IDENTIFIER &&
              (strcmp(currentToken.lexeme, "int") == 0 || strcmp(currentToken.lexeme, "float") == 0))) {
            parserError("Expected parameter type in function definition");
        }
        advanceToken();

        if (currentToken.type == TOKEN_COMMA)
            advanceToken();
        else if (currentToken.type != TOKEN_RPAREN)
            parserError("Expected ',' or ')' in parameter list");
    }
    advanceToken(); // consume ')'

    char retType[64] = "";
    if (currentToken.type == TOKEN_ARROW) {
        advanceToken();
        if (!(currentToken.type == TOKEN_IDENTIFIER ||
              currentToken.type == TOKEN_INT || currentToken.type == TOKEN_FLOAT))
            parserError("Expected return type after '->'");
        strncpy(retType, currentToken.lexeme, sizeof(retType));
        advanceToken();
    }

    printf("parseFuncDef: Token after header: type=%d, lexeme='%s'\n",
           currentToken.type, currentToken.lexeme);

    if (currentToken.type == TOKEN_SEMICOLON) {
        advanceToken();
        printf("parseFuncDef: Separador ';' consumed\n");
    }

    AstNode **body = NULL;
    int bodyCount = 0;
    while (currentToken.type == TOKEN_SEMICOLON)
        advanceToken();
    while (currentToken.type != TOKEN_END) {
        AstNode *stmt = parseStatement();
        while (currentToken.type == TOKEN_SEMICOLON)
            advanceToken();
        body = realloc(body, (bodyCount + 1) * sizeof(AstNode *));
        body[bodyCount++] = stmt;
    }
    advanceToken(); // consume 'end'

    AstNode *funcNode = createAstNode(AST_FUNC_DEF);
    strncpy(funcNode->funcDef.name, funcName, sizeof(funcNode->funcDef.name));
    funcNode->funcDef.parameters = parameters;
    funcNode->funcDef.paramCount = paramCount;
    strncpy(funcNode->funcDef.returnType, retType, sizeof(funcNode->funcDef.returnType));
    funcNode->funcDef.body = body;
    funcNode->funcDef.bodyCount = bodyCount;

    return funcNode;
}

/* parseReturn: Parsea una sentencia return */
static AstNode *parseReturn(void) {
    advanceToken(); // consume 'return'
    AstNode *expr = parseExpression();
    AstNode *retNode = createAstNode(AST_RETURN_STMT);
    retNode->returnStmt.expr = expr;
    return retNode;
}

/* parseIfStmt: Parsea una estructura if-else */
static AstNode *parseIfStmt(void) {
    advanceToken(); // consume 'if'
    AstNode *condition = parseExpression();

    skipStatementSeparators();

    AstNode **thenBranch = NULL;
    int thenCount = 0;
    while (currentToken.type != TOKEN_ELSE && currentToken.type != TOKEN_END) {
        AstNode *stmt = parseStatement();
        thenBranch = realloc(thenBranch, (thenCount + 1) * sizeof(AstNode *));
        thenBranch[thenCount++] = stmt;
        skipStatementSeparators();
    }

    AstNode **elseBranch = NULL;
    int elseCount = 0;
    if (currentToken.type == TOKEN_ELSE) {
        advanceToken(); // consume 'else'
        skipStatementSeparators();
        while (currentToken.type != TOKEN_END) {
            AstNode *stmt = parseStatement();
            elseBranch = realloc(elseBranch, (elseCount + 1) * sizeof(AstNode *));
            elseBranch[elseCount++] = stmt;
            skipStatementSeparators();
        }
    }

    if (currentToken.type != TOKEN_END)
        parserError("Expected 'end' after if statement");
    advanceToken(); // consume 'end'

    AstNode *ifNode = createAstNode(AST_IF_STMT);
    ifNode->ifStmt.condition = condition;
    ifNode->ifStmt.thenBranch = thenBranch;
    ifNode->ifStmt.thenCount = thenCount;
    ifNode->ifStmt.elseBranch = elseBranch;
    ifNode->ifStmt.elseCount = elseCount;
    return ifNode;
}

/* parseForStmt: for i in range(...) ... end */
static AstNode *parseForStmt(void) {
    advanceToken(); // consume "for"

    if (currentToken.type != TOKEN_IDENTIFIER)
        parserError("Expected iterator identifier in for loop");

    char iterator[256];
    strncpy(iterator, currentToken.lexeme, sizeof(iterator));
    advanceToken(); // consume iterador

    if (currentToken.type != TOKEN_IN)
        parserError("Expected 'in' in for loop");
    advanceToken(); // consume "in"

    if (currentToken.type != TOKEN_RANGE)
        parserError("Expected 'range' in for loop");
    advanceToken(); // consume "range"

    if (currentToken.type != TOKEN_LPAREN)
        parserError("Expected '(' after 'range'");
    advanceToken(); // consume '('

    AstNode *rangeStart = parseExpression();
    AstNode *rangeEnd = NULL;
    if (currentToken.type == TOKEN_COMMA) {
        advanceToken(); // consume ','
        rangeEnd = parseExpression();
    } else {
        AstNode *zeroNode = createAstNode(AST_NUMBER_LITERAL);
        zeroNode->numberLiteral.value = 0;
        rangeEnd = rangeStart;
        rangeStart = zeroNode;
    }

    if (currentToken.type != TOKEN_RPAREN)
        parserError("Expected ')' after range arguments");
    advanceToken(); // consume ')'

    skipStatementSeparators();

    AstNode **body = NULL;
    int bodyCount = 0;
    while (currentToken.type != TOKEN_END && currentToken.type != TOKEN_EOF) {
        AstNode *stmt = parseStatement();
        body = realloc(body, (bodyCount + 1) * sizeof(AstNode *));
        body[bodyCount++] = stmt;
        skipStatementSeparators();
    }
    if (currentToken.type != TOKEN_END)
        parserError("Expected 'end' to close for loop");
    advanceToken(); // consume "end"

    AstNode *forNode = createAstNode(AST_FOR_STMT);
    strncpy(forNode->forStmt.iterator, iterator, sizeof(forNode->forStmt.iterator));
    forNode->forStmt.rangeStart = rangeStart;
    forNode->forStmt.rangeEnd = rangeEnd;
    forNode->forStmt.body = body;
    forNode->forStmt.bodyCount = bodyCount;
    return forNode;
}

/* parseClassDef: Parsea class <Name>; ... end */
static AstNode *parseClassDef(void) {
    advanceToken(); // consume 'class'
    if (currentToken.type != TOKEN_IDENTIFIER)
        parserError("Expected class name");
    char className[256];
    strncpy(className, currentToken.lexeme, sizeof(className));
    advanceToken();
    if (currentToken.type == TOKEN_SEMICOLON)
        advanceToken(); // separador opcional

    AstNode **members = NULL;
    int memberCount = 0;
    while (currentToken.type != TOKEN_END) {
        AstNode *stmt = parseStatement();
        while (currentToken.type == TOKEN_SEMICOLON)
            advanceToken();
        members = realloc(members, (memberCount + 1) * sizeof(AstNode *));
        members[memberCount++] = stmt;
    }
    advanceToken(); // consume 'end'

    AstNode *classNode = createAstNode(AST_CLASS_DEF);
    strncpy(classNode->classDef.name, className, sizeof(classNode->classDef.name));
    classNode->classDef.members = members;
    classNode->classDef.memberCount = memberCount;
    return classNode;
}

/* parseLambda: ( <paramName> <paramType> [, <paramName> <paramType> ... ] ) -> <returnType> => <bodyExpr> */
static AstNode *parseLambda(void) {
    advanceToken(); // consume '('

    AstNode **parameters = NULL;
    int paramCount = 0;

    while (currentToken.type != TOKEN_RPAREN) {
        if (currentToken.type != TOKEN_IDENTIFIER)
            parserError("Expected parameter name in lambda");
        AstNode *param = createAstNode(AST_IDENTIFIER);
        strncpy(param->identifier.name, currentToken.lexeme, sizeof(param->identifier.name));
        advanceToken();

        if (!(currentToken.type == TOKEN_INT || currentToken.type == TOKEN_FLOAT ||
              (currentToken.type == TOKEN_IDENTIFIER &&
               (strcmp(currentToken.lexeme, "int") == 0 || strcmp(currentToken.lexeme, "float") == 0)))) {
            parserError("Expected parameter type in lambda after paramName");
        }
        advanceToken();

        if (currentToken.type == TOKEN_COMMA)
            advanceToken();
        else if (currentToken.type != TOKEN_RPAREN)
            parserError("Expected ',' or ')' in lambda parameter list");

        parameters = realloc(parameters, (paramCount + 1) * sizeof(AstNode *));
        parameters[paramCount++] = param;
    }
    advanceToken(); // consume ')'

    if (currentToken.type != TOKEN_ARROW)
        parserError("Expected '->' after lambda parameters");
    advanceToken();

    char retType[64] = "";
    if (currentToken.type == TOKEN_IDENTIFIER || currentToken.type == TOKEN_INT || currentToken.type == TOKEN_FLOAT) {
        strncpy(retType, currentToken.lexeme, sizeof(retType));
        advanceToken();
    }

    if (currentToken.type != TOKEN_FAT_ARROW)
        parserError("Expected '=>' in lambda");
    advanceToken();

    AstNode *body = parseExpression();

    AstNode *lambdaNode = createAstNode(AST_LAMBDA);
    lambdaNode->lambda.parameters = parameters;
    lambdaNode->lambda.paramCount = paramCount;
    strncpy(lambdaNode->lambda.returnType, retType, sizeof(lambdaNode->lambda.returnType));
    lambdaNode->lambda.body = body;
    return lambdaNode;
}

/* parseArrayLiteral: [ elem, elem, ... ] */
static AstNode *parseArrayLiteral(void) {
    advanceToken(); // consumir '['
    AstNode **elements = NULL;
    int count = 0;

    if (currentToken.type != TOKEN_RBRACKET) {
        while (1) {
            AstNode *element = parseExpression();
            elements = realloc(elements, (count + 1) * sizeof(AstNode *));
            if (!elements)
                parserError("Error de asignación de memoria para array literal");
            elements[count++] = element;

            if (currentToken.type == TOKEN_COMMA) {
                advanceToken(); // consumir ','
            } else {
                break;
            }
        }
    }
    
    if (currentToken.type != TOKEN_RBRACKET)
        parserError("Se esperaba ']' al finalizar el literal de arreglo");
    advanceToken(); // consumir ']'

    AstNode *node = createAstNode(AST_ARRAY_LITERAL);
    node->arrayLiteral.elements = elements;
    node->arrayLiteral.elementCount = count;
    return node;
}

/* Función auxiliar para consumir separadores de sentencia */
static void skipStatementSeparators(void) {
    while (currentToken.type == TOKEN_SEMICOLON)
        advanceToken();
}

void freeAst(AstNode *root) {
    freeAstNode(root);
}
