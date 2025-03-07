#include "semantic.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*              Gestión de la Pila de Tablas de Símbolos                      */
/* -------------------------------------------------------------------------- */

/* Se ha ampliado la estructura de símbolo para incluir el nombre del tipo en
   caso de que sea un tipo definido por el usuario (clase). */
typedef struct Symbol {
    char name[256];
    DataType type;
    char customType[256]; // Solo se usa si type == TYPE_CLASS
    struct Symbol *next;
} Symbol;

/* Cada tabla de símbolos es una lista enlazada; la pila se implementa mediante */
typedef struct SymbolTable {
    Symbol *symbols;
    struct SymbolTable *parent;
} SymbolTable;

static SymbolTable *currentTable = NULL;

/**
 * @brief Crea una nueva tabla de símbolos y la empuja en la pila.
 */
static void pushScope(void) {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!table) {
        fprintf(stderr, "Memory error while creating new scope.\n");
        exit(1);
    }
    table->symbols = NULL;
    table->parent = currentTable;
    currentTable = table;
}

/**
 * @brief Sale (pop) del ámbito actual, liberando la tabla de símbolos.
 */
static void popScope(void) {
    if (!currentTable) return;
    Symbol *sym = currentTable->symbols;
    while (sym) {
        Symbol *tmp = sym;
        sym = sym->next;
        free(tmp);
    }
    SymbolTable *toPop = currentTable;
    currentTable = currentTable->parent;
    free(toPop);
}

/**
 * @brief Busca un símbolo en la pila de tablas.
 *
 * Recorre desde el ámbito actual hasta el global.
 *
 * @param name Nombre de la variable.
 * @return Symbol* Puntero al símbolo, o NULL si no se encuentra.
 */
static Symbol *lookupSymbol(const char *name) {
    SymbolTable *table = currentTable;
    while (table) {
        Symbol *sym = table->symbols;
        while (sym) {
            if (strcmp(sym->name, name) == 0)
                return sym;
            sym = sym->next;
        }
        table = table->parent;
    }
    return NULL;
}

/**
 * @brief Inserta un símbolo en la tabla del ámbito actual.
 *
 * Si el símbolo ya existe en el ámbito actual, se emite un error.
 *
 * @param name Nombre de la variable.
 * @param type Tipo declarado.
 * @param customType Cadena con el nombre del tipo personalizado (si type == TYPE_CLASS), o "".
 */
static void addSymbol(const char *name, DataType type, const char *customType) {
    if (!currentTable) {
        pushScope();
    }
    Symbol *iter = currentTable->symbols;
    while (iter) {
        if (strcmp(iter->name, name) == 0) {
            fprintf(stderr, "Semantic error: Variable '%s' redeclared in the same scope.\n", name);
            exit(1);
        }
        iter = iter->next;
    }
    Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Memory error in addSymbol.\n");
        exit(1);
    }
    strncpy(sym->name, name, sizeof(sym->name)-1);
    sym->name[sizeof(sym->name)-1] = '\0';
    sym->type = type;
    if (type == TYPE_CLASS && customType) {
        strncpy(sym->customType, customType, sizeof(sym->customType)-1);
        sym->customType[sizeof(sym->customType)-1] = '\0';
    } else {
        sym->customType[0] = '\0';
    }
    sym->next = currentTable->symbols;
    currentTable->symbols = sym;
}

/**
 * @brief Actualiza el tipo de un símbolo (usado en asignaciones implícitas).
 *
 * @param name Nombre de la variable.
 * @param type Nuevo tipo.
 * @param customType Cadena con el nombre del tipo personalizado (si aplica).
 */
static void updateSymbol(const char *name, DataType type, const char *customType) {
    Symbol *sym = lookupSymbol(name);
    if (!sym) {
        fprintf(stderr, "Semantic error: Variable '%s' not declared.\n", name);
        exit(1);
    }
    sym->type = type;
    if (type == TYPE_CLASS && customType) {
        strncpy(sym->customType, customType, sizeof(sym->customType)-1);
        sym->customType[sizeof(sym->customType)-1] = '\0';
    }
}

/* -------------------------------------------------------------------------- */
/*                      Mapeo de cadenas de tipo a DataType                   */
/* -------------------------------------------------------------------------- */

/**
 * @brief Convierte una cadena de tipo a DataType.
 *
 * Si la cadena no coincide con "int", "float" o "string", se considera
 * un tipo de clase (TYPE_CLASS).
 *
 * @param typeStr Cadena que representa el tipo.
 * @param customTypeOut Buffer donde se copia el nombre del tipo si es custom.
 * @param customTypeSize Tamaño del buffer customTypeOut.
 * @return DataType
 */
