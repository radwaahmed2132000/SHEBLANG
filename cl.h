#pragma once

#include <cstdio>
#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <assert.h>
#include <new>
#include <system_error>

#include "value.h"
#include "result.h"

// Tracking line numbers
inline int currentLineNo = 1;

inline Result errorsOutput;
inline Result warningsOutput;

#include "node.h" // Must include after the structs are defined
// Needs `currentLineNo` to be defined
#include "node_constructors.h"

void yyerror(char *s);
void printSymbolTables();
void appendSymbolTable(int i);
// Forward declare `semantic_analysis` for use in cl.y
Result semantic_analysis(nodeType* p);
void setup_scopes(nodeType* p);
void set_break_parent(nodeType* node, nodeType* parent_switch);


// Scope analysis should guarantee that we can find the variable in some parent scope.
static ScopeSymbolTables* getSymbolScope(const std::string& symbol, ScopeSymbolTables* scope) {
    if(scope == nullptr) return nullptr;

    auto sym = scope->sym2;
    auto fn = scope->functions;
    auto en = scope->enums;

    auto symFound = sym.find(symbol) != sym.end();
    auto fnFound = fn.find(symbol) != fn.end();
    auto enFound = en.find(symbol) != en.end();

    if(symFound || fnFound || enFound) {
        return scope;
    } 

    return getSymbolScope(symbol, scope->parentScope);
}


// Used to get the symbol table entry for a specific variable.
// For use in the compiler and interpereter only.
static SymbolTableEntry& getSymEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    auto* symbolScope = getSymbolScope(symbol, scope);
    assert(symbolScope);
    return symbolScope->sym2[symbol];
}

static functionNodeType& getFnEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    auto* fnScope = getSymbolScope(symbol, scope);
    assert(fnScope);
    return fnScope->functions[symbol];
}

static enumNode& getEnumEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    auto* enumScope = getSymbolScope(symbol, scope);
    assert(enumScope);
    return enumScope->enums[symbol];
}
