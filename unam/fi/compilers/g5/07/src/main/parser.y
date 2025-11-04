/* ==================================================================
 * FILE: parser.y (Modular Version)
 * ================================================================== */

/* * ------------------------------------------------------------------
 * SECTION 1: C AND BISON DECLARATIONS
 * ------------------------------------------------------------------
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include the AST interface */
#include "ast.h" 

/* Lexer function prototype */
extern int yylex();

/* Error function prototype */
void yyerror(const char *s);

/* Global line number variable from lexer */
extern int yylineno;
%}

/* * ------------------------------------------------------------------
 * UNION AND TOKENS
 * ------------------------------------------------------------------
 */

/* Defines the semantic values that symbols can have */
%union {
  int intVal;
  float floatVal;
  char* strVal;
  struct ast_node *node; /* Non-terminals will return an AST node pointer */
}

// KEYWORDS

// Type keywords
%token <intVal> T_INT T_FLOAT T_CHAR T_VOID T_DOUBLE T_LONG T_SHORT T_SIGNED T_UNSIGNED
%token <intVal> T_CONST T_VOLATILE T_TYPEDEF

%token T_STRUCT T_UNION T_ENUM

// Selection and looping structures
%token T_IF T_ELSE T_WHILE T_FOR T_DO T_SWITCH T_CASE T_DEFAULT
%token T_BREAK T_CONTINUE T_RETURN T_GOTO

// More keywords
%token T_AUTO T_REGISTER T_STATIC T_EXTERN
%token T_SIZEOF

//IDENTIFIERS
%token <strVal> T_ID

//CONSTANTS
%token <intVal> T_ENTERO
%token <floatVal> T_NUMERO

//LITERALS
%token <strVal> T_CADENA
%token <strVal> T_CARACTER

//PUNCTUATORS
//parentesis
%token T_LPAREN T_RPAREN
//llaves
%token T_LBRACE T_RBRACE
//corchetes
%token T_LBRACKET T_RBRACKET
//punto y coma  & coma
%token T_SEMICOLON T_COMMA
//punto & flecha
%token T_DOT T_ARROW
//simbolo de interrogacion & dos puntos
%token T_QUESTION T_COLON

//OPERATORS
// =  &   +=    &      -=       &    *=    &    /=
%token T_ASSIGN T_ASSIGN_PLUS T_ASSIGN_MINUS T_ASSIGN_STAR T_ASSIGN_SLASH

// %=    &    <<=   &   >>=
%token T_ASSIGN_PERCENT T_ASSIGN_LSHIFT T_ASSIGN_RSHIFT

//  &=   &   |=      &    ^=
%token T_ASSIGN_AND T_ASSIGN_OR T_ASSIGN_XOR

//    +    -    * /       %
%token T_PLUS T_MINUS T_STAR T_SLASH T_PERCENT
%token T_INC T_DEC /* ++ -- */
//    ==   &   !=    &   <    &   <=    &   >    &   >=
%token T_EQ T_NEQ T_LT T_LE T_GT T_GE
%token T_AND T_OR  /* && || */
%token T_AMPERSAND /* & */
%token T_PIPE      /* | */
%token T_CARET     /* ^ */
%token T_NOT       /* ! */
%token T_TILDE     /* ~ */
%token T_LSHIFT T_RSHIFT /* << >> */

/* * ------------------------------------------------------------------
 * NON-TERMINAL TYPES
 * ------------------------------------------------------------------
 */
/* Specify that all non-terminals return a <node> pointer */
%type <node> programa declaracion_externa declaracion tipo_specifier
%type <node> lista_init_var init_var var funcion parametros parametro
%type <node> bloque sentencia expr_opcional if_sent while_sent
%type <node> do_while_sent for_sent switch_sent expr lista_args_opt lista_args


/* * ------------------------------------------------------------------
 * ERROR REPORTING
 * ------------------------------------------------------------------
 */
/* Enable detailed error messages (replaces %error-verbose) */
%define parse.error verbose