static DataType mapTypeString(const char *typeStr, char *customTypeOut, size_t customTypeSize) {
    if (strcmp(typeStr, "int") == 0)
        return TYPE_INT;
    else if (strcmp(typeStr, "float") == 0)
        return TYPE_FLOAT;
    else if (strcmp(typeStr, "string") == 0)
        return TYPE_STRING;
    else {
        if (customTypeOut && customTypeSize > 0) {
            strncpy(customTypeOut, typeStr, customTypeSize-1);
            customTypeOut[customTypeSize-1] = '\0';
        }
        return TYPE_CLASS;
    }
}

/* -------------------------------------------------------------------------- */
/*                     Inferencia y verificación de tipos                     */
/* -------------------------------------------------------------------------- */

/**
 * @brief Infiera el tipo de un nodo AST.
 *
 * Se diferencia entre números (int o float), cadenas, identificadores y llamadas.
 *
 * @param node Nodo del AST.
 * @return DataType Tipo inferido.
 */
static DataType inferType(AstNode *node) {
    if (!node) return TYPE_UNKNOWN;

    switch (node->type) {

        case AST_NUMBER_LITERAL: {
            double val = node->numberLiteral.value;
            return ((int)val == val) ? TYPE_INT : TYPE_FLOAT;
        }

        case AST_STRING_LITERAL:
            return TYPE_STRING;

        case AST_IDENTIFIER: {
            Symbol *sym = lookupSymbol(node->identifier.name);
            if (!sym) {
                fprintf(stderr, "Semantic error: Variable '%s' not declared.\n",
                        node->identifier.name);
                exit(1);
            }
            return sym->type;
        }

        case AST_BINARY_OP: {
            DataType left = inferType(node->binaryOp.left);
            DataType right = inferType(node->binaryOp.right);

            // Si no pudimos determinar uno de los dos, devolvemos desconocido
            if (left == TYPE_UNKNOWN || right == TYPE_UNKNOWN)
                return TYPE_UNKNOWN;

            // Operador '+'
            if (node->binaryOp.op == '+') {
                // Si alguno es string, el resultado es string (concatenación)
                if (left == TYPE_STRING || right == TYPE_STRING)
                    return TYPE_STRING;
                // Si alguno es float => float
                if (left == TYPE_FLOAT || right == TYPE_FLOAT)
                    return TYPE_FLOAT;
                // Por defecto int
                return TYPE_INT;
            }

            // Operadores '-', '*', '/'
            else if (node->binaryOp.op == '-' ||
                     node->binaryOp.op == '*' ||
                     node->binaryOp.op == '/') {
                // Si alguno es float => float
                if (left == TYPE_FLOAT || right == TYPE_FLOAT)
                    return TYPE_FLOAT;
                return TYPE_INT;
            }

            // Desconocido si no coincide
            return TYPE_UNKNOWN;
        }

        /*
         * Si tu parser produce un AST_FUNC_CALL para invocaciones (p.ej. x.to_str())
         * podrías reconocer aquí la función y forzar su tipo:
         */
        case AST_FUNC_CALL: {
            // Nombre de la función en node->funcCall.name
            if (strcmp(node->funcCall.name, "to_str") == 0) {
                // to_str() => string
                return TYPE_STRING;
            }
            else if (strcmp(node->funcCall.name, "suma_numpy") == 0) {
                // Elige lo que convenga (int o float)
                return TYPE_INT; 
            }
            // Si no es conocida, lo dejamos como desconocido
            return TYPE_UNKNOWN;
        }

        default:
            return TYPE_UNKNOWN;
    }
}

/* -------------------------------------------------------------------------- */
/*                      Análisis Semántico Recursivo                          */
/* -------------------------------------------------------------------------- */

