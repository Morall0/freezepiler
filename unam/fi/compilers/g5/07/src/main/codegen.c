#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "codegen.h"
#include "parser.tab.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>


static LLVMModuleRef module;
static LLVMBuilderRef builder;
static LLVMTypeRef i32_type;
static LLVMTypeRef i8_type;  /* char */
static LLVMTypeRef f64_type; /* use double for floating point */
static LLVMBasicBlockRef current_switch_end_block = NULL;


void codegen_block(ast_node *block, LLVMValueRef function);

// map token -> llvm type using the token names from parser.tab.h
static LLVMTypeRef map_type_token(int token) {
  switch (token) {
    case T_INT:       // 258
    case T_SHORT:
    case T_LONG:
    case T_SIGNED:
    case T_UNSIGNED:
    case T_CHAR:      // 260
      return LLVMInt32Type();

    case T_FLOAT:     // 259
      return LLVMFloatType();

    case T_DOUBLE:    // 262
      return LLVMDoubleType();

    case T_VOID:      // 261
      return LLVMVoidType();

    default:
      //       //fprintf(stderr, "[map_type_token] WARNING: token tipo desconocido (%d). Usando i32 por fallback.\n", token);
      return LLVMInt32Type();
  }
}

static LLVMTypeRef map_type_node(ast_node *type_node) {
  if (!type_node) {
    //     //fprintf(stderr, "[map_type_node] NULL type_node, fallback i32\n");
    return LLVMInt32Type();
  }

  LLVMTypeRef result = NULL;

  if (type_node->type == NT_TIPO) {
    result = map_type_token(type_node->value.intVal);
  } else {
    result = map_type_token(type_node->value.intVal);
  }


  if (result) {
    LLVMTypeKind kind = LLVMGetTypeKind(result);
    //     //fprintf(stderr, "[map_type_node] tipo resultante kind=%d\n", kind);


    if (kind != LLVMIntegerTypeKind && kind != LLVMFloatTypeKind && kind != LLVMDoubleTypeKind && kind != LLVMVoidTypeKind) {
      //       //fprintf(stderr, "[map_type_node] ERROR: tipo no permitido (kind=%d), forzando i32\n", kind);
      result = LLVMInt32Type();
    }


    if (kind == LLVMIntegerTypeKind) {
      unsigned width = LLVMGetIntTypeWidth(result);
      if (width != 32) {
        //         //fprintf(stderr, "[map_type_node] WARNING: integer width=%d, forzando i32\n", width);
        result = LLVMInt32Type();
      }
    }
  } else {
    //     //fprintf(stderr, "[map_type_node] ERROR: tipo NULL, usando i32\n");
    result = LLVMInt32Type();
  }

  return result;
}



typedef struct sym_entry {
  char *name;
  LLVMValueRef alloc; // i32* alloca
  struct sym_entry *next;
} sym_entry;
static sym_entry *sym_table = NULL;

static void sym_put(const char *name, LLVMValueRef alloc) {
  if (!name) name = "(null)";
  //   //fprintf(stderr, "[sym_put] %s -> %p\n", name, (void*)alloc);
  sym_entry *e = malloc(sizeof(*e));
  e->name = strdup(name);
  e->alloc = alloc;
  e->next = sym_table;
  sym_table = e;
}
static LLVMValueRef sym_get(const char *name) {
  if (!name) name = "(null)";
  //   //fprintf(stderr, "[sym_get] buscando '%s'\n", name);
  for (sym_entry *e = sym_table; e; e = e->next) {
    //     //fprintf(stderr, "  comprobando %s (alloc=%p)\n", e->name, (void*)e->alloc);
    if (strcmp(e->name, name) == 0) {
      //       //fprintf(stderr, "[sym_get] encontrado %s -> %p\n", e->name, (void*)e->alloc);
      return e->alloc;
    }
  }
  //   //fprintf(stderr, "[sym_get] NO encontrado '%s'\n", name);
  return NULL;
}
static void sym_clear(void) {
  //   //fprintf(stderr, "[sym_clear] start\n");
  while (sym_table) {
    sym_entry *t = sym_table;
    sym_table = t->next;
    //     //fprintf(stderr, "  free %s\n", t->name);
    free(t->name);
    free(t);
  }
  //   //fprintf(stderr, "[sym_clear] end\n");
}


static LLVMValueRef create_entry_alloca(LLVMValueRef function, const char *name, LLVMTypeRef elem_type) {
  //   //fprintf(stderr, "[create_entry_alloca] function=%p name=%s elem_type=%p\n", (void*)function, name?name:"(null)", (void*)elem_type);

  if (!function) {
    //     //fprintf(stderr, "[create_entry_alloca] ERROR: function NULL\n");
    return NULL;
  }


  elem_type = i32_type;
  //   //fprintf(stderr, "  [create_entry_alloca] FORZANDO i32_type: %p\n", (void*)elem_type);

  LLVMBasicBlockRef entry = LLVMGetEntryBasicBlock(function);
  LLVMBuilderRef tmp = LLVMCreateBuilder();

  if (!entry) {
    //     //fprintf(stderr, "  [create_entry_alloca] entry block no existe, creando uno\n");
    entry = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(tmp, entry);
  } else {
    LLVMValueRef first = LLVMGetFirstInstruction(entry);
    if (first) {
      LLVMPositionBuilderBefore(tmp, first);
      //       //fprintf(stderr, "  [create_entry_alloca] posicionando builder antes de la primera instruccion\n");
    } else {
      LLVMPositionBuilderAtEnd(tmp, entry);
      //       //fprintf(stderr, "  [create_entry_alloca] posicionando builder al final del entry (sin instrucciones)\n");
    }
  }

  LLVMValueRef a = LLVMBuildAlloca(tmp, elem_type, name);
  //   //fprintf(stderr, "  [create_entry_alloca] alloca creada: %p\n", (void*)a);

  LLVMDisposeBuilder(tmp);
  return a;
}



static LLVMValueRef safe_load(LLVMValueRef ptr, const char *name) {
  //   //fprintf(stderr, "[safe_load] ptr=%p name=%s\n", (void*)ptr, name?name:"(null)");
  if (!ptr) {
    //     //fprintf(stderr, "[safe_load] ERROR: ptr NULL\n");
    return NULL;
  }


  LLVMTypeRef t_elem = i32_type;

  //   //fprintf(stderr, "[safe_load] llamando LLVMBuildLoad2 con tipo elemento %p...\n", (void*)t_elem);
  LLVMValueRef v = LLVMBuildLoad2(builder, t_elem, ptr, name);
  //   //fprintf(stderr, "[safe_load] load ok -> %p\n", (void*)v);
  return v;
}












// =======================================================
// EXPRESIONES
// =======================================================
static LLVMValueRef cast_to_bool(LLVMValueRef v) {
  LLVMTypeRef ty = LLVMTypeOf(v);
  LLVMTypeKind k = LLVMGetTypeKind(ty);

  if (k == LLVMIntegerTypeKind) {
    LLVMValueRef zero = LLVMConstInt(ty, 0, 0);
    return LLVMBuildICmp(builder, LLVMIntNE, v, zero, "i_bool");
  }
  if (k == LLVMDoubleTypeKind) {
    LLVMValueRef zero = LLVMConstReal(ty, 0.0);
    return LLVMBuildFCmp(builder, LLVMRealUNE, v, zero, "f_bool");
  }

  return LLVMConstInt(LLVMInt1Type(), 0, 0);
}

