#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "parser.tab.h"
#include <assert.h>

typedef struct
{
    const char *start;   // Address of the initial character from the lexeme
    const char *current; // Address of the current character
} Scanner;

Scanner scanner;

int yylineno = 1;

// Initialize the scanner
void initScanner(const char *source_code)
{
    scanner.start = source_code;
    scanner.current = source_code;
}

void saveYYVal() {
    int len = (int)(scanner.current - scanner.start); // Calculate the length of the lexeme
    yylval.strVal = (char *)malloc(len + 1);
    strncpy(yylval.strVal, scanner.start, len);
    yylval.strVal[len] = '\0';
}

// Return a boolean if the lexeme match with the given string
bool matchStr(const char *start, int len, const char *str)
{
    // Verify based on the length
    if (strlen(str) != len)
        return false;

    return memcmp(start, str, len) == 0;
}

// Return an int corresponding to the keyword or identifier
int lookupKeyword(const char *start, const char *end)
{
    int len = (int)(end - start);

    // Decide which keyword is
    switch (start[0])
    {
    case 'a':
        if (matchStr(start, len, "auto"))
            return T_AUTO;
        break;
    case 'b':
        if (matchStr(start, len, "break"))
            return T_BREAK;
        break;
    case 'c':
        if (matchStr(start, len, "case"))
            return T_CASE;
        if (matchStr(start, len, "char"))
            return T_CHAR;
        if (matchStr(start, len, "const"))
            return T_CONST;
        if (matchStr(start, len, "continue"))
            return T_CONTINUE;
        break;
    case 'd':
        if (matchStr(start, len, "default"))
            return T_DEFAULT;
        if (matchStr(start, len, "do"))
            return T_DO;
        if (matchStr(start, len, "double"))
            return T_DOUBLE;
        break;
    case 'e':
        if (matchStr(start, len, "else"))
            return T_ELSE;
        if (matchStr(start, len, "enum"))
            return T_ENUM;
        if (matchStr(start, len, "extern"))
            return T_EXTERN;
        break;
    case 'f':
        if (matchStr(start, len, "float"))
            return T_FLOAT;
        if (matchStr(start, len, "for"))
            return T_FOR;
        break;
    case 'g':
        if (matchStr(start, len, "goto"))
            return T_GOTO;
        break;
    case 'i':
        if (matchStr(start, len, "if"))
            return T_IF;
        if (matchStr(start, len, "int"))
            return T_INT;
        break;
    case 'l':
        if (matchStr(start, len, "long"))
            return T_LONG;
        break;
    case 'r':
        if (matchStr(start, len, "register"))
            return T_REGISTER;
        if (matchStr(start, len, "return"))
            return T_RETURN;
        break;
    case 's':
        if (matchStr(start, len, "short"))
            return T_SHORT;
        if (matchStr(start, len, "signed"))
            return T_SIGNED;
        if (matchStr(start, len, "sizeof"))
            return T_SIZEOF;
        if (matchStr(start, len, "static"))
            return T_STATIC;
        if (matchStr(start, len, "struct"))
            return T_STRUCT;
        if (matchStr(start, len, "switch"))
            return T_SWITCH;
        break;
    case 't':
        if (matchStr(start, len, "typedef"))
            return T_TYPEDEF;
        break;
    case 'u':
        if (matchStr(start, len, "union") ||
            matchStr(start, len, "unsigned"))
            return T_UNSIGNED;
        break;
    case 'v':
        if (matchStr(start, len, "void"))
                return T_VOID;
        if (matchStr(start, len, "volatile"))
            return T_VOLATILE;
        break;
    case 'w':
        if (matchStr(start, len, "while"))
            return T_WHILE;
        break;
    }

    // If is not a keyword, is an identifier
    return T_ID;
}

