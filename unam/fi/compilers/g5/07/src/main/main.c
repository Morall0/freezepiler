#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "parser.tab.h"


/*
    Arguments: <source_file_path> | <-s source_str>
    Examples of execution:
    ./lexer path/to/program.c
    ./lexer -s 'printf("Hello World!");'
*/
int main(int argc, char *argv[])
{
    int extras = 0;
    char *HLL_code = NULL;
    if (argc < 2)
    {
        printf("ERROR: Please specify a file or a string to analize.\n");
        return 1;
    }

    if (argc == 2 || (argc == 3 && strcmp(argv[2], "-v") == 0)  ) // A source file path is received
    {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Error al abrir el archivo, verifique la ruta");
            return 1;
            // Aquí puedes salir, lanzar error o manejarlo como quieras
        } else {
            // Archivo abierto correctamente
            fclose(file);
        }
        if(argc == 3){
            extras = 1;
        }
        HLL_code = readFile(argv[1]);
    }
    else if ((argc == 3 && strcmp(argv[1], "-s") == 0) ||
        (argc == 4 && strcmp(argv[1], "-s") == 0 && strcmp(argv[3], "-v") == 0))
    { // A string is received
        int len = strlen(argv[2]);
        HLL_code = (char *)malloc(len * sizeof(char) + 1);
        strcpy(HLL_code, argv[2]);
        if(argc == 4){
            extras = 1;
        }
    }


    initScanner(HLL_code);

    int parse_result = yyparse(); // It takes the tokens from lexer (yylex())

    if (parse_result != 0) 
    {
        printf("Parsing error...\n");
        printf("SDT error...\n");
    }
    else 
    {
        printf("Parsing Success!\n");
        int sdt_result = validate_sdt(ast_root);

        if (sdt_result == 1){
            printf("SDT Verified!\n");
            if(extras ==1){
                print_ast(ast_root, 0);
                printf("TOKENS TOTALES DEL LEXER: %d\n", token_count );
            }
        }
        else
            printf("SDT error...\n");
    }

    /*
    printf("--- Abstract Syntax Tree (AST) ---\n");
    print_ast(ast_root, 0);
    printf("----------------------------------\n");
    */

    free(HLL_code);
    return parse_result;
}