static LLVMValueRef codegen_expr(ast_node *expr, LLVMValueRef current_fn) {
  if (!expr) {
    //     fprintf(stderr, "[codegen_expr] ERROR: expr == NULL\n");
    //printf("[DEBUG] codegen_expr: expr es NULL\n");
    return NULL;
  }
  //   fprintf(stderr, "[codegen_expr] entrada: type=%d lineno=%d addr=%p\n", expr->type, expr->lineno, (void*)expr);
  //printf("[DEBUG] codegen_expr: tipo=%d, lineno=%d\n", expr->type, expr->lineno);
  switch (expr->type) {
    case NT_ENTERO:
      //       fprintf(stderr, "[codegen_expr] ENTERO: %d\n", expr->value.intVal);
      return LLVMConstInt(i32_type, expr->value.intVal, 0);

    case NT_FLOTANTE: {
      double dv = (double) expr->value.floatVal;
      return LLVMConstReal(f64_type, dv);
    }
    case NT_CARACTER: {

      const char *s = expr->value.strVal ? expr->value.strVal : "\0";
      unsigned char ch = 0;
      if (s[0] == '\'' && s[1] && s[2] == '\'') ch = (unsigned char)s[1];
      else ch = (unsigned char)s[0];

      return LLVMConstInt(LLVMInt8Type(), ch, 0);
    }
    case NT_ID:
    case NT_VAR: {
      const char *name = expr->value.strVal;
      //       fprintf(stderr, "[codegen_expr] ID/VAR: '%s' (addr %p)\n", name?name:"(null)", (void*)expr);
      if (!name) { //fprintf(stderr, "[codegen_expr] ERROR: name NULL\n"); 
        return NULL; }
      LLVMValueRef a = sym_get(name);
      if (!a) {
        //fprintf(stderr, "[codegen_expr] error: uso de identificador no declarado '%s' (linea %d)\n", name, expr->lineno);
        return NULL;
      }
      LLVMTypeRef t_of_a = LLVMTypeOf(a);
      LLVMTypeRef et = t_of_a ? LLVMGetElementType(t_of_a) : NULL;
      //       fprintf(stderr, "[codegen_expr] preparándose para load: alloca=%p LLVMTypeOf(alloca)=%p LLVMGetElementType=%p\n",
      //(void*)a, (void*)t_of_a, (void*)et);
      LLVMBuilderRef cur_builder = builder;
      //       fprintf(stderr, "[codegen_expr] builder=%p\n", (void*)cur_builder);
      if (cur_builder) {
        LLVMBasicBlockRef ib = LLVMGetInsertBlock(cur_builder);
        //         fprintf(stderr, "[codegen_expr] LLVMGetInsertBlock(builder)=%p\n", (void*)ib);
      }
      LLVMValueRef loaded = safe_load(a, name);
      if (!loaded) {
        //         fprintf(stderr, "[codegen_expr] ERROR: safe_load devolvió NULL para %s\n", name);
        return NULL;
      }
      //       fprintf(stderr, "[codegen_expr] load OK -> %p\n", (void*)loaded);
      return loaded;
    }

    case NT_CADENA: {  // tipo 31
      //printf("[DEBUG] Procesando CADENA: %s\n", expr->value.strVal);

      // Verificar que builder y module estén inicializados
      if (!builder) {
        //printf("[ERROR] builder es NULL al procesar cadena!\n");
        return NULL;
      }

      if (!module) {
        //printf("[ERROR] module es NULL al procesar cadena!\n");
        return NULL;
      }

      const char *str = expr->value.strVal;
      if (!str) {
        //printf("[WARNING] strVal es NULL, usando cadena vacía\n");
        str = "";
      }

      // Crear cadena global constante
      LLVMValueRef global_str = LLVMBuildGlobalStringPtr(builder, str, ".str");
      //printf("[DEBUG] Cadena creada: %p\n", (void*)global_str);

      return global_str;
    }

    case NT_TERNARIO: { 
      //       fprintf(stderr, "=== DEBUG: OPERADOR TERNARIO ===\n");


      ast_node* cond_node = expr->child;
      ast_node* then_node = expr->child->sibling;
      ast_node* else_node = expr->child->sibling->sibling;


      LLVMValueRef cond_value = codegen_expr(cond_node, current_fn);
      if (!cond_value) return NULL;


      LLVMTypeRef cond_type = LLVMTypeOf(cond_value);
      LLVMValueRef bool_cond;

      if (LLVMGetTypeKind(cond_type) == LLVMIntegerTypeKind && 
        LLVMGetIntTypeWidth(cond_type) == 1) {
        bool_cond = cond_value;
      } else {

        bool_cond = LLVMBuildICmp(builder, LLVMIntNE, cond_value, 
                                  LLVMConstInt(LLVMInt32Type(), 0, 0), "ternary_cond");
      }


      LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_fn, "ternary_then");
      LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_fn, "ternary_else");
      LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_fn, "ternary_merge");


      LLVMBuildCondBr(builder, bool_cond, then_block, else_block);


      LLVMPositionBuilderAtEnd(builder, then_block);
      LLVMValueRef then_value = codegen_expr(then_node, current_fn);
      if (!then_value) return NULL;
      LLVMBuildBr(builder, merge_block);
      LLVMBasicBlockRef then_block_end = LLVMGetInsertBlock(builder);


      LLVMPositionBuilderAtEnd(builder, else_block);
      LLVMValueRef else_value = codegen_expr(else_node, current_fn);
      if (!else_value) return NULL;
      LLVMBuildBr(builder, merge_block);
      LLVMBasicBlockRef else_block_end = LLVMGetInsertBlock(builder);


      LLVMPositionBuilderAtEnd(builder, merge_block);


      if (LLVMTypeOf(then_value) != LLVMTypeOf(else_value)) {
        //         fprintf(stderr, "Error: tipos incompatibles en operador ternario\n");
        return NULL;
      }


      LLVMTypeRef result_type = LLVMTypeOf(then_value);
      LLVMValueRef phi = LLVMBuildPhi(builder, result_type, "ternary_result");

      LLVMValueRef phi_values[2] = {then_value, else_value};
      LLVMBasicBlockRef phi_blocks[2] = {then_block_end, else_block_end};
      LLVMAddIncoming(phi, phi_values, phi_blocks, 2);

      return phi;
    }
    case NT_OP_UNARIO: {
      int op = expr->value.op;
      //       fprintf(stderr, "[codegen_expr] OP_UNARIO token=%d (addr=%p)\n", op, (void*)expr);

      ast_node *operand = expr->child;
      if (!operand) {
        //         fprintf(stderr, "[codegen_expr] ERROR: operando unario inválido\n");
        return NULL;
      }

      //       fprintf(stderr, "[codegen_expr] Operando type=%d\n", operand->type);

      if (op == 323) { // ++ (incremento)
        //         fprintf(stderr, "[codegen_expr] INCREMENTO ++ INICIADO\n");

        const char *name = operand->value.strVal;
        //         fprintf(stderr, "[codegen_expr] Variable: %s\n", name);

        LLVMValueRef dest = sym_get(name);
        if (!dest) {
          //           fprintf(stderr, "[codegen_expr] ERROR: variable no declarada '%s'\n", name);
          return NULL;
        }


        LLVMValueRef current = safe_load(dest, "loadtmp");
        if (!current) {
          //           fprintf(stderr, "[codegen_expr] ERROR: no se pudo cargar valor\n");
          return NULL;
        }

        //         fprintf(stderr, "[codegen_expr] Valor actual cargado: %p\n", (void*)current);


        LLVMValueRef one = LLVMConstInt(i32_type, 1, 0);
        LLVMValueRef result = LLVMBuildAdd(builder, current, one, "inctmp");

        //         fprintf(stderr, "[codegen_expr] Resultado del incremento: %p\n", (void*)result);


        LLVMBuildStore(builder, result, dest);
        //         fprintf(stderr, "[codegen_expr] Nuevo valor guardado\n");


        //         fprintf(stderr, "[codegen_expr] INCREMENTO ++ COMPLETADO, retornando: %p\n", (void*)result);
        return result;
      }

      if (op == 324) { // -- (decremento)
        //         fprintf(stderr, "[codegen_expr] DECREMENTO -- INICIADO\n");

        const char *name = operand->value.strVal;
        //         fprintf(stderr, "[codegen_expr] Variable: %s\n", name);

        LLVMValueRef dest = sym_get(name);
        if (!dest) {
          //           fprintf(stderr, "[codegen_expr] ERROR: variable no declarada '%s'\n", name);
          return NULL;
        }

        LLVMValueRef current = safe_load(dest, "loadtmp");
        if (!current) {
          //           fprintf(stderr, "[codegen_expr] ERROR: no se pudo cargar valor\n");
          return NULL;
        }

        //         fprintf(stderr, "[codegen_expr] Valor actual cargado: %p\n", (void*)current);


        LLVMValueRef one = LLVMConstInt(i32_type, 1, 0);
        LLVMValueRef result = LLVMBuildSub(builder, current, one, "subtmp");

        //         fprintf(stderr, "[codegen_expr] Resultado del decremento: %p\n", (void*)result);


        LLVMBuildStore(builder, result, dest);
        //         fprintf(stderr, "[codegen_expr] Nuevo valor guardado\n");


        //         fprintf(stderr, "[codegen_expr] DECREMENTO -- COMPLETADO, retornando: %p\n", (void*)result);
        return result;
      }


      else if (op == 336) { // NOT
        LLVMValueRef result = NULL;

        LLVMValueRef operand = codegen_expr(expr->child, current_fn);


        LLVMTypeRef operand_type = LLVMTypeOf(operand);
        LLVMValueRef operand_i32;

        if (LLVMGetTypeKind(operand_type) == LLVMIntegerTypeKind && 
          LLVMGetIntTypeWidth(operand_type) == 1) {

          operand_i32 = LLVMBuildZExt(builder, operand, LLVMInt32Type(), "operand_i32");
        } else {

          operand_i32 = operand;
        }


        LLVMValueRef is_true = LLVMBuildICmp(builder, LLVMIntNE, operand_i32, 
                                             LLVMConstInt(LLVMInt32Type(), 0, 0), "is_true");


        LLVMValueRef not_bool = LLVMBuildICmp(builder, LLVMIntEQ, is_true, 
                                              LLVMConstInt(LLVMInt1Type(), 0, 0), "not_bool");

        result = LLVMBuildZExt(builder, not_bool, LLVMInt32Type(), "not_result");
        return result;
      }

      else if (op == 337) { // ~

        LLVMValueRef operand = codegen_expr(expr->child, current_fn);
        if (!operand) return NULL;


        LLVMTypeRef int32_type = LLVMInt32Type();
        LLVMValueRef minus_one = LLVMConstInt(int32_type, -1, 1); // 1 = signed

        return LLVMBuildXor(builder, operand, minus_one, "bitwise_not");
      }

      else if (op == 319 || op == 340) { // T_UMINUS (operador unario negativo) - AMBOS TOKENS
        //         fprintf(stderr, "[codegen_expr] OPERADOR UNARIO NEGATIVO\n");

        LLVMValueRef operand_val = codegen_expr(operand, current_fn);
        if (!operand_val) {
          //           fprintf(stderr, "[codegen_expr] ERROR: no se pudo generar operando\n");
          return NULL;
        }

        // Negar el valor
        LLVMValueRef result = LLVMBuildNeg(builder, operand_val, "negtmp");
        //         fprintf(stderr, "[codegen_expr] NEGACIÓN COMPLETADA: %p\n", (void*)result);
        return result;
      }

      //       fprintf(stderr, "[codegen_expr] op unario no soportado %d\n", op);
      return NULL;
    }


    case NT_OP_BINARIO: {
      int op = expr->value.op;
      //       fprintf(stderr, "[codegen_expr] OP_BINARIO token=%d (addr=%p)\n", op, (void*)expr);
      ast_node *L = expr->child;
      ast_node *R = L ? L->sibling : NULL;
      if (!L || !R) { 
        //fprintf(stderr,"[codegen_expr] ERROR: subexpresión inválida (linea %d) L=%p R=%p\n", expr->lineno, (void*)L, (void*)R); 
        return NULL; }

      if (op == 307) { // assign =
        const char *name = NULL;
        if (L->type == NT_ID || L->type == NT_VAR) name = L->value.strVal;
        if (!name) { 
          //fprintf(stderr,"[codegen_expr] assign: LHS inválido (linea %d)\n", expr->lineno); 
          return NULL; }
        LLVMValueRef dest = sym_get(name);
        if (!dest) { 
          //fprintf(stderr,"[codegen_expr] assign: variable no declarada %s (linea %d)\n", name, expr->lineno); 
          return NULL; }
        LLVMValueRef rv = codegen_expr(R, current_fn);
        if (!rv) { 
          //fprintf(stderr,"[codegen_expr] assign: RHS produjo NULL (linea %d)\n", expr->lineno); 
          return NULL; }
        LLVMBuildStore(builder, rv, dest);
        //         fprintf(stderr, "[codegen_expr] asignación store OK dest=%p val=%p\n", (void*)dest, (void*)rv);
        return rv;
      }
      if (op == 308) { // += 
        const char *name = (L->type==NT_ID||L->type==NT_VAR)?L->value.strVal:NULL;
        if (!name) { 
          //fprintf(stderr,"[codegen_expr] assignAdd: LHS inválido\n"); 
          return NULL; }
        LLVMValueRef dest = sym_get(name);
        if (!dest) { 
          //fprintf(stderr,"[codegen_expr] assignAdd: variable no declarada %s\n", name); 
          return NULL; }
        LLVMValueRef lv = safe_load(dest, "loadtmp");
        if (!lv) { 
          //fprintf(stderr, "[codegen_expr] assignAdd: load failed\n"); 
          return NULL; }
        LLVMValueRef rv = codegen_expr(R, current_fn);
        if (!rv) { 
          //fprintf(stderr,"[codegen_expr] assignAdd: RHS null\n"); 
          return NULL; }
        LLVMValueRef result = LLVMBuildAdd(builder, lv, rv, "addtmp");
        LLVMBuildStore(builder, result, dest);
        //         fprintf(stderr, "[codegen_expr] assignAdd OK dest=%p result=%p\n", (void*)dest, (void*)result);
        return result;
      }
      if (op == 310) { // *= compuesto
        const char *name = (L->type==NT_ID||L->type==NT_VAR)?L->value.strVal:NULL;
        if (!name) { 
          //fprintf(stderr,"[codegen_expr] assignComp: LHS inválido (linea %d)\n", expr->lineno); 
          return NULL; }
        LLVMValueRef dest = sym_get(name);
        if (!dest) { 
          //fprintf(stderr,"[codegen_expr] assignComp: variable no declarada %s (linea %d)\n", name, expr->lineno); 
          return NULL; }
        LLVMValueRef lv = safe_load(dest, "loadtmp");
        if (!lv) { 
          //fprintf(stderr, "[codegen_expr] assignComp: load failed\n"); 
          return NULL; }
        LLVMValueRef rv = codegen_expr(R, current_fn);
        if (!rv) { 
          //fprintf(stderr,"[codegen_expr] assignComp: RHS null\n"); 
          return NULL; }
        LLVMValueRef result = LLVMBuildMul(builder, lv, rv, "multmp");
        LLVMBuildStore(builder, result, dest);
        //         fprintf(stderr, "[codegen_expr] assignComp OK dest=%p result=%p\n", (void*)dest, (void*)result);
        return result;
      }




      LLVMValueRef lv = codegen_expr(L, current_fn);
      LLVMValueRef rv = codegen_expr(R, current_fn);
      if (!lv || !rv) { 
        //         fprintf(stderr, "[codegen_expr] ERROR: subexpr produjo NULL lv=%p rv=%p\n", (void*)lv, (void*)rv); 
        return NULL; 
      }


      if (op == 309) { // -=
        //         fprintf(stderr, "=== DEBUG: -= assignment ===\n");

        ast_node* lhs_node = expr->child;
        if (lhs_node->type != 28) {
          //           fprintf(stderr, "Error: lhs must be a variable for -= assignment\n");
          return NULL;
        }

        const char* var_name = lhs_node->value.strVal;
        LLVMValueRef lhs_ptr = sym_get(var_name);

        if (!lhs_ptr) {
          //           fprintf(stderr, "Error: variable '%s' not found\n", var_name);
          return NULL;
        }

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildSub(builder, current_val, rv, "sub_assign");
        LLVMBuildStore(builder, result, lhs_ptr);

        return result;
      }

      if (op == 311) { // /=
        //         fprintf(stderr, "=== DEBUG: /= assignment ===\n");

        ast_node* lhs_node = expr->child;
        if (lhs_node->type != 28) {
          //           fprintf(stderr, "Error: lhs must be a variable for /= assignment\n");
          return NULL;
        }

        const char* var_name = lhs_node->value.strVal;
        LLVMValueRef lhs_ptr = sym_get(var_name);

        if (!lhs_ptr) {
          //           fprintf(stderr, "Error: variable '%s' not found\n", var_name);
          return NULL;
        }

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildSDiv(builder, current_val, rv, "div_assign");
        LLVMBuildStore(builder, result, lhs_ptr);

        return result;
      }

      if (op == 312) { // %=
        //         fprintf(stderr, "=== DEBUG: %%= assignment ===\n");

        ast_node* lhs_node = expr->child;
        if (lhs_node->type != 28) {
          //           fprintf(stderr, "Error: lhs must be a variable for %%= assignment\n");
          return NULL;
        }

        const char* var_name = lhs_node->value.strVal;
        LLVMValueRef lhs_ptr = sym_get(var_name);

        if (!lhs_ptr) {
          //           fprintf(stderr, "Error: variable '%s' not found\n", var_name);
          return NULL;
        }

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildSRem(builder, current_val, rv, "mod_assign");
        LLVMBuildStore(builder, result, lhs_ptr);

        return result;
      }

      if (op == 333) { // & (AND bit a bit)
        return LLVMBuildAnd(builder, lv, rv, "andtmp");
      }
      if (op == 334) { // | (OR bit a bit)
        return LLVMBuildOr(builder, lv, rv, "ortmp");
      }

      if (op == 335) { // ^ (XOR bit a bit)
        return LLVMBuildXor(builder, lv, rv, "xortmp");
      }

      if (op == 338) { // << (shift left)
        return LLVMBuildShl(builder, lv, rv, "shltmp");
      }

      if (op == 339) { // >> (shift right)
        return LLVMBuildAShr(builder, lv, rv, "ashrtmp"); // Arithmetic shift
      }


      if (op == 340) { // &=

        ast_node *var_node = expr->child;

        LLVMValueRef lhs_ptr = sym_get(var_node->value.strVal);
        if (!lhs_ptr) {
          //           fprintf(stderr, "Error: variable no encontrada para asignación compuesta\n");
          return NULL;
        }

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildAnd(builder, current_val, rv, "and_assign");
        LLVMBuildStore(builder, result, lhs_ptr);
        return result;
      }

      if (op == 341) { // |= (OR assignment)
        LLVMValueRef lhs_ptr = codegen_expr(expr->child, current_fn);
        if (!lhs_ptr) return NULL;

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildOr(builder, current_val, rv, "or_assign");
        LLVMBuildStore(builder, result, lhs_ptr);
        return result;
      }

      if (op == 342) { // ^= (XOR assignment)  NO ESXISTe
        LLVMValueRef lhs_ptr = codegen_expr(expr->child, current_fn);
        if (!lhs_ptr) return NULL;

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildXor(builder, current_val, rv, "xor_assign");
        LLVMBuildStore(builder, result, lhs_ptr);
        return result;
      }

      if (op == 343) { // <<= (Shift left assignment) NO EXISTE
        LLVMValueRef lhs_ptr = codegen_expr(expr->child, current_fn);
        if (!lhs_ptr) return NULL;

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildShl(builder, current_val, rv, "shl_assign");
        LLVMBuildStore(builder, result, lhs_ptr);
        return result;
      }

      if (op == 344) { // >>= (Shift right assignment) NO EXISTE
        LLVMValueRef lhs_ptr = codegen_expr(expr->child, current_fn);
        if (!lhs_ptr) return NULL;

        LLVMValueRef current_val = LLVMBuildLoad2(builder, LLVMInt32Type(), lhs_ptr, "load_current");
        LLVMValueRef result = LLVMBuildAShr(builder, current_val, rv, "shr_assign");
        LLVMBuildStore(builder, result, lhs_ptr);
        return result;
      }

      if (op == 318) { LLVMValueRef t = LLVMBuildAdd(builder, lv, rv, "addtmp"); 
        //fprintf(stderr,"[codegen_expr] ADD -> %p\n",(void*)t); 
        return t; }

      if (op == 319) { // -
        LLVMValueRef t = LLVMBuildSub(builder, lv, rv, "subtmp"); 
        //         fprintf(stderr,"[codegen_expr] SUB -> %p\n",(void*)t); 
        return t; 
      }
      if (op == 320) { // *
        LLVMValueRef t = LLVMBuildMul(builder, lv, rv, "multmp"); 
        //         fprintf(stderr,"[codegen_expr] MUL -> %p\n",(void*)t); 
        return t; 
      }
      if (op == 321) { // /
        LLVMValueRef t = LLVMBuildSDiv(builder, lv, rv, "divtmp"); 
        //         fprintf(stderr,"[codegen_expr] DIV -> %p\n",(void*)t); 
        return t; 
      }

      if (op == 322) { // %
        LLVMValueRef t = LLVMBuildSRem(builder, lv, rv, "modtmp"); 
        //         fprintf(stderr,"[codegen_expr] MOD -> %p\n",(void*)t); 
        return t; 
      }


      if (op == 325) { // T_EQ (igualdad)
        //         fprintf(stderr, "[codegen_expr] CMP ==\n");
        LLVMValueRef t = LLVMBuildICmp(builder, LLVMIntEQ, lv, rv, "eqtmp");
        return t;
      }
      if (op == 326) { // T_NEQ (desigualdad)  
        //         fprintf(stderr, "[codegen_expr] CMP !=\n");
        LLVMValueRef t = LLVMBuildICmp(builder, LLVMIntNE, lv, rv, "netmp");
        return t;
      }

      if (op == 327) { LLVMValueRef t = LLVMBuildICmp(builder, LLVMIntSLT, lv, rv, "cmptmp"); 
        //fprintf(stderr,"[codegen_expr] CMP < -> %p\n",(void*)t); 
        return t; } // <


      if (op == 329) { // >
        LLVMValueRef t = LLVMBuildICmp(builder, LLVMIntSGT, lv, rv, "cmptmp"); 
        //         fprintf(stderr,"[codegen_expr] CMP > -> %p\n",(void*)t); 
        return t; 
      }

      if (op == 328) { // <=
        LLVMValueRef t = LLVMBuildICmp(builder, LLVMIntSLE, lv, rv, "cmptmp");
        //         fprintf(stderr,"[codegen_expr] CMP <= -> %p\n",(void*)t);
        return t;
      }
      if (op == 330) { // >=
        LLVMValueRef t = LLVMBuildICmp(builder, LLVMIntSGE, lv, rv, "cmptmp"); 
        //         fprintf(stderr,"[codegen_expr] CMP >= -> %p\n",(void*)t); 
        return t; 
      }


      if (op == 331) { //  && and 
        LLVMValueRef lb = cast_to_bool(lv);
        LLVMValueRef rb = cast_to_bool(rv);

        LLVMValueRef r = LLVMBuildAnd(builder, lb, rb, "and_i1");


        return r;    
      }
      if (op == 332) { // ||   or
        LLVMValueRef lb = cast_to_bool(lv);
        LLVMValueRef rb = cast_to_bool(rv);

        LLVMValueRef r = LLVMBuildOr(builder, lb, rb, "or_i1");


        return r;
      }
      //       fprintf(stderr, "[codegen_expr] op no soportado %d (linea %d)\n", op, expr->lineno);
      return NULL;
    }

    case NT_LLAMADA_FUNCION: {
      //       fprintf(stderr, "[codegen_expr] LLAMADA_FUNCION entrada (addr=%p)\n", (void*)expr);

      ast_node *fnexpr = expr->child;
      if (!fnexpr) {
        //         fprintf(stderr,"[codegen_expr] call: sin nombre (linea %d)\n", expr->lineno);
        return NULL;
      }
      const char *fname = fnexpr->value.strVal;
      //       fprintf(stderr, "[codegen_expr] call a '%s'\n", fname ? fname : "(null)");
      //printf("[DEBUG] Llamada a función: %s\n", fname);

      // Manejo especial para printf (función con argumentos variables)
      if (fname && strcmp(fname, "printf") == 0) {
        //printf("[DEBUG] Procesando llamada a printf\n");

        // Contar argumentos
        int nargs = 0;
        ast_node *arg_node = fnexpr->sibling;
        for (ast_node *t = arg_node; t; t = t->sibling) nargs++;

        //printf("[DEBUG] printf tiene %d argumentos\n", nargs);

        // Procesar argumentos
        LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * nargs);
        LLVMTypeRef *arg_types = malloc(sizeof(LLVMTypeRef) * nargs);

        int i = 0;
        for (ast_node *t = arg_node; t; t = t->sibling) {
          args[i] = codegen_expr(t, current_fn);
          LLVMTypeRef original_type = LLVMTypeOf(args[i]);

          // Para printf, los tipos deben ser específicos:
          // - cadenas: i8*
          // - enteros con %i, %d: i32
          // - caracteres con %c: i8 (pero se pasa como i32 en variádicas)
          // - floats/doubles con %f: double (se promueve float a double)

          if (LLVMGetTypeKind(original_type) == LLVMIntegerTypeKind) {
            // Para enteros, usar i32 (tamaño estándar para variádicas)
            unsigned width = LLVMGetIntTypeWidth(original_type);
            if (width != 32) {
              args[i] = LLVMBuildIntCast(builder, args[i], i32_type, "intcast");
            }
            arg_types[i] = i32_type;
          }
          else if (LLVMGetTypeKind(original_type) == LLVMPointerTypeKind) {
            // Para punteros (cadenas), usar i8*
            arg_types[i] = original_type;
          }
          else {
            // Para otros tipos, mantener el tipo original
            arg_types[i] = original_type;
          }

          i++;
        }

        // Crear tipo de función para esta llamada específica
        LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), arg_types, nargs, 1);

        // Obtener función printf 
        LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
        if (!printf_func) {
          // Declarar con el tipo correcto
          printf_func = LLVMAddFunction(module, "printf", printf_type);
        }

        // Construir la llamada
        LLVMValueRef call = LLVMBuildCall2(builder, printf_type, printf_func, args, nargs, "printfcall");

        //printf("[DEBUG] Llamada a printf construida exitosamente\n");

        free(args);
        free(arg_types);
        return call;
      }






      LLVMValueRef callee = LLVMGetNamedFunction(module, fname);
      if (!callee) {
        //         fprintf(stderr, "[codegen_expr] call: función '%s' no encontrada (linea %d)\n", fname, expr->lineno);
        return NULL;
      }


      LLVMTypeRef callee_type = LLVMTypeOf(callee);
      //       fprintf(stderr, "[codegen_expr] callee_type=%p\n", (void*)callee_type);

      if (!callee_type) {
        //         fprintf(stderr, "[codegen_expr] ERROR: no se pudo obtener tipo de callee\n");
        return NULL;
      }

      LLVMTypeKind callee_kind = LLVMGetTypeKind(callee_type);
      //       fprintf(stderr, "[codegen_expr] callee_type kind=%d\n", callee_kind);

      LLVMTypeRef callee_elem = NULL;
      if (callee_kind == LLVMPointerTypeKind) {
        callee_elem = LLVMGetElementType(callee_type);
        //         fprintf(stderr, "[codegen_expr] callee_elem=%p\n", (void*)callee_elem);
        if (callee_elem) {
          LLVMTypeKind elem_kind = LLVMGetTypeKind(callee_elem);
          //           fprintf(stderr, "[codegen_expr] callee_elem kind=%d\n", elem_kind);
        }
      }


      if (!callee_elem || LLVMGetTypeKind(callee_elem) != LLVMFunctionTypeKind) {
        //         fprintf(stderr, "[codegen_expr] WARNING: tipo de función inválido, creando fallback\n");


        int nargs = 0;
        ast_node *a = fnexpr->sibling;
        for (ast_node *t = a; t; t = t->sibling) nargs++;


        LLVMTypeRef *param_types = NULL;
        if (nargs > 0) {
          param_types = malloc(sizeof(LLVMTypeRef) * nargs);
          for (int i = 0; i < nargs; i++) {
            param_types[i] = i32_type;
          }
        }
        callee_elem = LLVMFunctionType(i32_type, param_types, nargs, 0);
        if (param_types) free(param_types);

        //         fprintf(stderr, "[codegen_expr] Fallback type creado: %p\n", (void*)callee_elem);
      }

      // Procesar argumentos
      int nargs = 0;
      ast_node *a = fnexpr->sibling;
      for (ast_node *t = a; t; t = t->sibling) nargs++;
      //       fprintf(stderr, "[codegen_expr] call: nargs AST = %d\n", nargs);

      LLVMValueRef *argv = NULL;
      if (nargs > 0) {
        argv = malloc(sizeof(LLVMValueRef) * nargs);
        if (!argv) {
          //           fprintf(stderr, "[codegen_expr] ERROR: malloc argv failed\n");
          return NULL;
        }
        int i = 0;
        for (ast_node *t = a; t; t = t->sibling) {
          argv[i] = codegen_expr(t, current_fn);
          //           fprintf(stderr, "[codegen_expr] call: arg %d expr -> %p\n", i, (void*)argv[i]);
          i++;
        }
      }


      unsigned expected_params = LLVMCountParamTypes(callee_elem);
      //       fprintf(stderr, "[codegen_expr] Función espera %u params, tenemos %d\n", expected_params, nargs);

      if ((unsigned)nargs != expected_params) {
        //         fprintf(stderr, "[codegen_expr] ERROR: número de argumentos incorrecto\n");
        if (argv) free(argv);
        return NULL;
      }

      //       fprintf(stderr, "[codegen_expr] calling LLVMBuildCall2 con callee=%p callee_elem=%p nargs=%d\n", 
      //    (void*)callee, (void*)callee_elem, nargs);

      LLVMValueRef call = LLVMBuildCall2(builder, callee_elem, callee, argv, nargs, "calltmp");
      //       fprintf(stderr, "[codegen_expr] call result %p\n", (void*)call);

      if (argv) free(argv);
      return call;
    }

    case NT_EXPR_SENTENCIA:
      return codegen_expr(expr->child, current_fn);

    default:
      //       fprintf(stderr, "[codegen_expr] tipo no soportado %d (linea %d)\n", expr->type, expr->lineno);
      return NULL;
  }
}


