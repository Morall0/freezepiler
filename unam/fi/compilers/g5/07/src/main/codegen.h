#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// Genera el módulo LLVM desde el AST raíz
int codegen_generate_module(ast_node *root, const char *filename);

#endif
