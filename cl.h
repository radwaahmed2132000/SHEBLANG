#pragma once

#include <cstdio>
#include <algorithm>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <assert.h>

#include "value.h"
#include "result.h"

void yyerror(char *s);

template <typename T>
struct LinkedListNode {
    T * prev = nullptr;
};

/* constants */
// Operators needed: +, -, *, /, %, &, |, ^, &&, ||, !
using conNodeType = Value;

/* identifiers */
typedef struct {
    std::string id;                 /* key/index to sym array */
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    std::vector<struct nodeType*> op;
} oprNodeType;

typedef struct {
    struct nodeType* self, *prev;
} caseNodeType;

typedef struct {
    int exit_label;
    bool break_encountered;
    struct nodeType* var, *case_list_head;
} switchNodeType;

typedef struct {
    struct nodeType* parent_switch;
} breakNodeType;

typedef struct {
    struct nodeType* return_type;
    struct nodeType* name;
    std::vector<struct VarDecl*> parameters;
    struct nodeType* statements;
} functionNodeType;

typedef struct VarDecl : LinkedListNode<VarDecl>{
	struct nodeType *type, *var_name;

	VarDecl(nodeType* type, nodeType* var_name) : type(type), var_name(var_name) {}

    std::string getType() const;
    std::string getName() const;

    bool isStump() const {
        return (this->type == nullptr) && (this->var_name == nullptr);
    }

	std::vector<VarDecl*> toVec() {
		std::vector<VarDecl*> nodes;

		VarDecl* current = this;
		while(current != nullptr && !current->isStump()) {
			nodes.push_back(current);
			current = current->prev;
		}
	
        std::reverse(nodes.begin(), nodes.end());
		return nodes;
	}
} VarDecl;

typedef struct {
    bool break_encountered;
    struct nodeType *condition, *loop_body;
} whileNodeType;

// Exactly the same as whileNodeType,
// the main difference is in the behaviour itself.
// But, we can't have two elements of the same type in std::variant, so I had to
// split them.
typedef struct {
    bool break_encountered;
    struct nodeType *condition, *loop_body;
} doWhileNodeType;

typedef struct {
    bool break_encountered;
    struct nodeType *init_statement, *loop_condition, *post_loop_statement, *loop_body;
} forNodeType;

#define NODE_TYPES \
    conNodeType, idNodeType, oprNodeType, switchNodeType, \
    caseNodeType, breakNodeType, functionNodeType, whileNodeType, \
    forNodeType, doWhileNodeType, VarDecl

typedef struct nodeType {
    std::variant<NODE_TYPES> un;
	nodeType(std::variant<NODE_TYPES> inner_union) : un(inner_union) {}
} nodeType;

typedef struct SymbolTableEntry {
    Value value;
    bool isConstant;
    std::string type;  
    nodeType* initExpr;

    SymbolTableEntry() = default;

    SymbolTableEntry(nodeType* initExpr, bool isConstant, std::string type):
        initExpr(initExpr), isConstant(isConstant), type(type) {}
    
    SymbolTableEntry(bool isConstant, std::string type):
        value(0), isConstant(isConstant), type(type) {}

    SymbolTableEntry& setValue(Value v) {
        value = v;
        return *this;
    }

    Value getValue() const { return value; }

    Value& getRef() { return value; }
} SymbolTableEntry;

extern float sym[26];
extern std::unordered_map<std::string, SymbolTableEntry> sym2;
extern std::unordered_map<std::string, functionNodeType> functions;

// Forward declare `semantic_analysis` for use in cl.y
Result semantic_analysis(nodeType* p);
void set_break_parent(nodeType* node, nodeType* parent_switch);

// Must include after the structs are defined
#include "node_constructors.h"