static void analyzeNode(AstNode *node) {
    if (!node) return;

    switch (node->type) {

        case AST_PROGRAM:
            pushScope();  // Ámbito global
            for (int i = 0; i < node->program.statementCount; i++) {
                analyzeNode(node->program.statements[i]);
            }
            popScope();
            break;

        case AST_VAR_DECL: {
            char customType[256] = "";
            DataType declType = mapTypeString(node->varDecl.type,
                                              customType,
                                              sizeof(customType));
            addSymbol(node->varDecl.name, declType, customType);
            break;
        }

        case AST_VAR_ASSIGN: {
            analyzeNode(node->varAssign.initializer);
            DataType assignedType = inferType(node->varAssign.initializer);
            Symbol *sym = lookupSymbol(node->varAssign.name);
            if (!sym) {
                // Declaración implícita
                addSymbol(node->varAssign.name, assignedType, "");
            } else {
                if (sym->type != TYPE_UNKNOWN && sym->type != assignedType) {
                    fprintf(stderr,
                            "Semantic error: Incompatible assignment for variable '%s'.\n",
                            node->varAssign.name);
                    exit(1);
                }
                if (sym->type == TYPE_UNKNOWN) {
                    updateSymbol(node->varAssign.name, assignedType, "");
                }
            }
            break;
        }

        case AST_FUNC_DEF:
            pushScope(); // Nuevo ámbito para la función
            for (int i = 0; i < node->funcDef.paramCount; i++) {
                // Se asume que los parámetros llevan un tipo, o int por defecto
                addSymbol(node->funcDef.parameters[i]->identifier.name, TYPE_INT, "");
            }
            for (int i = 0; i < node->funcDef.bodyCount; i++) {
                analyzeNode(node->funcDef.body[i]);
            }
            popScope();
            break;

        case AST_RETURN_STMT:
            analyzeNode(node->returnStmt.expr);
            break;

        case AST_PRINT_STMT:
            analyzeNode(node->printStmt.expr);
            break;

        case AST_BINARY_OP: {
            // Analizar subnodos
            analyzeNode(node->binaryOp.left);
            analyzeNode(node->binaryOp.right);

            // Verificar tipos
            DataType leftType = inferType(node->binaryOp.left);
            DataType rightType = inferType(node->binaryOp.right);

            // Aquí mantenemos el warning si no se determinó tipo
            if (leftType == TYPE_UNKNOWN || rightType == TYPE_UNKNOWN) {
                fprintf(stderr,
                        "Warning: Unable to determine types in binary operation '%c'.\n",
                        node->binaryOp.op);
            }
            else if (node->binaryOp.op == '+') {
                // Permitir concatenación con strings
                if (leftType == TYPE_STRING || rightType == TYPE_STRING) {
                    // Nada, se asume OK
                }
                else if (leftType != rightType) {
                    // Si son numéricos, exigimos que coincidan
                    fprintf(stderr,
                            "Semantic error: Incompatible types in binary '+' operation.\n");
                    exit(1);
                }
            }
            else if (node->binaryOp.op == '-' ||
                     node->binaryOp.op == '*' ||
                     node->binaryOp.op == '/') {
                // Exigimos que coincidan si no son desconocidos
                if (leftType != TYPE_UNKNOWN && rightType != TYPE_UNKNOWN &&
                    leftType != rightType) {
                    fprintf(stderr,
                            "Semantic error: Incompatible types in binary '%c' operation.\n",
                            node->binaryOp.op);
                    exit(1);
                }
            }
            break;
        }

        case AST_LAMBDA: {
            pushScope();
            for (int i = 0; i < node->lambda.paramCount; i++) {
                addSymbol(node->lambda.parameters[i]->identifier.name, TYPE_INT, "");
            }
            analyzeNode(node->lambda.body);
            popScope();
            break;
        }

        case AST_IF_STMT:
            analyzeNode(node->ifStmt.condition);
            pushScope();
            for (int i = 0; i < node->ifStmt.thenCount; i++) {
                analyzeNode(node->ifStmt.thenBranch[i]);
            }
            popScope();
            pushScope();
            for (int i = 0; i < node->ifStmt.elseCount; i++) {
                analyzeNode(node->ifStmt.elseBranch[i]);
            }
            popScope();
            break;

        case AST_FOR_STMT:
            analyzeNode(node->forStmt.rangeStart);
            analyzeNode(node->forStmt.rangeEnd);
            pushScope();
            addSymbol(node->forStmt.iterator, TYPE_INT, "");
            for (int i = 0; i < node->forStmt.bodyCount; i++) {
                analyzeNode(node->forStmt.body[i]);
            }
            popScope();
            break;

        case AST_CLASS_DEF:
            // Registrar el nombre de la clase
            addSymbol(node->classDef.name, TYPE_CLASS, node->classDef.name);
            pushScope();
            for (int i = 0; i < node->classDef.memberCount; i++) {
                analyzeNode(node->classDef.members[i]);
            }
            popScope();
            break;

        default:
            // Literales, identificadores, etc., sin acción extra
            break;
    }
}

/**
 * @brief Realiza el análisis semántico del AST.
 *
 * Inicializa la pila de ámbitos, analiza el AST y limpia la tabla de símbolos.
 *
 * @param root Puntero a la raíz del AST.
 */
void analyzeSemantics(AstNode *root) {
    pushScope(); // Ámbito global
    analyzeNode(root);
    popScope();
}