// =======================================================
// STATEMENTS
// =======================================================
static void codegen_statement(ast_node *stmt, LLVMValueRef current_fn) {
  if (!stmt) {
    //     fprintf(stderr, "[codegen_statement] stmt == NULL\n");
    return;
  }
  //   fprintf(stderr, "\n[codegen_statement] entrada: type=%d lineno=%d addr=%p\n", stmt->type, stmt->lineno, (void*)stmt);

  switch (stmt->type) {


    case NT_SWITCH: { // NO SE NECESITA
      //printf("[codegen_statement] SWITCH - adaptado a estructura actual\n");

      ast_node *switch_expr = stmt->child;
      ast_node *switch_body = switch_expr->sibling;

      // Generar expresión del switch
      LLVMValueRef switch_value = codegen_expr(switch_expr, current_fn);

      LLVMContextRef ctx = LLVMGetGlobalContext();
      LLVMBasicBlockRef switch_end_block = LLVMCreateBasicBlockInContext(ctx, "switch.end");
      LLVMBasicBlockRef switch_default_block = LLVMCreateBasicBlockInContext(ctx, "switch.default");

      // Guardar estado para break
      LLVMBasicBlockRef prev_switch_end = current_switch_end_block;
      current_switch_end_block = switch_end_block;

      LLVMValueRef switch_inst = LLVMBuildSwitch(builder, switch_value, switch_default_block, 0);


      ast_node *case_node = switch_body->child;
      int case_index = 0;

      while (case_node != NULL) {
        if (case_node->type == 13) { // CASE_VALUE con asignación
          //printf("  Procesando CASE_VALUE[%d]\n", case_index);

          ast_node *case_value_node = case_node->child; // Valor del case
          ast_node *assign_node = case_value_node->sibling; // Asignación

          // Generar valor del case
          LLVMValueRef case_value = codegen_expr(case_value_node, current_fn);


          char case_block_name[32];
          snprintf(case_block_name, sizeof(case_block_name), "switch.case.%d", case_index);
          LLVMBasicBlockRef case_block = LLVMCreateBasicBlockInContext(ctx, case_block_name);

          LLVMAddCase(switch_inst, case_value, case_block);


          LLVMPositionBuilderAtEnd(builder, case_block);
          codegen_statement(assign_node, current_fn);


          if (LLVMGetInsertBlock(builder) != NULL) {
            LLVMBuildBr(builder, switch_end_block);
          }

        } else if (case_node->type == 14) { 
          //printf("  Procesando CASE_FINAL como DEFAULT\n");

          LLVMPositionBuilderAtEnd(builder, switch_default_block);
          codegen_statement(case_node->child, current_fn);             
          if (LLVMGetInsertBlock(builder) != NULL) {
            LLVMBuildBr(builder, switch_end_block);
          }
        }

        case_node = case_node->sibling;
        case_index++;
      }

      if (LLVMGetBasicBlockTerminator(switch_default_block) == NULL) {
        LLVMPositionBuilderAtEnd(builder, switch_default_block);
        LLVMBuildBr(builder, switch_end_block);
      }

      LLVMPositionBuilderAtEnd(builder, switch_end_block);
      current_switch_end_block = prev_switch_end;

      //printf("  SWITCH completado\n");
      break;
    }
    case NT_BREAK: { // NO SE NECESITa
      //printf("[codegen_statement] BREAK encontrado (type=13) en línea %d\n", stmt->lineno);

      if (current_switch_end_block != NULL) {
        printf("  -> Break dentro de switch, saltando al final del switch\n");
      } else {
        printf("  -> Break fuera de switch (no implementado)\n");
      }
      break;
    }
    case NT_DECLARACION: {
      //       fprintf(stderr, "[codegen_statement] DECLARACION\n");
      ast_node *tipo = stmt->child;
      LLVMTypeRef decl_type = map_type_node(tipo);
      ast_node *inits = tipo ? tipo->sibling : NULL;
      for (ast_node *cur = inits; cur; cur = cur->sibling) {
        //         fprintf(stderr, "  decl element type=%d\n", cur->type);
        if (cur->type == NT_VAR || cur->type == NT_ID) {
          const char *name = cur->value.strVal;
          //           fprintf(stderr, "    VAR %s type=%p\n", name?name:"(null)", (void*)decl_type);
          LLVMValueRef a = create_entry_alloca(current_fn, name, decl_type);
          if (!a) { 
            //fprintf(stderr, "    ERROR: alloca NULL\n"); 
            continue; }
          sym_put(name, a);
        }
        else if (cur->type == NT_OP_BINARIO && cur->value.op == 307) {
          ast_node *left = cur->child;
          ast_node *right = left ? left->sibling : NULL;
          const char *vname = left ? left->value.strVal : "(null)";
          //           fprintf(stderr, "    VAR+INIT %s\n", vname);
          LLVMValueRef a = create_entry_alloca(current_fn, vname, decl_type);
          if (!a) { 
            //fprintf(stderr, "    ERROR: alloca NULL for init\n"); 
            continue; }
          sym_put(vname, a);
          LLVMValueRef rv = codegen_expr(right, current_fn);
          if (rv) {
            LLVMBuildStore(builder, rv, a);
            //             fprintf(stderr, "    store init OK\n");
          } else {
            //             fprintf(stderr, "    WARNING: init produced NULL\n");
          }
        } else {
          //           fprintf(stderr, "    elemento decl no soportado type=%d\n", cur->type);
        }
      }
      break;
    }

    case NT_EXPR_SENTENCIA: {
      //       fprintf(stderr, "[codegen_statement] EXPR_SENTENCIA\n");
      LLVMValueRef v = codegen_expr(stmt->child, current_fn);
      //       fprintf(stderr, "[codegen_statement] EXPR_SENTENCIA result %p\n", (void*)v);
      break;
    }

    case NT_RETURN: {
      //printf("[codegen_statement] RETURN\n");

      LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
      if (current_block == NULL) {
        printf("  ERROR CRÍTICO: Builder no está en un bloque válido!\n");
        printf("  No se puede generar return. Posible problema con switch anterior.\n");
        break;
      }

      //printf("  Bloque actual es válido, generando return...\n");
      LLVMValueRef rv = NULL;
      if (stmt->child) rv = codegen_expr(stmt->child, current_fn);
      else rv = LLVMConstInt(i32_type, 0, 0);
      if (!rv) {
        //         fprintf(stderr,"[codegen_statement] return: expr produjo NULL (line %d)\n", stmt->lineno);
        LLVMBuildRet(builder, LLVMConstInt(i32_type, 0, 0));
      } else {
        LLVMBuildRet(builder, rv);
        //         fprintf(stderr, "[codegen_statement] return OK\n");
      }
      break;
    }

    case NT_IF: {
      //       fprintf(stderr, "[codegen_statement] IF\n");
      ast_node *cond = stmt->child;
      ast_node *thenb = cond ? cond->sibling : NULL;
      ast_node *elseb = thenb ? thenb->sibling : NULL;      
      if (!cond || !thenb) {
        //         fprintf(stderr, "[codegen_statement] IF malformed cond=%p then=%p\n", (void*)cond, (void*)thenb);
        return;
      }

      LLVMValueRef cval = codegen_expr(cond, current_fn);
      //       fprintf(stderr, "[codegen_statement] IF cond value = %p\n", (void*)cval);
      if (!cval) { 
        fprintf(stderr,"[codegen_statement] if: condición nula (linea %d)\n", stmt->lineno); 
        return; 
      }

      LLVMBasicBlockRef thenBB = LLVMAppendBasicBlock(current_fn, "then");
      LLVMBasicBlockRef elseBB = LLVMAppendBasicBlock(current_fn, "else");
      LLVMBasicBlockRef contBB = LLVMAppendBasicBlock(current_fn, "ifcont");

      //       fprintf(stderr, "[codegen_statement] IF building condbr then=%p else=%p cont=%p\n", 
      //   (void*)thenBB, (void*)elseBB, (void*)contBB);

      LLVMBuildCondBr(builder, cval, thenBB, elseBB);

      // Generar bloque THEN
      LLVMPositionBuilderAtEnd(builder, thenBB);
      if (thenb->type == NT_BLOQUE) 
        codegen_block(thenb, current_fn);
      else 
        codegen_statement(thenb, current_fn);

      if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL)
        LLVMBuildBr(builder, contBB);

      LLVMPositionBuilderAtEnd(builder, elseBB);
      if (elseb) {
        if (elseb->type == NT_BLOQUE) 
          codegen_block(elseb, current_fn);
        else 
          codegen_statement(elseb, current_fn);
      }

      if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder)) == NULL)
        LLVMBuildBr(builder, contBB);

      LLVMPositionBuilderAtEnd(builder, contBB);
      break;
    }

    case NT_FOR: {  // 276
      //       fprintf(stderr, "[codegen_statement] FOR\n");

      ast_node *init = stmt->child;
      ast_node *cond = init ? init->sibling : NULL;
      ast_node *inc = cond ? cond->sibling : NULL;
      ast_node *body = inc ? inc->sibling : NULL;

      if (!init || !cond || !inc || !body) {
        //         fprintf(stderr, "[codegen_statement] ERROR: estructura for inválida\n");
        return;
      }

      codegen_expr(init, current_fn);

      LLVMBasicBlockRef condBB = LLVMAppendBasicBlock(current_fn, "for.cond");
      LLVMBasicBlockRef bodyBB = LLVMAppendBasicBlock(current_fn, "for.body");
      LLVMBasicBlockRef incBB = LLVMAppendBasicBlock(current_fn, "for.inc");
      LLVMBasicBlockRef afterBB = LLVMAppendBasicBlock(current_fn, "for.after");

      LLVMBuildBr(builder, condBB);

      LLVMPositionBuilderAtEnd(builder, condBB);
      LLVMValueRef cond_val = codegen_expr(cond, current_fn);
      if (!cond_val) {
        //         fprintf(stderr, "[codegen_statement] ERROR: condición en for inválida\n");
        return;
      }
      LLVMBuildCondBr(builder, cond_val, bodyBB, afterBB);

      LLVMPositionBuilderAtEnd(builder, bodyBB);
      if (body->type == NT_BLOQUE) {
        codegen_block(body, current_fn); 
      } else {
        codegen_statement(body, current_fn); 
      }

      if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
        LLVMBuildBr(builder, incBB);
      }

      LLVMPositionBuilderAtEnd(builder, incBB);
      codegen_expr(inc, current_fn);

      if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
        LLVMBuildBr(builder, condBB);
      }

      LLVMPositionBuilderAtEnd(builder, afterBB);
      break;
    }

    case NT_WHILE: {
      //       fprintf(stderr, "[codegen_statement] WHILE loop\n");

      ast_node* while_node = stmt;
      ast_node* cond_node = while_node->child;
      ast_node* body_node = while_node->child->sibling;

      // Crear bloques básicos
      LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_fn, "while_cond");
      LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(current_fn, "while_body");
      LLVMBasicBlockRef after_block = LLVMAppendBasicBlock(current_fn, "while_after");

      // Saltar al bloque de condición
      LLVMBuildBr(builder, cond_block);

      // Bloque de condición
      LLVMPositionBuilderAtEnd(builder, cond_block);
      LLVMValueRef cond_value = codegen_expr(cond_node, current_fn);
      if (!cond_value) return;

      LLVMTypeRef cond_type = LLVMTypeOf(cond_value);
      LLVMValueRef bool_cond;

      if (LLVMGetTypeKind(cond_type) == LLVMIntegerTypeKind && 
        LLVMGetIntTypeWidth(cond_type) == 1) {
        bool_cond = cond_value;
      } else {
        bool_cond = LLVMBuildICmp(builder, LLVMIntNE, cond_value, 
                                  LLVMConstInt(LLVMInt32Type(), 0, 0), "while_cond");
      }

      LLVMBuildCondBr(builder, bool_cond, body_block, after_block);

      LLVMPositionBuilderAtEnd(builder, body_block);
      if (body_node->type == 7) { // BLOCK
        codegen_block(body_node, current_fn);
      } else {
        codegen_statement(body_node, current_fn);
      }

      LLVMBuildBr(builder, cond_block);

      LLVMPositionBuilderAtEnd(builder, after_block);

      break;
    } 
    case NT_DO_WHILE : { // DO_WHILE statement NO SE NECESITa
      //       fprintf(stderr, "[codegen_statement] DO_WHILE loop\n");

      ast_node* do_while_node = stmt;
      ast_node* body_node = do_while_node->child;
      ast_node* cond_block_node = do_while_node->child->sibling;

      //       fprintf(stderr, "=== DEBUG: Body type: %d, Cond block type: %d\n", 
      //  body_node->type, cond_block_node->type);

      // Crear bloques básicos
      LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(current_fn, "do_body");
      LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_fn, "do_cond");
      LLVMBasicBlockRef after_block = LLVMAppendBasicBlock(current_fn, "do_after");

      // Saltar al cuerpo
      LLVMBuildBr(builder, body_block);

      // Cuerpo
      LLVMPositionBuilderAtEnd(builder, body_block);
      if (body_node->type == 22) {
        LLVMValueRef body_result = codegen_expr(body_node, current_fn);
        (void)body_result; // Evitar warning de variable no usada
      } else if (body_node->type == 7) {
        codegen_block(body_node, current_fn);
      } else {
        codegen_statement(body_node, current_fn);
      }
      LLVMBuildBr(builder, cond_block);

      LLVMPositionBuilderAtEnd(builder, cond_block);

      LLVMValueRef cond_value = NULL;
      if (cond_block_node->type == 7 && cond_block_node->child) {
        ast_node* current_stmt = cond_block_node->child;
        ast_node* last_expr = NULL;

        while (current_stmt) {
          if (current_stmt->type == 20 || current_stmt->type == 22) {
            last_expr = current_stmt;
          }
          current_stmt = current_stmt->sibling;
        }

        if (last_expr) {
          current_stmt = cond_block_node->child;
          while (current_stmt && current_stmt != last_expr) {
            codegen_statement(current_stmt, current_fn);
            current_stmt = current_stmt->sibling;
          }

          if (last_expr->type == 20) {
            cond_value = codegen_expr(last_expr->child, current_fn);
          } else if (last_expr->type == 22) {
            cond_value = codegen_expr(last_expr, current_fn);
          }
        }
      }

      if (!cond_value) {
        //         fprintf(stderr, "Error: No se pudo generar condición. Usando condición falsa.\n");
        cond_value = LLVMConstInt(LLVMInt32Type(), 0, 0);
      }

      // Convertir a booleano
      LLVMValueRef bool_cond = LLVMBuildICmp(builder, LLVMIntNE, cond_value, 
                                             LLVMConstInt(LLVMInt32Type(), 0, 0), "do_cond");

      LLVMBuildCondBr(builder, bool_cond, body_block, after_block);

      // After
      LLVMPositionBuilderAtEnd(builder, after_block);

      break;

    }

    default:
    //       fprintf(stderr, "[codegen_statement] tipo no soportado %d (addr=%p)\n", stmt->type, (void*)stmt);
  }
}