/* * ------------------------------------------------------------------
 * PRECEDENCE AND ASSOCIATIVITY
 * ------------------------------------------------------------------
 * (Lowest precedence at top, highest at bottom)
 */

%left T_COMMA
%right T_ASSIGN T_ASSIGN_PLUS T_ASSIGN_MINUS T_ASSIGN_STAR T_ASSIGN_SLASH
%right T_ASSIGN_PERCENT T_ASSIGN_LSHIFT T_ASSIGN_RSHIFT
%right T_ASSIGN_AND T_ASSIGN_OR T_ASSIGN_XOR

%right T_QUESTION T_COLON

%left T_OR
%left T_AND

%left T_PIPE
%left T_CARET
%left T_AMPERSAND

%left T_EQ T_NEQ

%left T_LT T_LE T_GT T_GE

%left T_LSHIFT T_RSHIFT

%left T_PLUS T_MINUS

%left T_STAR T_SLASH T_PERCENT

%right T_INC T_DEC T_NOT T_TILDE T_UMINUS T_SIZEOF

%nonassoc T_IFX
%nonassoc T_ELSE

%left T_LBRACKET T_RBRACKET T_LPAREN T_RPAREN T_DOT T_ARROW

%start programa

%%
/* ==================================================================
 * SECTION 2: GRAMMAR RULES (WITH SEMANTIC ACTIONS)
 * ================================================================== */

programa:
    /* empty program */
    { $$ = NULL; }
  | programa declaracion_externa
    { $$ = ast_append_sibling($1, $2); ast_root = $$; }
  ;

declaracion_externa:
    declaracion
    { $$ = $1; }
  | funcion
    { $$ = $1; }
  ;

/* --- Declarations --- */
declaracion:
    tipo_specifier lista_init_var T_SEMICOLON
    { $$ = make_node(NT_DECLARACION, $1); $$->child->sibling = $2; }
  ;

tipo_specifier:
    T_VOID
    { $$ = make_leaf_int(NT_TIPO, T_VOID); }
  | T_CHAR
    { $$ = make_leaf_int(NT_TIPO, T_CHAR); }
  | T_SHORT
    { $$ = make_leaf_int(NT_TIPO, T_SHORT); }
  | T_INT
    { $$ = make_leaf_int(NT_TIPO, T_INT); }
  | T_LONG
    { $$ = make_leaf_int(NT_TIPO, T_LONG); }
  | T_FLOAT
    { $$ = make_leaf_int(NT_TIPO, T_FLOAT); }
  | T_DOUBLE
    { $$ = make_leaf_int(NT_TIPO, T_DOUBLE); }
  | T_SIGNED
    { $$ = make_leaf_int(NT_TIPO, T_SIGNED); }
  | T_UNSIGNED
    { $$ = make_leaf_int(NT_TIPO, T_UNSIGNED); }
  | T_CONST
    { $$ = make_leaf_int(NT_TIPO, T_CONST); }
  | T_VOLATILE
    { $$ = make_leaf_int(NT_TIPO, T_VOLATILE); }
  | T_STRUCT T_ID
    { $$ = make_leaf_str(NT_TIPO, $2); $$->value.op = T_STRUCT; }
  | T_UNION T_ID
    { $$ = make_leaf_str(NT_TIPO, $2); $$->value.op = T_UNION; }
  | T_ENUM T_ID
    { $$ = make_leaf_str(NT_TIPO, $2); $$->value.op = T_ENUM; }
  | T_TYPEDEF
    { $$ = make_leaf_int(NT_TIPO, T_TYPEDEF); }
  ;

lista_init_var:
    init_var
    { $$ = $1; }
  | lista_init_var T_COMMA init_var
    { $$ = ast_append_sibling($1, $3); }
  ;

init_var:
    var
    { $$ = $1; }
  | var T_ASSIGN expr 
    { $$ = make_op_node(T_ASSIGN, $1, $3); }
  ;

