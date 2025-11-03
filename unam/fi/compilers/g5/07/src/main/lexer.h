#ifndef LEXER_H 
#define LEXER_H

void initScanner(const char *source_code);
char *readFile(const char *source_file_path);
int yylex();

#endif
