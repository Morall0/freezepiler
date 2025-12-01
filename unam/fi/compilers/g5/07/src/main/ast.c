#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

// Global AST root definition
struct ast_node *ast_root = NULL;
extern int yylineno;


struct ast_node *make_node(NodeType type, struct ast_node *child) {
    struct ast_node *node = (struct ast_node*) malloc(sizeof(struct ast_node));
    if (node == NULL) {
        fprintf(stderr, "Fatal Error: malloc failed creating AST node\n");
        exit(1);
    }
    node->type = type;
    node->child = child;
    node->sibling = NULL;
    node->lineno = yylineno;
    return node;
}

struct ast_node *make_op_node(int op, struct ast_node *left, struct ast_node *right) {
    struct ast_node *node = make_node(NT_OP_BINARIO, left);
    node->value.op = op;
    node->child->sibling = right;
    return node;
}

struct ast_node *make_unary_op_node(int op, struct ast_node *operand) {
    struct ast_node *node = make_node(NT_OP_UNARIO, operand);
    node->value.op = op;
    return node;
}

struct ast_node *make_leaf_int(NodeType type, int val) {
    struct ast_node *node = make_node(type, NULL);
    node->value.intVal = val;
    return node;
}

struct ast_node *make_leaf_float(NodeType type, float val) {
    struct ast_node *node = make_node(type, NULL);
    node->value.floatVal = val;
    return node;
}

struct ast_node *make_leaf_str(NodeType type, char *str) {
    struct ast_node *node = make_node(type, NULL);
    node->value.strVal = str; 
    return node;
}

struct ast_node *ast_append_sibling(struct ast_node *list_head, struct ast_node *new_sibling) {
    if (list_head == NULL) {
        return new_sibling; // List was empty
    }
    struct ast_node *curr = list_head;
    while (curr->sibling != NULL) {
        curr = curr->sibling;
    }
    curr->sibling = new_sibling;
    return list_head;
}

/*
Semantic (SDT) Validation
This function is called by main.c after a successful parse.
Returns 1 if semantics are valid.
Returns 0 if a semantic error is found.
 */
int validate_sdt(struct ast_node *root) {
    //printf("--- (Running SDT/Semantic Validation) ---\n");
    
    /* * STUB: This is where Symbol Table logic, type checking, etc. goes.
     * We assume success for now.
     * To test the "SDT error..." output, change this to 'return 0;'.
     */
    
    if (root == NULL) {
        // An empty program is semantically valid.
    }

    return 1; // 1 = Success (SDT Verified!)
}

// Helper to convert node types to strings for printing */
char* node_type_to_string(NodeType type) {
    switch (type) {
        case NT_PROGRAMA: return "PROGRAMA";
        case NT_FUNCION: return "FUNCION";
        case NT_PARAMETRO: return "PARAMETRO";
        case NT_DECLARACION: return "DECLARACION";
        case NT_TIPO: return "TIPO";
        case NT_VAR: return "VAR";
        case NT_ARRAY_DECL: return "ARRAY_DECL";
        case NT_BLOQUE: return "BLOQUE";
        case NT_IF: return "IF";
        case NT_WHILE: return "WHILE";
        case NT_DO_WHILE: return "DO_WHILE";
        case NT_FOR: return "FOR";
        case NT_SWITCH: return "SWITCH";
        case NT_CASE: return "CASE";
        case NT_DEFAULT: return "DEFAULT";
        case NT_BREAK: return "BREAK";
        case NT_CONTINUE: return "CONTINUE";
        case NT_RETURN: return "RETURN";
        case NT_GOTO: return "GOTO";
        case NT_ETIQUETA: return "ETIQUETA";
        case NT_EXPR_SENTENCIA: return "EXPR_SENTENCIA";
        case NT_ASIGNACION: return "ASIGNACION";
        case NT_OP_BINARIO: return "OP_BINARIO";
        case NT_OP_UNARIO: return "OP_UNARIO";
        case NT_TERNARIO: return "TERNARIO";
        case NT_LLAMADA_FUNCION: return "LLAMADA_FUNCION";
        case NT_ACCESO_ARRAY: return "ACCESO_ARRAY";
        case NT_ACCESO_MIEMBRO: return "ACCESO_MIEMBRO";
        case NT_ID: return "ID";
        case NT_ENTERO: return "ENTERO";
        case NT_FLOTANTE: return "FLOTANTE";
        case NT_CADENA: return "CADENA";
        case NT_CARACTER: return "CARACTER";
        default: return "DESCONOCIDO";
    }
}

void print_ast(struct ast_node *node, int indent) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("%s", node_type_to_string(node->type));

    switch (node->type) {
        case NT_ID:
        case NT_VAR:
        case NT_CADENA:
        case NT_CARACTER:
            printf(": %s\n", node->value.strVal);
            break;
        case NT_ENTERO:
            printf(": %d\n", node->value.intVal);
            break;
        case NT_FLOTANTE:
            printf(": %f\n", node->value.floatVal);
            break;
        case NT_TIPO:
            printf(": (Type Token %d)\n", node->value.intVal); 
            break;
        case NT_OP_BINARIO:
        case NT_OP_UNARIO:
            printf(": (Operator Token %d)\n", node->value.op); 
            break;
        default:
            printf("\n"); // Structure node
    }

    struct ast_node *child = node->child;
    while (child != NULL) {
        print_ast(child, indent + 1);
        child = child->sibling;
    }
}
