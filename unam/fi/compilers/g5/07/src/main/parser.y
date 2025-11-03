/* ==================================================================
 * ARCHIVO: parser.y (Versión Final Completa)
 * ================================================================== */

/* * ------------------------------------------------------------------
 * SECCIÓN 1: DECLARACIONES DE C Y BISON
 * ------------------------------------------------------------------
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//En esta parte se introduce nuestro analizador lexico
extern int yylex();

//Esta es la funcion de error por si el paser falla
void yyerror(const char *s);

/* * Variable global para el Nro de línea.
 * ¡Asegúrate de definirla ('int yylineno = 1;')
 * y actualizarla en tu lexer.c!
 */
//Variable global para indicar el numero de linea
extern int yylineno;
%}

/* * ------------------------------------------------------------------
 * SINDICATO Y TOKENS
 * ------------------------------------------------------------------
 */

/* Define los tipos de datos que pueden tener los símbolos */

//En esta parte se define el tipo de dato que se utiliza en los
//elementos de la pila de Bison
%union {
  int intVal;
  float floatVal;
  char* strVal;
}

//KEYWORDS
//Definimos los diferentes tokens que se puede obtener del lexer que pertenecen a
//los keywords

//Palabras clave para los tipos de datos
%token T_INT T_FLOAT T_CHAR T_VOID T_DOUBLE T_LONG T_SHORT T_SIGNED T_UNSIGNED
%token T_STRUCT T_UNION T_ENUM
%token T_CONST T_VOLATILE
%token T_TYPEDEF

//Palabras clave para las estructuras de control
%token T_IF T_ELSE T_WHILE T_FOR T_DO T_SWITCH T_CASE T_DEFAULT
%token T_BREAK T_CONTINUE T_RETURN T_GOTO

//Otras palabras clave
%token T_AUTO T_REGISTER T_STATIC T_EXTERN
%token T_SIZEOF

//IDENTIFIERS
// La parte entre <> antes del nombre del token es el tipo de dato definido antes en union
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
//operadores de asignacion
// =  &   +=    &     -=      &    *=    &    /=
%token T_ASSIGN T_ASSIGN_PLUS T_ASSIGN_MINUS T_ASSIGN_STAR T_ASSIGN_SLASH

// %=    &    <<=   &   >>=
%token T_ASSIGN_PERCENT T_ASSIGN_LSHIFT T_ASSIGN_RSHIFT

//  &=   &   |=     &    ^=
%token T_ASSIGN_AND T_ASSIGN_OR T_ASSIGN_XOR

//   +    -    *     /     %
%token T_PLUS T_MINUS T_STAR T_SLASH T_PERCENT
%token T_INC T_DEC /* ++ -- */
//   ==   &   !=    &   <    &   <=    &   >    &   >=
%token T_EQ T_NEQ T_LT T_LE T_GT T_GE
%token T_AND T_OR  /* && || */
%token T_AMPERSAND /* & */
%token T_PIPE      /* | */
%token T_CARET     /* ^ */
%token T_NOT       /* ! */
%token T_TILDE     /* ~ */
%token T_LSHIFT T_RSHIFT /* << >> */

/* * ------------------------------------------------------------------
 * PRECEDENCIA Y ASOCIATIVIDAD
 * ------------------------------------------------------------------
 * De MENOR precedencia (arriba) a MAYOR (abajo).
 */

// La sentencia %left indica que la asociatividad es de izquierda a derecha
//La sentencia %right indica que la asociatividad es de derecha a izquierda
 
// La coma es la de menor precedencia
%left T_COMMA
//Despues van las asignaciones
%right T_ASSIGN T_ASSIGN_PLUS T_ASSIGN_MINUS T_ASSIGN_STAR T_ASSIGN_SLASH
%right T_ASSIGN_PERCENT T_ASSIGN_LSHIFT T_ASSIGN_RSHIFT
%right T_ASSIGN_AND T_ASSIGN_OR T_ASSIGN_XOR

