#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdarg.h>

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "cl.h"
#include "node_constructors.h"


#define CON_INIT(ptr_name, value) new nodeType(std::variant<NODE_TYPES>(conNodeType{value}), currentLineNo)

nodeType *con(int iValue)   { return CON_INIT(p, iValue); }
nodeType *con(float fValue) { return CON_INIT(p, fValue); }
nodeType *con(bool bValue)  { return CON_INIT(p, bValue); }
nodeType *con(char cValue)  { return CON_INIT(p, cValue); }
nodeType *con(char* sValue) { return CON_INIT(p, std::string(sValue)); }

// Create an operator node with a variable number of inputs (usually 2)
// Note that `oper` is an enum value (or #ifdef), you can find the definintions in `parser.h`.
nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    int i;

    /* copy information */
    auto opr = oprNodeType{};

    opr.oper = oper;
    va_start(ap, nops);
    for (i = 0; i < nops; i++) {
        opr.op.push_back(va_arg(ap, nodeType*));
    }
    va_end(ap);

    return new nodeType(opr, currentLineNo);
}

// Create an identifier node.
nodeType *id(const char* id) {
    /* copy information */
    auto idNode = idNodeType{std::string(id)};
    return new nodeType(idNode, currentLineNo);
}

nodeType *sw(nodeType* var, nodeType* caseListTail) {
    assert(caseListTail->is<caseNodeType>());

    auto switchNode = std::variant<NODE_TYPES>(switchNodeType{ 0, var, caseListTail });

    return new nodeType(switchNode, currentLineNo);
}

nodeType *cs(nodeType* labelExpr, nodeType* caseBody) {
    return new nodeType(caseNodeType(labelExpr, caseBody), currentLineNo);
}

nodeType *br() {
    auto breakNode = std::variant<NODE_TYPES>(breakNodeType{NULL});
    return new nodeType(breakNode, currentLineNo);
}

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body) {
    auto forNode = forNodeType{false, init_statement, loop_condition, post_loop_statement, loop_body};
    return new nodeType(forNode, currentLineNo);
}

nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body) {
    auto whileNode = whileNodeType{false, loop_condition, loop_body};
    return new nodeType(whileNode, currentLineNo);
}

nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body) {
    auto doWhileNode = std::variant<NODE_TYPES>(doWhileNodeType{false, loop_condition, loop_body});
    return new nodeType(doWhileNode, currentLineNo);
}

nodeType* varDecl(nodeType* type, nodeType* name) {
    auto nameStr = name->as<idNodeType>().id;
    auto typeStr = type->as<idNodeType>().id;

    return new nodeType(VarDecl(type, name), currentLineNo);
}

nodeType* varDefn(nodeType* decl, nodeType* initExpr, bool isConstant) {
    auto* declPtr = decl->asPtr<VarDecl>();
    return new nodeType(VarDefn(declPtr, initExpr, isConstant), currentLineNo);
}

std::string VarDecl::getType() const { return type->as<idNodeType>().id; } 
std::string VarDecl::getName() const { return var_name->as<idNodeType>().id; } 


nodeType* fn(nodeType* name, VarDecl* paramsTail, nodeType* return_type, nodeType* statements) {
    assert(name->is<idNodeType>());
    assert(return_type->is<idNodeType>());
    assert(statements->is<StatementList>());

    return new nodeType(functionNodeType{return_type, name, paramsTail, statements}, currentLineNo);
}

nodeType* fn_call(nodeType* name) {
    return new nodeType(functionNodeType{nullptr, name}, currentLineNo);
}

struct set_break_parent_visitor {
    nodeType *parent_switch;

    void operator()(breakNodeType &b) const { b.parent_switch = parent_switch; }

    void operator()(oprNodeType &opr) const {
        for (int i = 0; i < opr.op.size(); i++) {
            set_break_parent(opr.op[i], parent_switch);
        }
    }

    void operator()(StatementList& sl) const {
        for(const auto* statement: sl.toVec()) {
            set_break_parent(statement->statementCode, parent_switch);
        }
    }

    void operator()(caseNodeType &c) const {
        auto cases = c.toVec();
        for (const auto *cs : cases) {
            set_break_parent(cs->caseBody, parent_switch);
        }
    }

    // the default case:
    template <typename T> void operator()(T const & /*UNUSED*/) const {}
};

void set_break_parent(nodeType* node, nodeType* parent_switch) {
    if(node == NULL) return;
    std::visit(set_break_parent_visitor{parent_switch}, *node);
}

nodeType* enum_defn(nodeType* enumIdentifier, std::vector<IdentifierListNode*>& members) {
    assert(enumIdentifier->is<idNodeType>());

    std::vector<std::string> memberNames;
    memberNames.reserve(members.size());
    for(const auto& node: members) { memberNames.emplace_back(node->identifierName->id); }

    // Needed now so the interpreter works correctly..
    auto e = enumNode{enumIdentifier, memberNames};
    auto enumName =  enumIdentifier->as<idNodeType>().id;

    return new nodeType(e, currentLineNo);
}

nodeType* enum_use(nodeType* enumIdentifier, nodeType* enumMemberIdentifier) {
    auto enumName =  enumIdentifier->as<idNodeType>().id;
    auto enumMemberName =  enumMemberIdentifier->as<idNodeType>().id;

    return new nodeType(enumUseNode{enumName, enumMemberName, currentLineNo}, currentLineNo);
}

nodeType* identifierListNode(nodeType* idNode, bool addNewScope) {
    assert(idNode->is<idNodeType>());
    return new nodeType(IdentifierListNode(idNode->asPtr<idNodeType>()), currentLineNo ,addNewScope);
}

nodeType* statementList(nodeType* statement) {
    return new nodeType(StatementList(statement), currentLineNo);
}

nodeType* exprListNode(nodeType* exprCode) {
    return new nodeType(ExprListNode(exprCode), currentLineNo);
}

nodeType* functionCall(nodeType* fnIdentifier, nodeType* exprListTail) {
    assert(exprListTail->is<ExprListNode>());
    assert(fnIdentifier->is<idNodeType>());

    auto exprList = exprListTail->as<ExprListNode>().toVec();
    auto fnName = fnIdentifier->as<idNodeType>().id;

    return new nodeType(FunctionCall{fnName, exprList, currentLineNo}, currentLineNo);
}
