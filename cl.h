#pragma once

#include <cstddef>
#include <cstdio>
#include <algorithm>
#include <memory>
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

void yyerror(char *s);

template <typename T>
struct LinkedListNode {
    T * prev = nullptr;
    LinkedListNode() = default;
    LinkedListNode(T* p) : prev(p) {}

    
	std::vector<T*> toVec() {
		std::vector<T*> nodes;

		T* current = static_cast<T*>(this);
		while(current != nullptr) {
			nodes.push_back(current);
			current = current->prev;
		}
	
        std::reverse(nodes.begin(), nodes.end());
		return nodes;
	}
};

/* constants */
// Operators needed: +, -, *, /, %, &, |, ^, &&, ||, !
using conNodeType = Value;

/* identifiers */
typedef struct {
    std::string id;                 /* key/index to sym array */
    int lineNo;
    struct nodeType* scopeNodePtr;         /* Used purely for scoping purposes */
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    std::vector<struct nodeType*> op;
} oprNodeType;

typedef struct caseNodeType: LinkedListNode<caseNodeType> {
    struct  nodeType* labelExpr, * caseBody;

    caseNodeType(): caseBody(nullptr), LinkedListNode(nullptr) {}
    caseNodeType(nodeType* labelExpr, nodeType* caseBody): labelExpr(labelExpr), caseBody(caseBody) {}

    bool isDefault() const { return (labelExpr == nullptr); }
} caseNodeType;

typedef struct {
    int exit_label;
    // TODO: remove break_encountered since switches break automatically on case completion.
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

typedef struct IdentifierListNode: LinkedListNode<IdentifierListNode> {
    idNodeType* identifierName;
    IdentifierListNode(): identifierName(nullptr), LinkedListNode(nullptr) {}
    IdentifierListNode(idNodeType* idNode): identifierName(idNode), LinkedListNode(nullptr) {}

} IdentifierListNode;

typedef struct enumNode {
    nodeType* name;
    std::vector<std::string> enumMembers;
} enumNode;

typedef struct enumUseNode {
    std::string enumName, memberName;
} enumUseNode;

typedef struct StatementList: LinkedListNode<StatementList> {
    struct nodeType* statementCode;

    StatementList(): statementCode(nullptr) {}

    StatementList(nodeType* statementCode): statementCode(statementCode) {}
} StatementList;

typedef struct ExprListNode: LinkedListNode<ExprListNode> {
    struct nodeType* exprCode;

    ExprListNode(): exprCode(nullptr) {}
    ExprListNode(nodeType* exprCode): exprCode(exprCode) {}

} ExprListNode;

typedef struct FunctionCall {
    std::string functionName;
    std::vector<ExprListNode*> parameterExpressions;    
} FunctionCall;

typedef struct VarDecl : LinkedListNode<VarDecl>{
	struct nodeType *type, *var_name;
    bool isConstant = false;
    struct nodeType* initExpr = nullptr;

    VarDecl(): type(nullptr), var_name(nullptr), LinkedListNode(nullptr) {}
	VarDecl(nodeType* type, nodeType* var_name) : type(type), var_name(var_name) {}
    VarDecl(nodeType* type, nodeType* var_name, nodeType* initExpr, bool isConstant) : type(type), var_name(var_name), initExpr(initExpr), isConstant(isConstant) {}

    std::string getType() const;
    std::string getName() const;
} VarDecl;

typedef struct VarDefn {
	VarDecl* decl;
    bool isConstant;
    struct nodeType* initExpr = nullptr;

	VarDefn(VarDecl* decl, nodeType* initExpr, bool isConstant) : decl(decl), initExpr(initExpr), isConstant(isConstant) {}
} VarDefn;

typedef struct SymbolTableEntry {
    Value value;
    bool isConstant;
    bool isUsed = false;
    int declaredAtLine = -1;
    std::string type = "<no type>";  
    nodeType* initExpr = nullptr;

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

struct ScopeSymbolTables {
    static int tableCount;
    int tableId;
    ScopeSymbolTables() {
        tableId = ++tableCount;
        parentScope = nullptr;
    }
    std::unordered_map<std::string, SymbolTableEntry> sym2;
    std::unordered_map<std::string, functionNodeType> functions;
    std::unordered_map<std::string, enumNode> enums;
    ScopeSymbolTables* parentScope;
};

#define NODE_TYPES                                                      \
    conNodeType, idNodeType, oprNodeType, switchNodeType, caseNodeType, \
    breakNodeType, functionNodeType, whileNodeType, forNodeType,        \
    doWhileNodeType, VarDecl, VarDefn, enumNode, IdentifierListNode,    \
    enumUseNode, StatementList, FunctionCall, ExprListNode              

typedef struct nodeType {
    std::variant<NODE_TYPES> un;
    int lineNo;
    std::string conversionType = ""; 
    bool addNewScope;
    ScopeSymbolTables* currentScope;
    nodeType(std::variant<NODE_TYPES> inner_union, int lineNo) : un(inner_union), lineNo(lineNo), currentScope(nullptr) {}
	nodeType(std::variant<NODE_TYPES> inner_union, int lineNo, bool addNewScope) : un(inner_union), lineNo(lineNo), addNewScope(addNewScope), currentScope(nullptr) {}
} nodeType;

inline Result errorsOutput;
inline Result warningsOutput;

// Forward declare `semantic_analysis` for use in cl.y
Result semantic_analysis(nodeType* p);
void setup_scopes(nodeType* p);
void set_break_parent(nodeType* node, nodeType* parent_switch);

// Tracking line numbers
inline int currentLineNo = 1;

// Must include after the structs are defined
#include "node_constructors.h"


// Scope analysis should guarantee that we can find the variable in some parent scope.
static SymbolTableEntry& varSymTableEntry(std::string variableName, ScopeSymbolTables* scope) {
    auto& symbols = scope->sym2;

    if(symbols.find(variableName) != symbols.end()) {
        return symbols[variableName];
    } 

    return varSymTableEntry(variableName, scope->parentScope);
}

static auto& enumSymTable(std::string enumName, ScopeSymbolTables* scope) {
    auto& enums = scope->enums;

    if(enums.find(enumName) != enums.end()) {
        return enums;
    } 

    return enumSymTable(enumName, scope->parentScope);
}

static auto& fnSymTable(std::string fnName, ScopeSymbolTables* scope) {
    auto& functions = scope->functions;

    if(functions.find(fnName) != functions.end()) {
        return functions;
    } 

    return fnSymTable(fnName, scope->parentScope);
}

static enumNode& enumSymTableEntry(std::string enumName, ScopeSymbolTables* scope) {
    return enumSymTable(enumName, scope)[enumName];
}

static functionNodeType& fnSymTableEntry(std::string enumName, ScopeSymbolTables* scope) {
    return fnSymTable(enumName, scope)[enumName];
}