int lookupPunctuator(const char *c)
{
    switch (c[0])
    {
        // OPERATORS AND PUNCTUATORS
        case '(': 
            scanner.current++; 
            return T_LPAREN;
        case ')':
            scanner.current++; 
            return T_RPAREN;
        case '{':
            scanner.current++; 
            return T_LBRACE;
        case '}': 
            scanner.current++;
            return T_RBRACE;
        case '[': 
            scanner.current++;
            return T_LBRACKET;
        case ']': 
            scanner.current++;
            return T_RBRACKET;
        case ';': 
            scanner.current++;
            return T_SEMICOLON;
        case ',': 
            scanner.current++;
            return T_COMMA;
        case ':': 
            scanner.current++;
            return T_COLON;
        case '?': 
            scanner.current++;
            return T_QUESTION;
        case '.': 
            scanner.current++;
            return T_DOT;
        case '=':
            scanner.current++;
            if (*scanner.current == '=') 
            {
                scanner.current++; 
                return T_EQ; 
            }
            return T_ASSIGN;
        case '!':
            scanner.current++;
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_NEQ;
            }
            return T_NOT;
        case '+':
            scanner.current++;
            if (*scanner.current == '+')
            { 
                scanner.current++;
                return T_INC;
            }
            if (*scanner.current == '=')
            { 
                scanner.current++;
                return T_ASSIGN_PLUS;
            }
            return T_PLUS;
        case '-':
            scanner.current++;
            if (*scanner.current == '-')
            {
                scanner.current++;
                return T_DEC;
            }
            if (*scanner.current == '=')
            { 
                scanner.current++;
                return T_ASSIGN_MINUS;
            }
            if (*scanner.current == '>')
            {
                scanner.current++;
                return T_ARROW;
            }
            return T_MINUS;
        case '*':
            scanner.current++;
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_ASSIGN_STAR;
            }
            return T_STAR;
        case '/':
            scanner.current++;
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_ASSIGN_SLASH;
            }
            return T_SLASH;
        case '%':
            scanner.current++;
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_ASSIGN_PERCENT;
            }
            return T_PERCENT;
        case '&':
            scanner.current++;
            if (*scanner.current == '&')
            {
                scanner.current++;
                return T_AND;
            }
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_ASSIGN_AND;
            }
            return T_AMPERSAND;
        case '|':
            scanner.current++;
            if (*scanner.current == '|')
            {
                scanner.current++;
                return T_OR;
            }
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_ASSIGN_OR;
            }
            return T_PIPE;
        case '^':
            scanner.current++;
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_ASSIGN_XOR;
            }
            return T_CARET;
        case '<':
            scanner.current++;
            if (*scanner.current == '<')
            {
                scanner.current++;
                if (*scanner.current == '=')
                {
                    scanner.current++;
                    return T_ASSIGN_LSHIFT;
                }
                return T_LSHIFT;
            }
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_LE;
            }
            return T_LT;
        case '>':
            scanner.current++;
            if (*scanner.current == '>')
            {
                scanner.current++;
                if (*scanner.current == '=')
                {
                    scanner.current++;
                    return T_ASSIGN_RSHIFT;
                }
                return T_RSHIFT;
            }
            if (*scanner.current == '=')
            {
                scanner.current++;
                return T_GE;
            }
            return T_GT;
    }
    return -1; // ERROR
}

// Skipping whitespaces and linebreak
void skipWhitespaces()
{
    while (*scanner.current == ' ' ||
           *scanner.current == '\t' ||
           *scanner.current == '\n')
    {
        if (*scanner.current == '\n')
            yylineno++;
        scanner.current++;
    }
        
}