// =======================================================
// BLOQUE
// =======================================================
void codegen_block(ast_node *block, LLVMValueRef function) {
  if (!block) {
    //     fprintf(stderr, "[codegen_block] block == NULL\n");
    return;
  }
  //   fprintf(stderr, "codegen_block: ENTER block=%p type=%d\n", (void*)block, block->type);
  if (block->type != NT_BLOQUE) {
    //     fprintf(stderr, "[codegen_block] ERROR: nodo no es BLOQUE (type=%d)\n", block->type);
    return;
  }
  ast_node *stmt = block->child;
  int idx = 0;
  while (stmt) {
    //     fprintf(stderr, "  BLOCK stmt #%d type=%d ptr=%p lineno=%d\n", idx, stmt->type, (void*)stmt, stmt->lineno);
    codegen_statement(stmt, function);
    //     fprintf(stderr, "  returned from stmt #%d\n", idx);
    stmt = stmt->sibling;
    idx++;
  }
  //   fprintf(stderr, "codegen_block: EXIT\n");
}


// =======================================================
// FUNCIÓN
// =======================================================
void codegen_function(ast_node *fn_node) {
  //   fprintf(stderr, "\n==== codegen_function INICIO ====\n");
  if (!fn_node) { //fprintf(stderr, "ERROR: fn_node NULL\n"); 
    return; }
  //   fprintf(stderr, "fn_node->type = %d\n", fn_node->type);

  ast_node *tipo_node = fn_node->child;
  ast_node *idnode = tipo_node ? tipo_node->sibling : NULL;
  if (!tipo_node || !idnode) { 
    //fprintf(stderr, "ERROR: función sin tipo o ID\n"); 
    return; }
  const char *fnname = idnode->value.strVal;
  //   fprintf(stderr, "Función nombre = %s\n", fnname);

  ast_node *after_id = idnode->sibling;
  //   fprintf(stderr, "after_id->type = %d (addr=%p)\n", after_id ? after_id->type : -1, (void*)after_id);

  // Mapear tipo de retorno
  LLVMTypeRef ret_type = map_type_node(tipo_node);
  if (!ret_type) { 
    //     fprintf(stderr, "ERROR: ret_type NULL, fallback i32\n"); 
    ret_type = LLVMInt32Type(); 
  }

  // Contar parámetros
  int nparams = 0;
  ast_node *it = after_id;
  while (it && it->type == NT_PARAMETRO) { nparams++; it = it->sibling; }
  //   fprintf(stderr, "Total parámetros = %d\n", nparams);

  LLVMTypeRef *param_types = NULL;
  if (nparams > 0) {
    param_types = malloc(sizeof(LLVMTypeRef) * nparams);
    if (!param_types) {
      //       fprintf(stderr, "ERROR: no se pudo allocar param_types\n");
      return;
    }

    // Llenar los tipos de parámetros
    it = after_id;
    int idx = 0;
    while (it && it->type == NT_PARAMETRO) {
      ast_node *ptype = it->child;
      LLVMTypeRef pt = map_type_node(ptype);
      if (!pt) {
        //         fprintf(stderr, "  param %d: tipo NULL, usando i32\n", idx);
        pt = LLVMInt32Type();
      }
      param_types[idx] = pt;
      //       fprintf(stderr, "  param %d llvm type = %p\n", idx, (void*)pt);
      idx++; 
      it = it->sibling;
    }
  }

  LLVMTypeRef fty = LLVMFunctionType(ret_type, param_types, nparams, 0);

  if (!fty) {
    //     fprintf(stderr, "ERROR: no se pudo crear function type\n");
    if (param_types) free(param_types);
    return;
  }

  //   fprintf(stderr, "Tipo de función creado: %p\n", (void*)fty);
  //   fprintf(stderr, "Kind del tipo de función: %d\n", LLVMGetTypeKind(fty));

  if (!module) {
    //     fprintf(stderr, "ERROR: module es NULL\n");
    if (param_types) free(param_types);
    return;
  }

  LLVMValueRef function = LLVMAddFunction(module, fnname, fty);
  //   fprintf(stderr, "LLVMAddFunction OK: %p\n", (void*)function);

  if (!function) {
    //     fprintf(stderr, "ERROR: no se pudo añadir función %s\n", fnname);
    if (param_types) free(param_types);
    return;
  }

  LLVMTypeRef actual_func_type = LLVMTypeOf(function);
  //   fprintf(stderr, "Tipo real de la función: %p\n", (void*)actual_func_type);
  if (actual_func_type) {
    //     fprintf(stderr, "Kind del tipo real: %d\n", LLVMGetTypeKind(actual_func_type));
    if (LLVMGetTypeKind(actual_func_type) == LLVMPointerTypeKind) {
      LLVMTypeRef element_type = LLVMGetElementType(actual_func_type);
      //       fprintf(stderr, "Tipo elemento: %p\n", (void*)element_type);
      if (element_type) {
        //         fprintf(stderr, "Kind del tipo elemento: %d\n", LLVMGetTypeKind(element_type));
      }
    }
  }

  // Crear entry block
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
  //   fprintf(stderr, "entry block = %p\n", (void*)entry);

  if (!builder) {
    //     fprintf(stderr, "ERROR: builder es NULL\n");
    if (param_types) free(param_types);
    return;
  }

  LLVMPositionBuilderAtEnd(builder, entry);
  sym_clear();

  // Procesar parámetros
  it = after_id;
  int idx = 0;
  while (it && it->type == NT_PARAMETRO) {
    ast_node *ptype = it->child;
    ast_node *pid = ptype ? ptype->sibling : NULL;
    const char *pname = pid ? pid->value.strVal : "(null)";
    LLVMTypeRef pt = map_type_node(ptype);
    if (!pt) pt = LLVMInt32Type();
    //     fprintf(stderr, "  Param %d = %s type=%p\n", idx, pname, (void*)pt);

    LLVMValueRef arg = LLVMGetParam(function, idx);
    //     fprintf(stderr, "    LLVMGetParam OK: %p\n", (void*)arg);

    LLVMValueRef a = create_entry_alloca(function, pname, pt);
    if (!a) { 
      //       fprintf(stderr, "    ERROR: alloca param NULL for %s\n", pname); 
    } else {
      if (arg && a) {
        LLVMBuildStore(builder, arg, a);
      } else {
        //         fprintf(stderr, "    WARNING: arg o alloca inválidos, omitiendo store\n");
      }
      sym_put(pname, a);
    }
    idx++; 
    it = it->sibling;
  }

  if (param_types) free(param_types);

  ast_node *body = it;
  //   fprintf(stderr, "Body detectado: type=%d addr=%p\n", body ? body->type : -1, (void*)body);

  if (body && body->type == NT_BLOQUE) {
    codegen_block(body, function);
  } else if (body) {
    codegen_statement(body, function);
  } else {
    LLVMBuildRet(builder, LLVMConstInt(i32_type, 0, 0));
  }

  // Verificar terminador
  if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
    LLVMBuildRet(builder, LLVMConstInt(i32_type, 0, 0));
  }

  //   fprintf(stderr, "==== codegen_function FIN ====\n");
}

