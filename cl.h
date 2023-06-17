#pragma once

#include <cstdio>
#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>

#include "nodes.h"
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
Result<Type, std::vector<std::string>> semanticAnalysis(Node* p);
void setup_scopes(Node* p);
void set_break_parent(Node* node, Node* parent_switch);

template<typename  S>
void error(Result<S, std::vector<std::string>> errs) { auto e = std::get<std::vector<std::string>>(errs); Utils::extendVector(errors, e); }

template<typename  S>
void warning(Result<S, std::vector<std::string>> warns) { auto w = std::get<std::vector<std::string>>(warns); Utils::extendVector(warnings, w); }


enum class SymbolType {
    Variable,
    Function,
    Enum,
    Invalid
};

struct SymTypeScope: std::unordered_map<SymbolType, ScopeSymbolTables*> {
    bool hasType(SymbolType type) {
        return !this->empty() && this->contains(type);
    }

   ScopeSymbolTables* getTypeScope(SymbolType type) {
        return  (*this)[type];
   }
};

static std::pair<ScopeSymbolTables*, SymbolType> getSymbolScopeUtil(
    const std::string &symbolName,
    ScopeSymbolTables *startingScope
) {
    if (startingScope == nullptr) return {nullptr, SymbolType::Invalid};

    std::vector<std::pair<bool, SymbolType>> symbolFound = {
        {startingScope->sym2.contains(symbolName), SymbolType::Variable},
        {startingScope->functions.contains(symbolName), SymbolType::Function},
        {startingScope->enums.contains(symbolName), SymbolType::Enum},
    };

    auto foundIter =
        std::find_if(symbolFound.begin(), symbolFound.end(), [](auto pair) {
            return pair.first == true;
        });

    // No symbol found
    if(foundIter == symbolFound.end()) { return getSymbolScopeUtil(symbolName, startingScope->parentScope); }

    return {startingScope, foundIter->second};
}

// Scope analysis should guarantee that we can find the variable in some parent scope.
static std::pair<ScopeSymbolTables*, SymbolType> getSymbolScope(const std::string& symbol, ScopeSymbolTables* scope) {
    return getSymbolScopeUtil(symbol, scope);
}

#define GET_ENTRY(symbolName, scope, symbolType, table)                        \
    auto [symMap, symType] = getSymbolScope(symbolName, scope);                \
    assert(symMap != nullptr);                                                 \
    assert(symType == symbolType);                                             \
    return symMap->table[symbol];

// Used to get the symbol table entry for a specific variable.
// For use in the compiler and interpereter only. As it assumes the given symbol names exist.
static SymbolTableEntry& getVarEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    GET_ENTRY(symbol, scope, SymbolType::Variable, sym2);
}

static FunctionDefn& getFnEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    GET_ENTRY(symbol, scope, SymbolType::Function, functions);
}

static enumNode& getEnumEntry(const std::string& symbol, ScopeSymbolTables* scope) {
    GET_ENTRY(symbol, scope, SymbolType::Enum, enums);
}

static std::string getFnType(const FunctionDefn& fn) {
    auto returnType = fn.return_type->as<Type>().innerType;
    auto declList = fn.parametersTail->asPtr<VarDecl>()->toVec();

    std::stringstream ss;
    ss << '(';
    for (int i = 0; i < declList.size(); i++) {
        auto paramType = declList[i]->getType().innerType;

        ss << paramType; 

        if(i < declList.size() - 1) { ss << ", "; }
    }
    ss << ')';
    ss << " -> " << returnType;

    return ss.str();
}

struct ControlFlow {
    enum class Type {
        Continue,
        Return,
    };

    const Type type;

    template<typename T>
    ControlFlow(T val): _val(val), type(Type::Continue) {}

    ControlFlow(Value&& val): _val(val), type(Type::Continue) {}
    ControlFlow(Value&& val, const Type type): _val(val), type(type) {}

    ControlFlow& operator=(Value& val) { this->_val = val;  return *this; }

    static ControlFlow Return(Value&& val) { return ControlFlow(std::forward<Value>(val), Type::Return); }

    bool operator==(const ControlFlow& other) const { return (_val == other._val) && (type == other.type); }

    bool operator&&(const ControlFlow& other) const { return _val && other._val; }
    bool operator||(const ControlFlow& other) const { return _val || other._val; }
    bool operator!() const { return !_val; }

    explicit operator bool() const {
        return bool(_val);
    }

    Value &val() { return _val; }
    operator Value() const { return _val; }

  private:
    Value _val;
};

ControlFlow ex(Node* p);