int yylex()
{
    while(true)
    {
        skipWhitespaces();
        scanner.start = scanner.current;

        // Skipping macros
        if (*scanner.current == '#')
        {
            while (*scanner.current != '\n' && *scanner.current != '\0')
                scanner.current++;
            continue;
        }

        // Skipping comments
        if (*scanner.current == '/' && *(scanner.current + 1) == '/')
        {
            while (*scanner.current != '\n' && *scanner.current != '\0')
                scanner.current++;
            continue;
        }
        else if (*scanner.current == '/' && *(scanner.current + 1) == '*')
        {
            scanner.current += 2; // jumps the /*
            while (!(*scanner.current == '*' && *(scanner.current + 1) == '/'))
            {
                scanner.current++;
                if (*scanner.current == '\n')
                    yylineno++;
                if (*scanner.current == '\0')
                    return YYEOF;
            }
            scanner.current += 2;//jumps the */
            continue;
        }

        // Detects the EOF
        if (*scanner.current == '\0')
            return YYEOF;

        char c = *scanner.start; // Stores the first character

        // LITERALS
        if (c == '"')
        {
            scanner.current++;
            while (*scanner.current != '"' && *scanner.current != '\0')
            {
                if (*scanner.current == '\n') // Multiline strings
                    yylineno++;
                scanner.current++;
            }
            scanner.current++; //gets the runaway "
            saveYYVal();
            return T_CADENA;
        }
        if (c == '\'')
        {
            scanner.current++;
            while (*scanner.current != '\'' && *scanner.current != '\0')
                scanner.current++;
            scanner.current++; //same as string, gets the runaway '
            saveYYVal();
            return T_CARACTER;
        }

        // CONSTANTS
        if (isdigit(c))
        {
            int e_consumed = 0, dot_consumed = 0;
            while (isdigit(*scanner.current) || *scanner.current == '.' || *scanner.current == 'e' || *scanner.current == 'E')
            {
                 if (*scanner.current == '.')
                 {
                        dot_consumed++;
                        if(e_consumed > 0) dot_consumed++; 
                 } 
                 if (*scanner.current == 'e' || *scanner.current == 'E')
                 {
                     e_consumed++;
                     if (*(scanner.current + 1) == '+' || *(scanner.current + 1) == '-')
                     {
                         scanner.current++;
                     }
                 }
                 scanner.current++;
            }
            
            int len = (int)(scanner.current - scanner.start);
            char *lexeme = (char *)malloc(len + 1);
            strncpy(lexeme, scanner.start, len);
            lexeme[len] = '\0';

            if (dot_consumed == 0 && e_consumed == 0)
            {
                yylval.intVal = atoi(lexeme);
                free(lexeme);
                return T_ENTERO;
            }
            else if(dot_consumed <=1 && e_consumed <=1)
            {
                yylval.floatVal = atof(lexeme);
                free(lexeme);
                return T_NUMERO;
            }
            else
            {
                printf("(LEXICAL ERROR): in line %d: malformed number '%s'\n", yylineno, lexeme);
                free(lexeme);
                return YYEOF;
            }
        }

        // KEYWORDS and IDENTIFIERS
        if (isalpha(c) || c == '_')
        { // If matches with the initial char of a keyword or identifier
            while (isalnum(*scanner.current) || *scanner.current == '_')
            { // Traverse the lexeme
                scanner.current++;
            }
            int type = lookupKeyword(scanner.start, scanner.current);
            if (type == T_ID)
                saveYYVal();
            return type;
        }

        // PUNCTUATORS and OPERATORS
        if (!isalnum(c))
        {
            int type = lookupPunctuator(scanner.start);
            return type;
        }

        // Unclassified lexeme
        printf("(LEXICAL ERROR): in line %d: no recognized '%c'", yylineno, *scanner.start);
        scanner.current++; // Move to the next character
        continue;
    }
}

// Read an entire file and return the content as a string
char *readFile(const char *source_file_path)
{
    FILE *f = fopen(source_file_path, "rb");
    if (!f)
    {
        printf("ERROR: The file `%s` doesn't exists.\n", source_file_path);
        exit(74);
    }

    // Find the file lenght
    fseek(f, 0L, SEEK_END);
    size_t fileSize = ftell(f);
    rewind(f);

    // Allocate the memory for the file content
    char *buffer = (char *)malloc(fileSize+1);
    fread(buffer, sizeof(char), fileSize, f);
    buffer[fileSize] = '\0';

    fclose(f);
    return buffer;
}
