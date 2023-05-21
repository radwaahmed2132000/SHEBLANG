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
    std::vector<struct nodeTypeTag*> op;
} oprNodeType;

typedef struct {
    struct nodeTypeTag* self, *prev;
} caseNodeType;

typedef struct {
    int exit_label;
    bool break_encountered;
    struct nodeTypeTag* var, *case_list_head;
} switchNodeType;

typedef struct {
    struct nodeTypeTag* parent_switch;
} breakNodeType;

typedef struct {
    struct nodeTypeTag* return_type;
    struct nodeTypeTag* name;
    std::vector<struct VarDecl*> parameters;
    struct nodeTypeTag* statements;
} functionNodeType;

typedef struct VarDecl : LinkedListNode<VarDecl>{
	struct nodeTypeTag *type, *var_name;

	VarDecl(nodeTypeTag* type, nodeTypeTag* var_name) : type(type), var_name(var_name) {}

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
    struct nodeTypeTag *condition, *loop_body;
} whileNodeType;

// Exactly the same as whileNodeType,
// the main difference is in the behaviour itself.
// But, we can't have two elements of the same type in std::variant, so I had to
// split them.
typedef struct {
    bool break_encountered;
    struct nodeTypeTag *condition, *loop_body;
} doWhileNodeType;

typedef struct {
    bool break_encountered;
    struct nodeTypeTag *init_statement, *loop_condition, *post_loop_statement, *loop_body;
} forNodeType;

#define NODE_TYPES \
	conNodeType, idNodeType, oprNodeType, switchNodeType,  \
	caseNodeType, breakNodeType, functionNodeType, whileNodeType, \
	forNodeType, doWhileNodeType, VarDecl

typedef struct nodeTypeTag {
    std::variant<NODE_TYPES> un;
	nodeTypeTag(std::variant<NODE_TYPES> inner_union) : un(inner_union) {}
} nodeType;

extern std::unordered_map<std::string, Value> sym2;
extern std::unordered_map<std::string, functionNodeType> functions;

// Must include after the structs are defined
#include "node_constructors.h"

// Forward declare `semantic_analysis` for use in cl.y
Result semantic_analysis(nodeType* p);

void set_break_parent(nodeType* node, nodeType* parent_switch);