//Despues van ?  &  :
%right T_QUESTION T_COLON

//Despues van las sentencias logicas || ,  &&
%left T_OR
%left T_AND

//Despues va   | ,  ^   ,   &   
%left T_PIPE
%left T_CARET
%left T_AMPERSAND

//Despues va ==   !=
%left T_EQ T_NEQ

//Despes va   <   <=   >   >=
%left T_LT T_LE T_GT T_GE

//Despues va    <<   >>
%left T_LSHIFT T_RSHIFT

//Despues va +    -
%left T_PLUS T_MINUS

//Despues va    *    /    %    
%left T_STAR T_SLASH T_PERCENT

//Despues va  ++    --    !      ~    negacion numerica ( -x  )   sizeof
%right T_INC T_DEC T_NOT T_TILDE T_UMINUS T_SIZEOF

//Precedencia para solucionar un dangling else
//nonassoc significa que un token no puede asociarse con otros del mismo tipo
//Al poner esta precedenia donde el else es mayor que el if el else siempre se asociara
//con el if mas cercano
%nonassoc T_IFX
%nonassoc T_ELSE

//Despues sigue    [    ]    (    )     .    ->
%left T_LBRACKET T_RBRACKET T_LPAREN T_RPAREN T_DOT T_ARROW

//Y el que tiene mayor precedencia es el simbolo inicial
%start programa

%%
/* ==================================================================
 * SECCIÓN 2: REGLAS DE LA GRAMÁTICA (BNF)
 * ================================================================== */

programa:
    /* programa vacío */
  | programa declaracion_externa
  ;

declaracion_externa:
    declaracion
  | funcion
  ;

/* --- Declaraciones --- */
declaracion:
    tipo_specifier lista_init_var T_SEMICOLON
  ;

/* * SOLUCIÓN: Regla de tipo SIMPLIFICADA.
 * NO es recursiva, por lo tanto NO es ambigua.
 * No permite 'unsigned long', solo 'unsigned' o 'long'.
 */
tipo_specifier:
    T_VOID
  | T_CHAR
  | T_SHORT
  | T_INT
  | T_LONG
  | T_FLOAT
  | T_DOUBLE
  | T_SIGNED
  | T_UNSIGNED
  | T_CONST
  | T_VOLATILE
  | T_STRUCT T_ID
  | T_UNION T_ID
  | T_ENUM T_ID
  | T_TYPEDEF
  ;

lista_init_var:
    init_var
  | lista_init_var T_COMMA init_var
  ;

init_var:
    var
  | var T_ASSIGN expr /* Inicialización, ej: int x = 5; */
  ;

var:
    T_ID
  | T_ID T_LBRACKET expr_opcional T_RBRACKET
  ;

/* --- Funciones --- */
funcion:
    tipo_specifier T_ID T_LPAREN parametros_opt T_RPAREN T_LBRACE bloque T_RBRACE
  ;

parametros_opt:
    /* vacío */
  | parametros
  ;

parametros:
    parametro
  | parametros T_COMMA parametro
  ;

parametro:
    tipo_specifier T_ID
  ;

/* --- Bloques y Sentencias --- */
bloque:
    /* bloque vacío */
  | bloque sentencia
  ;

sentencia:
    declaracion
  | if_sent
  | while_sent
  | do_while_sent
  | for_sent
  | switch_sent
  | T_BREAK T_SEMICOLON
  | T_CONTINUE T_SEMICOLON
  | T_RETURN expr_opcional T_SEMICOLON
  | T_GOTO T_ID T_SEMICOLON
  | T_LBRACE bloque T_RBRACE
  | T_ID T_COLON sentencia /* Etiqueta para GOTO o CASE */
  | T_CASE expr T_COLON sentencia
  | T_DEFAULT T_COLON sentencia
  | expr_opcional T_SEMICOLON
  ;

