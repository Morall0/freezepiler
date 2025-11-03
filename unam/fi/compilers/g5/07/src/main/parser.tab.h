/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_MAIN_PARSER_TAB_H_INCLUDED
# define YY_YY_SRC_MAIN_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_INT = 258,                   /* T_INT  */
    T_FLOAT = 259,                 /* T_FLOAT  */
    T_CHAR = 260,                  /* T_CHAR  */
    T_VOID = 261,                  /* T_VOID  */
    T_DOUBLE = 262,                /* T_DOUBLE  */
    T_LONG = 263,                  /* T_LONG  */
    T_SHORT = 264,                 /* T_SHORT  */
    T_SIGNED = 265,                /* T_SIGNED  */
    T_UNSIGNED = 266,              /* T_UNSIGNED  */
    T_STRUCT = 267,                /* T_STRUCT  */
    T_UNION = 268,                 /* T_UNION  */
    T_ENUM = 269,                  /* T_ENUM  */
    T_CONST = 270,                 /* T_CONST  */
    T_VOLATILE = 271,              /* T_VOLATILE  */
    T_TYPEDEF = 272,               /* T_TYPEDEF  */
    T_IF = 273,                    /* T_IF  */
    T_ELSE = 274,                  /* T_ELSE  */
    T_WHILE = 275,                 /* T_WHILE  */
    T_FOR = 276,                   /* T_FOR  */
    T_DO = 277,                    /* T_DO  */
    T_SWITCH = 278,                /* T_SWITCH  */
    T_CASE = 279,                  /* T_CASE  */
    T_DEFAULT = 280,               /* T_DEFAULT  */
    T_BREAK = 281,                 /* T_BREAK  */
    T_CONTINUE = 282,              /* T_CONTINUE  */
    T_RETURN = 283,                /* T_RETURN  */
    T_GOTO = 284,                  /* T_GOTO  */
    T_AUTO = 285,                  /* T_AUTO  */
    T_REGISTER = 286,              /* T_REGISTER  */
    T_STATIC = 287,                /* T_STATIC  */
    T_EXTERN = 288,                /* T_EXTERN  */
    T_SIZEOF = 289,                /* T_SIZEOF  */
    T_ID = 290,                    /* T_ID  */
    T_ENTERO = 291,                /* T_ENTERO  */
    T_NUMERO = 292,                /* T_NUMERO  */
    T_CADENA = 293,                /* T_CADENA  */
    T_CARACTER = 294,              /* T_CARACTER  */
    T_LPAREN = 295,                /* T_LPAREN  */
    T_RPAREN = 296,                /* T_RPAREN  */
    T_LBRACE = 297,                /* T_LBRACE  */
    T_RBRACE = 298,                /* T_RBRACE  */
    T_LBRACKET = 299,              /* T_LBRACKET  */
    T_RBRACKET = 300,              /* T_RBRACKET  */
    T_SEMICOLON = 301,             /* T_SEMICOLON  */
    T_COMMA = 302,                 /* T_COMMA  */
    T_DOT = 303,                   /* T_DOT  */
    T_ARROW = 304,                 /* T_ARROW  */
    T_QUESTION = 305,              /* T_QUESTION  */
    T_COLON = 306,                 /* T_COLON  */
    T_ASSIGN = 307,                /* T_ASSIGN  */
    T_ASSIGN_PLUS = 308,           /* T_ASSIGN_PLUS  */
    T_ASSIGN_MINUS = 309,          /* T_ASSIGN_MINUS  */
    T_ASSIGN_STAR = 310,           /* T_ASSIGN_STAR  */
    T_ASSIGN_SLASH = 311,          /* T_ASSIGN_SLASH  */
    T_ASSIGN_PERCENT = 312,        /* T_ASSIGN_PERCENT  */
    T_ASSIGN_LSHIFT = 313,         /* T_ASSIGN_LSHIFT  */
    T_ASSIGN_RSHIFT = 314,         /* T_ASSIGN_RSHIFT  */
    T_ASSIGN_AND = 315,            /* T_ASSIGN_AND  */
    T_ASSIGN_OR = 316,             /* T_ASSIGN_OR  */
    T_ASSIGN_XOR = 317,            /* T_ASSIGN_XOR  */
    T_PLUS = 318,                  /* T_PLUS  */
    T_MINUS = 319,                 /* T_MINUS  */
    T_STAR = 320,                  /* T_STAR  */
    T_SLASH = 321,                 /* T_SLASH  */
    T_PERCENT = 322,               /* T_PERCENT  */
    T_INC = 323,                   /* T_INC  */
    T_DEC = 324,                   /* T_DEC  */
    T_EQ = 325,                    /* T_EQ  */
    T_NEQ = 326,                   /* T_NEQ  */
    T_LT = 327,                    /* T_LT  */
    T_LE = 328,                    /* T_LE  */
    T_GT = 329,                    /* T_GT  */
    T_GE = 330,                    /* T_GE  */
    T_AND = 331,                   /* T_AND  */
    T_OR = 332,                    /* T_OR  */
    T_AMPERSAND = 333,             /* T_AMPERSAND  */
    T_PIPE = 334,                  /* T_PIPE  */
    T_CARET = 335,                 /* T_CARET  */
    T_NOT = 336,                   /* T_NOT  */
    T_TILDE = 337,                 /* T_TILDE  */
    T_LSHIFT = 338,                /* T_LSHIFT  */
    T_RSHIFT = 339,                /* T_RSHIFT  */
    T_UMINUS = 340,                /* T_UMINUS  */
    T_IFX = 341                    /* T_IFX  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 37 "src/main/parser.y"

  int intVal;
  float floatVal;
  char* strVal;

#line 156 "src/main/parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_SRC_MAIN_PARSER_TAB_H_INCLUDED  */