var:
    T_ID
    { $$ = make_leaf_str(NT_VAR, $1); }
  | T_ID T_LBRACKET expr_opcional T_RBRACKET
    { $$ = make_node(NT_ARRAY_DECL, make_leaf_str(NT_VAR, $1)); $$->child->sibling = $3; }
  ;

/* --- Functions --- */
/* Handle all 3 parameter cases explicitly to avoid ambiguity */
funcion:
    tipo_specifier T_ID T_LPAREN T_RPAREN T_LBRACE bloque T_RBRACE
    { $$ = make_node(NT_FUNCION, $1); $$->child->sibling = make_leaf_str(NT_ID, $2); $$->child->sibling->sibling = $6; }
  | tipo_specifier T_ID T_LPAREN T_VOID T_RPAREN T_LBRACE bloque T_RBRACE
    { $$ = make_node(NT_FUNCION, $1); $$->child->sibling = make_leaf_str(NT_ID, $2); $$->child->sibling->sibling = make_leaf_int(NT_TIPO, T_VOID); $$->child->sibling->sibling->sibling = $7; }
  | tipo_specifier T_ID T_LPAREN parametros T_RPAREN T_LBRACE bloque T_RBRACE
    { $$ = make_node(NT_FUNCION, $1); $$->child->sibling = make_leaf_str(NT_ID, $2); $$->child->sibling->sibling = $4; $$->child->sibling->sibling->sibling = $7; }
  ;

parametros:
    parametro
    { $$ = $1; }
  | parametros T_COMMA parametro
    { $$ = ast_append_sibling($1, $3); }
  ;

parametro:
    tipo_specifier T_ID
    { $$ = make_node(NT_PARAMETRO, $1); $$->child->sibling = make_leaf_str(NT_ID, $2); }
  ;

/* --- Blocks and Statements --- */
bloque:
    /* empty */
    { $$ = NULL; }
  | bloque sentencia
    { $$ = ast_append_sibling($1, $2); }
  ;

sentencia:
    declaracion
    { $$ = $1; }
  | if_sent
    { $$ = $1; }
  | while_sent
    { $$ = $1; }
  | do_while_sent
    { $$ = $1; }
  | for_sent
    { $$ = $1; }
  | switch_sent
    { $$ = $1; }
  | T_BREAK T_SEMICOLON
    { $$ = make_node(NT_BREAK, NULL); }
  | T_CONTINUE T_SEMICOLON
    { $$ = make_node(NT_CONTINUE, NULL); }
  | T_RETURN expr_opcional T_SEMICOLON
    { $$ = make_node(NT_RETURN, $2); }
  | T_GOTO T_ID T_SEMICOLON
    { $$ = make_node(NT_GOTO, make_leaf_str(NT_ID, $2)); }
  | T_LBRACE bloque T_RBRACE
    { $$ = make_node(NT_BLOQUE, $2); }
  | T_ID T_COLON sentencia
    { $$ = make_node(NT_ETIQUETA, make_leaf_str(NT_ID, $1)); $$->child->sibling = $3; }
  | T_CASE expr T_COLON sentencia
    { $$ = make_node(NT_CASE, $2); $$->child->sibling = $4; }
  | T_DEFAULT T_COLON sentencia
    { $$ = make_node(NT_DEFAULT, $3); }
  | expr_opcional T_SEMICOLON
    { $$ = make_node(NT_EXPR_SENTENCIA, $1); }
  ;

expr_opcional:
    /* empty */
    { $$ = NULL; }
  | expr
    { $$ = $1; }
  ;

/* --- Control Flow Statements --- */

if_sent:
    T_IF T_LPAREN expr T_RPAREN sentencia %prec T_IFX
    { $$ = make_node(NT_IF, $3); $$->child->sibling = $5; }
  | T_IF T_LPAREN expr T_RPAREN sentencia T_ELSE sentencia
    { $$ = make_node(NT_IF, $3); $$->child->sibling = $5; $$->child->sibling->sibling = $7; }
  ;

