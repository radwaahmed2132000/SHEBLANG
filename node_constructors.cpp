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

#include "y.tab.h"

extern int yylineno;  

#define CON_INIT(ptr_name, value) new nodeType(std::variant<NODE_TYPES>(conNodeType{value}),yylineno)

nodeType *con(int iValue)   { return CON_INIT(p, iValue); }
nodeType *con(float fValue) { return CON_INIT(p, fValue); }
nodeType *con(bool bValue)  { return CON_INIT(p, bValue); }
nodeType *con(char cValue)  { return CON_INIT(p, cValue); }
nodeType *con(char* sValue) { return CON_INIT(p, std::string(sValue)); }

// Create an operator node with a variable number of inputs (usually 2)
// Note that `oper` is an enum value (or #ifdef), you can find the definintions in `y.tab.h`.
nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    int i;

    /* copy information */
    auto opr = oprNodeType{};

    opr.oper = oper;
    va_start(ap, nops);
    for (i = 0; i < nops; i++)
        opr.op.push_back(va_arg(ap, nodeType*));
    va_end(ap);

    return new nodeType(opr, yylineno);
}

// Create an identifier node.
nodeType *id(const char* id) {
    /* copy information */
    auto idNode = idNodeType{std::string(id)};
    return new nodeType(idNode, yylineno);
}

nodeType *sw(nodeType* var, nodeType* case_list_head) {
    auto switchNode = std::variant<NODE_TYPES>(switchNodeType{ 0, false, var, case_list_head });
    return new nodeType(switchNode, yylineno);
}

nodeType *cs(nodeType* labelExpr, nodeType* caseBody) {
    return new nodeType(caseNodeType(labelExpr, caseBody), yylineno);
}

nodeType *br() {
    auto breakNode = std::variant<NODE_TYPES>(breakNodeType{NULL});
    return new nodeType(breakNode, yylineno);
}

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body) {
    auto forNode = forNodeType{false, init_statement, loop_condition, post_loop_statement, loop_body};
    return new nodeType(forNode, yylineno);
}

nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body) {
    auto whileNode = whileNodeType{false, loop_condition, loop_body};
    return new nodeType(whileNode, yylineno);
}

nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body) {
    auto doWhileNode = std::variant<NODE_TYPES>(doWhileNodeType{false, loop_condition, loop_body});
    return new nodeType(doWhileNode, yylineno);
}

nodeType* varDecl(nodeType* type, nodeType* name) {
    auto nameStr = std::get<idNodeType>(name->un).id;
    auto typeStr = std::get<idNodeType>(type->un).id;

    return new nodeType(VarDecl(type, name), yylineno);
}

nodeType* varDefn(nodeType* decl, nodeType* initExpr, bool isConstant) {
    auto* declPtr = std::get_if<VarDecl>(&decl->un);
    return new nodeType(VarDefn(declPtr, initExpr, isConstant), yylineno);
}

std::string VarDecl::getType() const { return std::get<idNodeType>(type->un).id; } 
std::string VarDecl::getName() const { return std::get<idNodeType>(var_name->un).id; } 


nodeType* fn(nodeType* name, std::vector<VarDecl*>& params, nodeType* return_type, nodeType* statements) {
    assert(std::holds_alternative<idNodeType>(name->un));
    assert(std::holds_alternative<idNodeType>(return_type->un));
    assert(std::holds_alternative<StatementList>(statements->un));

    auto returnType = std::get<idNodeType>(return_type->un).id;

    // TODO: probably move this to semantic analysis
    if(returnType != "void") {
        auto stmts = std::get<StatementList>(statements->un).toVec();
        auto isReturn = [](const StatementList* s) {
            if (const auto *opr = std::get_if<oprNodeType>(&s->statementCode->un); opr) {
                return opr->oper == RETURN;
            }
            return false;
        };
        bool containsReturn = std::any_of(stmts.begin(), stmts.end(), isReturn);

        assert(containsReturn);
    }

    return new nodeType(functionNodeType{return_type, name, params, statements}, yylineno);
}

nodeType* fn_call(nodeType* name) {
    return new nodeType(functionNodeType{nullptr, name}, yylineno);
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
    std::visit(set_break_parent_visitor{parent_switch}, node->un);
}

nodeType* enum_defn(nodeType* enumIdentifier, std::vector<IdentifierListNode*>& members) {
    assert(std::holds_alternative<idNodeType>(enumIdentifier->un));

    std::vector<std::string> memberNames;
    memberNames.reserve(members.size());
    for(const auto& node: members) { memberNames.emplace_back(node->identifierName->id); }

    // TODO: move to semantic analysis
    // Needed now so the interpreter works correctly..
    auto e = enumNode{enumIdentifier, memberNames};
    auto enumName = std::get<idNodeType>(enumIdentifier->un).id;
    enums[enumName] = e;

    return new nodeType(e, yylineno);
}

nodeType* enum_use(nodeType* enumIdentifier, nodeType* enumMemberIdentifier) {
    auto enumName = std::get<idNodeType>(enumIdentifier->un).id;
    auto enumMemberName = std::get<idNodeType>(enumMemberIdentifier->un).id;

    return new nodeType(enumUseNode{enumName, enumMemberName}, yylineno);
}

nodeType* identifierListNode(nodeType* idNode) {
    assert(std::holds_alternative<idNodeType>(idNode->un));
    return new nodeType(IdentifierListNode(std::get_if<idNodeType>(&idNode->un)), yylineno);
}

nodeType* statementList(nodeType* statement) {
    return new nodeType(StatementList(statement), yylineno);
}

nodeType* exprListNode(nodeType* exprCode) {
    return new nodeType(ExprListNode(exprCode), yylineno);
}

nodeType* functionCall(nodeType* fnIdentifier, nodeType* exprListTail) {
    assert(std::holds_alternative<ExprListNode>(exprListTail->un));
    assert(std::holds_alternative<idNodeType>(fnIdentifier->un));

    auto exprList = std::get<ExprListNode>(exprListTail->un).toVec();
    auto fnName = std::get<idNodeType>(fnIdentifier->un).id;

    return new nodeType(FunctionCall{fnName, exprList}, yylineno);
}