expr_opcional:
    /* vacío */
  | expr
  ;

/* --- Sentencias de Control --- */

/* * SOLUCIÓN: Regla 'if' con %prec T_IFX
 * para resolver la ambigüedad del 'dangling else'.
 */
if_sent:
    T_IF T_LPAREN expr T_RPAREN sentencia %prec T_IFX
  | T_IF T_LPAREN expr T_RPAREN sentencia T_ELSE sentencia
  ;

while_sent:
    T_WHILE T_LPAREN expr T_RPAREN sentencia
  ;

do_while_sent:
    T_DO sentencia T_WHILE T_LPAREN expr T_RPAREN T_SEMICOLON
  ;

for_sent:
    T_FOR T_LPAREN expr_opcional T_SEMICOLON expr_opcional T_SEMICOLON expr_opcional T_RPAREN sentencia
  ;

switch_sent:
    T_SWITCH T_LPAREN expr T_RPAREN sentencia
  ;

/* --- Expresiones (Completas) --- */
expr:
    /* Asignación */
    T_ID T_ASSIGN expr
  | expr T_ASSIGN_PLUS expr
  | expr T_ASSIGN_MINUS expr
  | expr T_ASSIGN_STAR expr
  | expr T_ASSIGN_SLASH expr
  | expr T_ASSIGN_PERCENT expr
  | expr T_ASSIGN_LSHIFT expr
  | expr T_ASSIGN_RSHIFT expr
  | expr T_ASSIGN_AND expr
  | expr T_ASSIGN_OR expr
  | expr T_ASSIGN_XOR expr

    /* Ternario */
  | expr T_QUESTION expr T_COLON expr

    /* Lógicos y Bitwise */
  | expr T_OR expr
  | expr T_AND expr
  | expr T_PIPE expr
  | expr T_CARET expr
  | expr T_AMPERSAND expr
  | expr T_EQ expr
  | expr T_NEQ expr
  | expr T_LT expr
  | expr T_LE expr
  | expr T_GT expr
  | expr T_GE expr
  | expr T_LSHIFT expr
  | expr T_RSHIFT expr

    /* Aritméticos */
  | expr T_PLUS expr
  | expr T_MINUS expr
  | expr T_STAR expr
  | expr T_SLASH expr
  | expr T_PERCENT expr

    /* Unarios (con %prec T_UMINUS para - y +) */
  | T_MINUS expr %prec T_UMINUS
  | T_PLUS expr %prec T_UMINUS
  | T_INC expr
  | expr T_INC
  | T_DEC expr
  | expr T_DEC
  | T_NOT expr
  | T_TILDE expr
  | T_AMPERSAND expr /* Dirección de (ej. &x) */
  | T_STAR expr      /* Desreferencia (ej. *p) */
  | T_SIZEOF expr
  | T_SIZEOF T_LPAREN tipo_specifier T_RPAREN

    /* Postfijos / Acceso */
  | expr T_LBRACKET expr T_RBRACKET
  | expr T_LPAREN lista_args_opt T_RPAREN
  | expr T_DOT T_ID
  | expr T_ARROW T_ID

    /* Primitivos */
  | T_LPAREN expr T_RPAREN
  | T_ID
  | T_ENTERO
  | T_NUMERO
  | T_CARACTER
  | T_CADENA
  ;

lista_args_opt:
    /* vacío */
  | lista_args
  ;

lista_args:
    expr
  | lista_args T_COMMA expr
  ;

%%
/* ==================================================================
 * SECCIÓN 3: CÓDIGO C ADICIONAL
 * ================================================================== */

/*
 * Función de error. Bison la llama automáticamente.
 * Usa 'yylineno' (definida en tu lexer.c) para reportar la línea.
 */
void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis en línea %d: %s\n", yylineno, s);
}

/*
 * NO HAY 'main()' AQUÍ.
 * Tu 'main()' está en lexer.c y debe llamar a yyparse().
 */