while_sent:
    T_WHILE T_LPAREN expr T_RPAREN sentencia
    { $$ = make_node(NT_WHILE, $3); $$->child->sibling = $5; }
  ;

do_while_sent:
    T_DO sentencia T_WHILE T_LPAREN expr T_RPAREN T_SEMICOLON
    { $$ = make_node(NT_DO_WHILE, $5); $$->child->sibling = $2; }
  ;

for_sent:
    T_FOR T_LPAREN expr_opcional T_SEMICOLON expr_opcional T_SEMICOLON expr_opcional T_RPAREN sentencia
    { $$ = make_node(NT_FOR, $3); $$->child->sibling = $5; $$->child->sibling->sibling = $7; $$->child->sibling->sibling->sibling = $9; }
  ;

switch_sent:
    T_SWITCH T_LPAREN expr T_RPAREN sentencia
    { $$ = make_node(NT_SWITCH, $3); $$->child->sibling = $5; }
  ;

/* --- Expressions --- */
expr:
    /* Assignment */
    T_ID T_ASSIGN expr
    { $$ = make_op_node(T_ASSIGN, make_leaf_str(NT_ID, $1), $3); }
  | expr T_ASSIGN_PLUS expr
    { $$ = make_op_node(T_ASSIGN_PLUS, $1, $3); }
  | expr T_ASSIGN_MINUS expr
    { $$ = make_op_node(T_ASSIGN_MINUS, $1, $3); }
  | expr T_ASSIGN_STAR expr
    { $$ = make_op_node(T_ASSIGN_STAR, $1, $3); }
  | expr T_ASSIGN_SLASH expr
    { $$ = make_op_node(T_ASSIGN_SLASH, $1, $3); }
  | expr T_ASSIGN_PERCENT expr
    { $$ = make_op_node(T_ASSIGN_PERCENT, $1, $3); }
  | expr T_ASSIGN_LSHIFT expr
    { $$ = make_op_node(T_ASSIGN_LSHIFT, $1, $3); }
  | expr T_ASSIGN_RSHIFT expr
    { $$ = make_op_node(T_ASSIGN_RSHIFT, $1, $3); }
  | expr T_ASSIGN_AND expr
    { $$ = make_op_node(T_ASSIGN_AND, $1, $3); }
  | expr T_ASSIGN_OR expr
    { $$ = make_op_node(T_ASSIGN_OR, $1, $3); }
  | expr T_ASSIGN_XOR expr
    { $$ = make_op_node(T_ASSIGN_XOR, $1, $3); }

    /* Ternary */
  | expr T_QUESTION expr T_COLON expr
    { $$ = make_node(NT_TERNARIO, $1); $$->child->sibling = $3; $$->child->sibling->sibling = $5; }

    /* Logical and Bitwise */
  | expr T_OR expr
    { $$ = make_op_node(T_OR, $1, $3); }
  | expr T_AND expr
    { $$ = make_op_node(T_AND, $1, $3); }
  | expr T_PIPE expr
    { $$ = make_op_node(T_PIPE, $1, $3); }
  | expr T_CARET expr
    { $$ = make_op_node(T_CARET, $1, $3); }
  | expr T_AMPERSAND expr
    { $$ = make_op_node(T_AMPERSAND, $1, $3); }
  | expr T_EQ expr
    { $$ = make_op_node(T_EQ, $1, $3); }
  | expr T_NEQ expr
    { $$ = make_op_node(T_NEQ, $1, $3); }
  | expr T_LT expr
    { $$ = make_op_node(T_LT, $1, $3); }
  | expr T_LE expr
    { $$ = make_op_node(T_LE, $1, $3); }
  | expr T_GT expr
    { $$ = make_op_node(T_GT, $1, $3); }
  | expr T_GE expr
    { $$ = make_op_node(T_GE, $1, $3); }
  | expr T_LSHIFT expr
    { $$ = make_op_node(T_LSHIFT, $1, $3); }
  | expr T_RSHIFT expr
    { $$ = make_op_node(T_RSHIFT, $1, $3); }

    /* Arithmetic */
  | expr T_PLUS expr
    { $$ = make_op_node(T_PLUS, $1, $3); }
  | expr T_MINUS expr
    { $$ = make_op_node(T_MINUS, $1, $3); }
  | expr T_STAR expr
    { $$ = make_op_node(T_STAR, $1, $3); }
  | expr T_SLASH expr
    { $$ = make_op_node(T_SLASH, $1, $3); }
  | expr T_PERCENT expr
    { $$ = make_op_node(T_PERCENT, $1, $3); }

    /* Unary */
  | T_MINUS expr %prec T_UMINUS
    { $$ = make_unary_op_node(T_MINUS, $2); }
  | T_PLUS expr %prec T_UMINUS
    { $$ = $2; } /* Unary plus is a no-op */
  | T_INC expr
    { $$ = make_unary_op_node(T_INC, $2); }
  | expr T_INC
    { $$ = make_unary_op_node(T_INC, $1); } 
  | T_DEC expr
    { $$ = make_unary_op_node(T_DEC, $2); }
  | expr T_DEC
    { $$ = make_unary_op_node(T_DEC, $1); }
  | T_NOT expr
    { $$ = make_unary_op_node(T_NOT, $2); }
  | T_TILDE expr
    { $$ = make_unary_op_node(T_TILDE, $2); }
  | T_AMPERSAND expr %prec T_UMINUS 
    { $$ = make_unary_op_node(T_AMPERSAND, $2); } /* Address-of */
  | T_STAR expr %prec T_UMINUS 
    { $$ = make_unary_op_node(T_STAR, $2); } /* Dereference */
  | T_SIZEOF expr
    { $$ = make_unary_op_node(T_SIZEOF, $2); }
  | T_SIZEOF T_LPAREN tipo_specifier T_RPAREN
    { $$ = make_unary_op_node(T_SIZEOF, $3); }

    /* Postfix / Access */
  | expr T_LBRACKET expr T_RBRACKET
    { $$ = make_node(NT_ACCESO_ARRAY, $1); $$->child->sibling = $3; }
  | expr T_LPAREN lista_args_opt T_RPAREN
    { $$ = make_node(NT_LLAMADA_FUNCION, $1); $$->child->sibling = $3; }
  | expr T_DOT T_ID
    { $$ = make_node(NT_ACCESO_MIEMBRO, $1); $$->child->sibling = make_leaf_str(NT_ID, $3); $$->value.op = T_DOT; }
  | expr T_ARROW T_ID
    { $$ = make_node(NT_ACCESO_MIEMBRO, $1); $$->child->sibling = make_leaf_str(NT_ID, $3); $$->value.op = T_ARROW; }

    /* Primitives */
  | T_LPAREN expr T_RPAREN
    { $$ = $2; } /* Pass inner node up */
  | T_ID
    { $$ = make_leaf_str(NT_ID, $1); }
  | T_ENTERO
    { $$ = make_leaf_int(NT_ENTERO, $1); }
  | T_NUMERO
    { $$ = make_leaf_float(NT_FLOTANTE, $1); }
  | T_CARACTER
    { $$ = make_leaf_str(NT_CARACTER, $1); }
  | T_CADENA
    { $$ = make_leaf_str(NT_CADENA, $1); }
  ;

lista_args_opt:
    /* empty */
    { $$ = NULL; }
  | lista_args
    { $$ = $1; }
  ;

lista_args:
    expr
    { $$ = $1; }
  | lista_args T_COMMA expr
    { $$ = ast_append_sibling($1, $3); }
  ;

%%
/* ==================================================================
 * SECTION 3: ADDITIONAL C CODE
 * ================================================================== */

/*
 * All AST helper functions (make_node, print_ast, etc.)
 * have been moved to 'ast.c'.
 */

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error in line %d: %s\n", yylineno, s);
}
