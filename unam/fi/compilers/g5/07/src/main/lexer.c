#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "token.h"

typedef struct
{
    const char *start;   // Address of the initial character from the lexeme
    const char *current; // Address of the current character
} Scanner;

Scanner scanner;

// Return the string corresponding to the TokenType
char *getTokenTypeString(int token_index)
{
    char *token_map[8] = {"KEYWORD",
                          "IDENTIFIER",
                          "PUNCTUATOR",
                          "OPERATOR",
                          "CONST",
                          "LITERAL",
                          "END_OF_FILE",
                          "NOT_A_TOKEN"};
    return token_map[token_index];
}

// Initialize the scanner
void initScanner(const char *source_code)
{
    scanner.start = source_code;
    scanner.current = source_code;
}

// Return a token of the given type
token createToken(TokenType type)
{
    token t;
    t.type = type;

    int len = (int)(scanner.current - scanner.start); // Calculate the length of the lexeme
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, scanner.start, len); // Copy the lexeme
    lexeme[len] = '\0';                  // Add the end of string character

    t.lexeme = lexeme;
    return t;
}

// Return a boolean if the lexeme match with the given string
bool matchStr(const char *start, int len, const char *str)
{
    // Verify based on the length
    if (strlen(str) != len)
        return false;

    return memcmp(start, str, len) == 0;
}

// Return a TokenType: KEYWORD or a IDENTIFIER
TokenType lookupKeyword(const char *start, const char *end)
{
    int len = (int)(end - start);

    // Decide which keyword is
    switch (start[0])
    {
    case 'a':
        if (matchStr(start, len, "auto"))
            return KEYWORD;
        break;
    case 'b':
        if (matchStr(start, len, "break"))
            return KEYWORD;
        break;
    case 'c':
        if (matchStr(start, len, "case") ||
            matchStr(start, len, "char") ||
            matchStr(start, len, "const") ||
            matchStr(start, len, "continue"))
            return KEYWORD;
        break;
    case 'd':
        if (matchStr(start, len, "default") ||
            matchStr(start, len, "do") ||
            matchStr(start, len, "double"))
            return KEYWORD;
        break;
    case 'e':
        if (matchStr(start, len, "else") ||
            matchStr(start, len, "enum") ||
            matchStr(start, len, "extern"))
            return KEYWORD;
        break;
    case 'f':
        if (matchStr(start, len, "float") ||
            matchStr(start, len, "for"))
            return KEYWORD;
        break;
    case 'g':
        if (matchStr(start, len, "goto"))
            return KEYWORD;
        break;
    case 'i':
        if (matchStr(start, len, "if") ||
            matchStr(start, len, "int"))
            return KEYWORD;
        break;
    case 'l':
        if (matchStr(start, len, "long"))
            return KEYWORD;
        break;
    case 'r':
        if (matchStr(start, len, "register") ||
            matchStr(start, len, "return"))
            return KEYWORD;
        break;
    case 's':
        if (matchStr(start, len, "short") ||
            matchStr(start, len, "signed") ||
            matchStr(start, len, "sizeof") ||
            matchStr(start, len, "static") ||
            matchStr(start, len, "struct") ||
            matchStr(start, len, "switch"))
            return KEYWORD;
        break;
    case 't':
        if (matchStr(start, len, "typedef"))
            return KEYWORD;
        break;
    case 'u':
        if (matchStr(start, len, "union") ||
            matchStr(start, len, "unsigned"))
            return KEYWORD;
        break;
    case 'v':
        if (matchStr(start, len, "void") ||
            matchStr(start, len, "volatile"))
            return KEYWORD;
        break;
    case 'w':
        if (matchStr(start, len, "while"))
            return KEYWORD;
        break;
    }

    // If is not a keyword, is an identifier
    return IDENTIFIER;
}

