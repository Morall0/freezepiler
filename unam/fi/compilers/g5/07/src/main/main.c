#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "parser.tab.h"
#include "codegen.h"
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>

/*
Arguments: <source_file_path> | <-s source_str>
Examples of execution:
./lexer path/to/program.c
./lexer -s 'printf("Hello World!");'
*/
int main(int argc, char *argv[])
{
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
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
            printf("ERROR: Unable to open file %s", argv[1]);
            return 1;
        } else {
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
        if(argc == 4)
        {
            extras = 1;
        }
    }

    initScanner(HLL_code);

    int parse_result = yyparse(); // It takes the tokens from lexer (yylex())

    if (parse_result != 0) 
    {
        printf("ERROR: Parsing error...\n");
        //printf("SDT error...\n");
        return parse_result;
    }
    else 
    {
        //printf("Parsing Success!\n");
        int sdt_result = validate_sdt(ast_root);

        if (sdt_result == 1)
        {
            //printf("SDT Verified!\n");
            if(extras ==1)
            {
                print_ast(ast_root, 0);
                printf("Total number of tokens: %d\n", token_count );
            }
        }
        else
            printf("ERROR: SDT error...\n");
    }

    /*
       printf("--- Abstract Syntax Tree (AST) ---\n");
       print_ast(ast_root, 0);
       printf("----------------------------------\n");
       */

    if (codegen_generate_module(ast_root, "out.o") != 0)
    {
        fprintf(stderr, "ERROR: Object Code generation error...\n");
        return 1;
    }
    if (extras == 1) 
    {
        //printf("==========================================================================================================================");
        printf("OK: out.o succesfully generated (Object Code).\n");
    }

    char linker[512], crt1[512], crti[512], crtn[512];
    FILE *fp;

    // Obtener rutas usando gcc -print-file-name
    fp = popen("gcc -print-file-name=ld-linux-x86-64.so.2", "r");
    fgets(linker, sizeof(linker), fp); fclose(fp);

    fp = popen("gcc -print-file-name=crt1.o", "r");
    fgets(crt1, sizeof(crt1), fp); fclose(fp);

    fp = popen("gcc -print-file-name=crti.o", "r");
    fgets(crti, sizeof(crti), fp); fclose(fp);

    fp = popen("gcc -print-file-name=crtn.o", "r");
    fgets(crtn, sizeof(crtn), fp); fclose(fp);

    // Quitar saltos de línea
    linker[strcspn(linker, "\n")] = 0;
    crt1[strcspn(crt1, "\n")] = 0;
    crti[strcspn(crti, "\n")] = 0;
    crtn[strcspn(crtn, "\n")] = 0;

    // Crear el comando ld dinámicamente
    char *ld_command = malloc(2048);
    if (!ld_command) 
    { 
        perror("malloc");
        return 1; 
    }

    snprintf(ld_command, 2048,
            "ld -dynamic-linker %s %s %s out.o -lc %s -o program",
            linker, crt1, crti, crtn);

    //printf("Comando ld generado:\n%s\n", ld_command);
    if (extras == 1)
    {
        printf("INFO: Linking executable (program) with ld...\n");
    }
    if (system(ld_command) != 0)
    {
        fprintf(stderr, "ERROR: ld failed. Verify ld_command paths and look for it in PATH env variable.\n");
        return 1;
    }

    if (extras == 1)
    {
        //printf("==========================================================================================================================");
        printf("OK: Compilation finished. Program 'program' generated.\n");
        printf("Executing program ...\n");
        system("./program");
    }
    //system("rm out.o");

    free(ld_command);
    free(HLL_code);
    return 0;
}
