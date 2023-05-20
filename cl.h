#pragma once
#include <cstdio>
#include <new>
#include <system_error>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>

#include "value.h"

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
    // TODO: some sort of list for parameters
    struct nodeTypeTag* statemenst;
} functionNodeType;

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

#define NODE_TYPES conNodeType, idNodeType, oprNodeType, switchNodeType, caseNodeType, breakNodeType, functionNodeType, whileNodeType, forNodeType, doWhileNodeType

typedef struct nodeTypeTag {
    std::variant<NODE_TYPES> un;
} nodeType;

extern float sym[26];
extern std::unordered_map<std::string, Value> sym2;

// Forward declare `semantic_analysis` for use in cl.y
Value semantic_analysis(nodeType* p);