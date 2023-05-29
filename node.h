#include <variant>
#include "nodes.h"

#define NODE_TYPES                                                      \
    conNodeType, idNodeType, oprNodeType, switchNodeType, caseNodeType, \
    breakNodeType, functionNodeType, whileNodeType, forNodeType,        \
    doWhileNodeType, VarDecl, VarDefn, enumNode, IdentifierListNode,    \
    enumUseNode, StatementList, FunctionCall, ExprListNode, BinOp

typedef struct nodeType : std::variant<NODE_TYPES> {
    int lineNo;
    std::string conversionType; 
    bool addNewScope;
    ScopeSymbolTables* currentScope;

    nodeType() = default;
    nodeType(std::variant<NODE_TYPES> inner_union, int lineNo);
	nodeType(std::variant<NODE_TYPES> inner_union, int lineNo, bool addNewScope);

    template<typename T> 
    bool is() const { return std::holds_alternative<T>(*this); }

    template<typename T>
    T* asPtr() { return std::get_if<T>(this); }

    template<typename T>
    T& as() { return std::get<T>(*this); }
} nodeType;