// =======================================================
// MÓDULO
// =======================================================
int codegen_generate_module(ast_node *root, const char *filename) {
  //printf("[DEBUG] Iniciando generación de módulo\n");

  i32_type = LLVMInt32Type();
  i8_type  = LLVMInt8Type();
  f64_type = LLVMDoubleType();

  //printf("[DEBUG] Tipos básicos inicializados\n");

  module = LLVMModuleCreateWithName("mini_c_module");
  if (!module) {
    printf("[ERROR] No se pudo crear módulo\n");
    return -1;
  }

  //printf("[DEBUG] Módulo creado: %p\n", (void*)module);

  builder = LLVMCreateBuilder();
  if (!builder) {
    printf("[ERROR] No se pudo crear builder\n");
    LLVMDisposeModule(module);
    return -1;
  }

  //printf("[DEBUG] Builder creado: %p\n", (void*)builder);

  //printf("[DEBUG] Declarando printf...\n");
  LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
  LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, 1);
  LLVMValueRef printf_func = LLVMAddFunction(module, "printf", printf_type);
  //printf("[DEBUG] printf declarado: %p\n", (void*)printf_func);




  // Procesar funciones
  ast_node *fn = root->child;
  int function_count = 0;
  while (fn) {
    //     fprintf(stderr, "Procesando función #%d\n", function_count);
    if (fn->type == NT_FUNCION) {
      codegen_function(fn);
      function_count++;
    }
    fn = fn->sibling;
  }

  //   fprintf(stderr, "Procesadas %d funciones\n", function_count);

  // Verificar módulo
  char *err = NULL;
  //fprintf(stderr, "Verificando módulo...\n");
  LLVMVerifyModule(module, LLVMAbortProcessAction, &err);
  if (err) {
    //fprintf(stderr, "Error verificando módulo: %s\n", err);
    LLVMDisposeMessage(err);
    // Continuar a pesar del error
  }



  // 1. Obtener Triple y Target
  char *triple = LLVMGetDefaultTargetTriple();
  LLVMTargetRef target;
  if (LLVMGetTargetFromTriple(triple, &target, &err) != 0) {
    //     fprintf(stderr, "ERROR Target: %s\n", err);
    LLVMDisposeMessage(err);
    LLVMDisposeMessage(triple);
    return -1;
  }

  // 2. Crear Target Machine
  LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(
    target, triple, "generic", "",
    LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault
  );

  if (!target_machine) {
    //     fprintf(stderr, "ERROR: Falló LLVMCreateTargetMachine\n");
    LLVMDisposeMessage(triple);
    return -1;
  }



  // 3. Configurar Data Layout
  LLVMTargetDataRef data_layout = LLVMCreateTargetDataLayout(target_machine);


  LLVMSetModuleDataLayout(module, data_layout);


  LLVMDisposeTargetData(data_layout);

  // 4. Emitir archivo .o
  //   fprintf(stderr, "Emitiendo objeto a: %s\n", filename);
  if (LLVMTargetMachineEmitToFile(target_machine, module, (char*)filename, LLVMObjectFile, &err) != 0) {
    //     fprintf(stderr, "ERROR emitiendo archivo: %s\n", err);
    LLVMDisposeMessage(err);
    LLVMDisposeTargetMachine(target_machine);
    LLVMDisposeMessage(triple);
    return -1;
  }

  // 5. Limpieza Final
  LLVMDisposeTargetMachine(target_machine);
  LLVMDisposeMessage(triple);
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);

  //   fprintf(stderr, "EXITO: .o generado.\n");
  return 0;
}




