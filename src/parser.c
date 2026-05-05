// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Lexer */
typedef struct {
    const char *sql;
    size_t pos;
    int current_tok;
    char *tok_str;
    size_t tok_len;
} Lexer;

/* AST node types */
typedef enum {
    AST_CREATE_TABLE,
    AST_INSERT,
    AST_SELECT,
    AST_UPDATE,
    AST_DELETE,
    AST_CREATE_INDEX,
    AST_DROP_TABLE,
    AST_DROP_INDEX,
    AST_BEGIN,
    AST_COMMIT,
    AST_ROLLBACK
} AstType;

typedef struct AstNode {
    AstType type;
    /* union of specific data */
} AstNode;

/* Parser entry point */
int zsdf_parse_sql(const char *sql, AstNode **root) {
    Lexer lex;
    /* Initialize lexer */
    /* Call recursive descent */
    return ZSDF_OK;
}