TokenType lookupPunctuator(const char *c)
{
    TokenType type = NOT_A_TOKEN;
    switch (c[0])
    {
    // PUNCTUATORS
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
    case ',':
    case ':':
    case '?':
        type = PUNCTUATOR;
        break;
    // OPERATORS applying lookeaheads
    case '=':
        if (c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '+':
        if (c[1] == '+' || c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '-':
        if (c[1] == '-' || c[1] == '=' || c[1] == '>')
            scanner.current++;
        type = OPERATOR;
        break;
    case '*':
        if (c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '/':
        if (c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '%':
        if (c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '&':
        if (c[1] == '&' || c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '|':
        if (c[1] == '|' || c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '!':
        if (c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '^':
        if (c[1] == '=')
            scanner.current++;
        type = OPERATOR;
        break;
    case '~':
        type = OPERATOR;
        break;
    case '<':
        if (c[1] == '<' || c[1] == '=')
        {
            scanner.current++;
            if (c[1] == '<' && c[2] == '=')
                scanner.current++;
        }
        type = OPERATOR;
        break;
    case '>':
        if (c[1] == '>' || c[1] == '=')
        {
            scanner.current++;
            if (c[1] == '>' && c[2] == '=')
                scanner.current++;
        }
        type = OPERATOR;
        break;
    case '.':
        type = OPERATOR;
        break;
    }
    return type;
}

// Skipping whitespaces and linebreak
void skipWhitespaces()
{
    while (*scanner.current == ' ' ||
           *scanner.current == '\t' ||
           *scanner.current == '\n')
        scanner.current++;
}

token classifyToken()
{
    skipWhitespaces();
    scanner.start = scanner.current;

    // Skipping macros
    if (*scanner.current == '#')
    {
        // printf("MACRO: ");
        while (*scanner.current != '\n')
            scanner.current++;
        return createToken(NOT_A_TOKEN);
    }

    // Skipping comments
    if (*scanner.current == '/' && *(scanner.current + 1) == '/')
    {
        // printf("COMMENT: ");
        while (*scanner.current != '\n')
            scanner.current++;
        return createToken(NOT_A_TOKEN);
    }
    else if (*scanner.current == '/' && *(scanner.current + 1) == '*')
    {
        scanner.current += 2; // jumps the /*
        while (!(*scanner.current == '*' && *(scanner.current + 1) == '/'))
        {
            scanner.current++;
        }
        scanner.current += 2;//jumps the */
        return createToken(NOT_A_TOKEN);
    }

    // Detects the EOF
    if (*scanner.current == '\0')
    {
        token t = createToken(END_OF_FILE);
        return t;
    }

    char c = *scanner.start; // Stores the first character
    scanner.current++;

    // LITERALS
    if (c == '"')
    {
        while (*scanner.current != '"')
        {
            scanner.current++;
        }
        scanner.current++; //gets the runaway "
        return createToken(LITERAL);
    }
    if (c == '\'')
    {
        while (*scanner.current != '\'')
        {
            scanner.current++;
        }
        scanner.current++; //same as string, gets the runaway '
        return createToken(LITERAL);
    }

    // CONSTANTS
    if (isdigit(c))
    {
        int e_consumed = 0, dot_consumed = 0;
        while (isdigit(*scanner.current) || *scanner.current == '.' || *scanner.current == 'e' || *scanner.current == 'E')
        {
            if (*scanner.current == '.')
            {
                dot_consumed++; //Deletion of extra logic helps to make easier to understand
                //The flag now grows bigger and this lets the scanner to get to the end of the string
                if(e_consumed != 0) //Checks if is there an 'e' already in the string
                //If there is, now the flag will get out of bounds in cases like 1e1.2 
                    dot_consumed++;
                
            }

            if (*scanner.current == 'e' || *scanner.current == 'E')
            {
                e_consumed++; //Same as the dot case
                //Since the expression can have an explicit positive or negative, it will be jumped anyways
                if (*(scanner.current + 1) == '+' || *(scanner.current + 1) == '-')
                {
                    scanner.current++;
                }
            }
            scanner.current++;
        }
        if (e_consumed > 1 || dot_consumed > 1) //Minimization of logic here
            return createToken(NOT_A_TOKEN);
        return createToken(CONSTANT);
    }

    // KEYWORDS and IDENTIFIERS
    if (isalpha(c) || c == '_')
    { // If matches with the initial char of a keyword or identifier
        while (isalnum(*scanner.current) || *scanner.current == '_')
        { // Traverse the lexeme
            scanner.current++;
        }
        TokenType type = lookupKeyword(scanner.start, scanner.current);
        return createToken(type);
    }

    // PUNCTUATORS and OPERATORS
    if (!isalnum(c))
    {
        TokenType type = lookupPunctuator(scanner.start);
        return createToken(type);
    }

    // Unclassified lexeme
    return createToken(NOT_A_TOKEN);
}

// Check every token in the file
void lexer(char *source)
{
    initScanner(source);

    int token_counter = 0;
    // While the current character is not the EOF
    while (*scanner.current != '\0')
    {
        token t = classifyToken();

        // If the EOF is reached
        if (t.type == END_OF_FILE) //This is never reached,
        {
            free(t.lexeme); // Free the last lexeme
            break;
        }
        else if (t.type != NOT_A_TOKEN)
        {
            token_counter++;
            printf("%s('%s')\n", getTokenTypeString(t.type), t.lexeme);
        }

        free(t.lexeme); // Free each lexeme
    }
    printf("\nTOTAL: %i\n", token_counter);
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
    char *buffer = (char *)malloc(fileSize);

    fclose(f);
    return buffer;
}

/*
    Arguments: <source_file_path> | <-s source_str>
    Examples of execution:
    ./lexer path/to/program.c
    ./lexer -s 'printf("Hello World!");'
*/
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("ERROR: Please specify a file or a string to analize.");
        return 1;
    }

    if (argc == 2)
    { // A source file path is received
        char *HLL_code = readFile(argv[1]);
        lexer(HLL_code);
    }
    else if (argc == 3 && strcmp(argv[1], "-s") == 0)
    { // A string is received
        int len = strlen(argv[2]);
        char *HLL_code = (char *)malloc(len * sizeof(char));
        strcpy(HLL_code, argv[2]);
        lexer(HLL_code);
    }

    return 0;
}
