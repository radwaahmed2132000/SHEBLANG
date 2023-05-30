#include <variant>
#include "nodes.h"

#define NODE_TYPES                                                             \
    conNodeType, idNodeType, switchNodeType, caseNodeType, breakNodeType,      \
        FunctionDefn, whileNodeType, forNodeType, doWhileNodeType, VarDecl,    \
        VarDefn, enumNode, IdentifierListNode, enumUseNode, StatementList,     \
        FunctionCall, ExprListNode, BinOp, UnOp, IfNode

struct Node : std::variant<NODE_TYPES> {
    int lineNo;
    std::string conversionType; 
    bool addNewScope;
    ScopeSymbolTables* currentScope;

    Node() = default;
    Node(std::variant<NODE_TYPES> inner_union, int lineNo);
	Node(std::variant<NODE_TYPES> inner_union, int lineNo, bool addNewScope);

    template<typename T> 
    bool is() const { return std::holds_alternative<T>(*this); }

    template<typename T>
    T* asPtr() { return std::get_if<T>(this); }

    template<typename T>
    T& as() { return std::get<T>(*this); }
};
