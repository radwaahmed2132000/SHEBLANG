#pragma once

#include <cstdio>
#include <algorithm>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>

#include "value.h"
#include "result.h"

// Tracking line numbers
inline int currentLineNo = 1;

inline std::vector<std::string> errors;
inline std::vector<std::string> warnings;
inline std::string filePath;

#include "node.h" // Must include after the structs are defined
// Needs `currentLineNo` to be defined
#include "node_constructors.h"
#include "template_utils.h"

void yyerror(char *s);
void printSymbolTables();
void appendSymbolTable(int i);
// Forward declare `semantic_analysis` for use in cl.y
Result<Type, std::vector<std::string>> semantic_analysis(Node* p);
void setup_scopes(Node* p);
void set_break_parent(Node* node, Node* parent_switch);

template<typename  S>
void error(Result<S, std::vector<std::string>> errs) { auto e = std::get<std::vector<std::string>>(errs); Utils::extendVector(errors, e); }

template<typename  S>
void warning(Result<S, std::vector<std::string>> warns) { auto w = std::get<std::vector<std::string>>(warns); Utils::extendVector(warnings, w); }


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

static FunctionDefn& getFnEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    auto* fnScope = getSymbolScope(symbol, scope);
    assert(fnScope);
    return fnScope->functions[symbol];
}

static enumNode& getEnumEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    auto* enumScope = getSymbolScope(symbol, scope);
    assert(enumScope);
    return enumScope->enums[symbol];
}

struct ControlFlow {
    enum class Type {
        Continue,
        Return,
    };

    Value val;
    Type type;

    ControlFlow() = default;
    ControlFlow(const Value& val): val(val), type(Type::Continue) {}
    ControlFlow(const Value& val, const Type& type): val(val), type(type) {}

    ControlFlow& operator=(const Value& val) { this->val = val;  return *this; }

    static ControlFlow Return(const Value&& val) { return ControlFlow{val, Type::Return}; }

    bool operator&&(const ControlFlow& other) const { return val && other.val; }
    bool operator||(const ControlFlow& other) const { return val || other.val; }
    bool operator!() const { return !val; }

    explicit operator bool() const {
        return bool(val);
    }
};

ControlFlow ex(Node* p);
