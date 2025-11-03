#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.tab.h"

/*
    Arguments: <source_file_path> | <-s source_str>
    Examples of execution:
    ./lexer path/to/program.c
    ./lexer -s 'printf("Hello World!");'
*/
int main(int argc, char *argv[])
{
    char *HLL_code = NULL;
    if (argc < 2)
    {
        printf("ERROR: Please specify a file or a string to analize.\n");
        return 1;
    }

    if (argc == 2) // A source file path is received
    {
        HLL_code = readFile(argv[1]);
    }
    else if (argc == 3 && strcmp(argv[1], "-s") == 0)
    { // A string is received
        int len = strlen(argv[2]);
        HLL_code = (char *)malloc(len * sizeof(char) + 1);
        strcpy(HLL_code, argv[2]);
    }

    initScanner(HLL_code);

    int result = yyparse();

    if (result == 0)
        printf("Success\n");
    else
        printf("Fail\n");

    free(HLL_code);
    return result;
}
