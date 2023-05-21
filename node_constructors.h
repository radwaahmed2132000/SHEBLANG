#pragma once
#include "cl.h"

nodeType *con(int iValue);
nodeType *con(float fValue);
nodeType *con(bool bValue);
nodeType *con(char cValue);
nodeType *con(char* sValue);

nodeType *opr(int oper, int nops, ...);
nodeType *id(const char* id);

nodeType *sw(nodeType* var, nodeType* case_list_head);
nodeType *cs(nodeType* self, nodeType* prev);
nodeType *br();

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body);
nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body);
nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body);

nodeType* varDecl(nodeType* type, nodeType* name);

nodeType* fn(nodeTypeTag* name, std::vector<VarDecl*>& params, nodeTypeTag* return_type, nodeType* statements);
nodeType* fn_call(nodeTypeTag* name);

nodeType* fnParamList(nodeType* end);
nodeType* fnParamList(nodeType* prev, nodeType* next);