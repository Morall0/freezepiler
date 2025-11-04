#ifndef AST_H
#define AST_H

// Abstract Syntax Tree Node Types
typedef enum {
    NT_PROGRAMA,
    NT_FUNCION,
    NT_PARAMETRO,
    NT_DECLARACION,
    NT_TIPO,
    NT_VAR,
    NT_ARRAY_DECL,
    NT_BLOQUE,
    NT_IF,
    NT_WHILE,
    NT_DO_WHILE,
    NT_FOR,
    NT_SWITCH,
    NT_CASE,
    NT_DEFAULT,
    NT_BREAK,
    NT_CONTINUE,
    NT_RETURN,
    NT_GOTO,
    NT_ETIQUETA,
    NT_EXPR_SENTENCIA,
    NT_ASIGNACION,
    NT_OP_BINARIO,
    NT_OP_UNARIO,
    NT_TERNARIO,
    NT_LLAMADA_FUNCION,
    NT_ACCESO_ARRAY,
    NT_ACCESO_MIEMBRO,
    NT_ID,
    NT_ENTERO,
    NT_FLOTANTE,
    NT_CADENA,
    NT_CARACTER
} NodeType;

// Abstract Syntax Tree Node Structure
typedef struct ast_node {
    NodeType type;
    struct ast_node *child;   // First child
    struct ast_node *sibling; // Next sibling in a list

    // Leaf node value
    union {
        int intVal;
        float floatVal;
        char *strVal;
        int op; // Operator token
    } value;
} ast_node;

// Global AST root pointer
extern struct ast_node *ast_root;

struct ast_node *make_node(NodeType type, struct ast_node *child);
struct ast_node *make_op_node(int op, struct ast_node *left, struct ast_node *right);
struct ast_node *make_unary_op_node(int op, struct ast_node *operand);
struct ast_node *make_leaf_int(NodeType type, int val);
struct ast_node *make_leaf_float(NodeType type, float val);
struct ast_node *make_leaf_str(NodeType type, char *str);
struct ast_node *ast_append_sibling(struct ast_node *list_head, struct ast_node *new_sibling);

// Function to print the AST
void print_ast(struct ast_node *node, int indent);

/* 
Main semantic validation (SDT) function.
Called by main.c after a successful parse.
Returns 1 on semantic success, 0 on semantic error.
*/
int validate_sdt(struct ast_node *root);

#endif // AST_H
