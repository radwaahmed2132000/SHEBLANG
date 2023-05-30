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
#include "nodes.h"


#define CON_INIT(ptr_name, value) new Node(std::variant<NODE_TYPES>(conNodeType{value}), currentLineNo)

Node *con(int iValue)   { return CON_INIT(p, iValue); }
Node *con(float fValue) { return CON_INIT(p, fValue); }
Node *con(bool bValue)  { return CON_INIT(p, bValue); }
Node *con(char cValue)  { return CON_INIT(p, cValue); }
Node *con(char* sValue) { return CON_INIT(p, std::string(sValue)); }

// Create an identifier node.
Node *id(const char* id) {
    /* copy information */
    auto idNode = idNodeType{std::string(id)};
    return new Node(idNode, currentLineNo);
}

Node *sw(Node* var, Node* caseListTail) {
    assert(caseListTail->is<caseNodeType>());

    auto switchNode = std::variant<NODE_TYPES>(switchNodeType{ 0, var, caseListTail });

    return new Node(switchNode, currentLineNo);
}

Node *cs(Node* labelExpr, Node* caseBody) {
    return new Node(caseNodeType(labelExpr, caseBody), currentLineNo);
}

Node *br() {
    auto breakNode = std::variant<NODE_TYPES>(breakNodeType{NULL});
    return new Node(breakNode, currentLineNo);
}

Node *for_loop(Node* init_statement, Node* loop_condition, Node* post_loop_statement, Node* loop_body) {
    auto forNode = forNodeType{false, init_statement, loop_condition, post_loop_statement, loop_body};
    return new Node(forNode, currentLineNo);
}

Node *while_loop(Node* loop_condition, Node* loop_body) {
    auto whileNode = whileNodeType{false, loop_condition, loop_body};
    return new Node(whileNode, currentLineNo);
}

Node *do_while_loop(Node* loop_condition, Node* loop_body) {
    auto doWhileNode = std::variant<NODE_TYPES>(doWhileNodeType{false, loop_condition, loop_body});
    return new Node(doWhileNode, currentLineNo);
}

Node* varDecl(Node* type, Node* name) {
    auto nameStr = name->as<idNodeType>().id;
    auto typeStr = type->as<idNodeType>().id;

    return new Node(VarDecl(type, name), currentLineNo);
}

Node* varDefn(Node* decl, Node* initExpr, bool isConstant) {
    auto* declPtr = decl->asPtr<VarDecl>();
    return new Node(VarDefn(declPtr, initExpr, isConstant), currentLineNo);
}

std::string VarDecl::getType() const { return type->as<idNodeType>().id; } 
std::string VarDecl::getName() const { return var_name->as<idNodeType>().id; } 

Node* fn_call(Node* name) {
    return new Node(FunctionDefn{nullptr, name}, currentLineNo);
}

/* Note: we don't recurse into inner loops, if an inner loop contains a break
 * statement, then that belongs to it. Example:
 *  while(...) {
 *      ...
 *      for(...) {
 *          if(...) break; // Belongs to the inner for, so we don't recurse.
 *      }
 *  }
 */
struct set_break_parent_visitor {
    // Can be a loop (for, while, do while)
    Node *parentNode;

    void operator()(breakNodeType &b) const { b.parent_switch = parentNode; }

    void operator()(StatementList& sl) const {
        for(auto* statement: sl.statements) {
            set_break_parent(statement, parentNode);
        }
    }

    void operator()(IfNode &ifNode) const {
        set_break_parent(ifNode.ifCode, parentNode);
        set_break_parent(ifNode.elseCode, parentNode);
    }

    // the default case:
    template <typename T> void operator()(T const & /*UNUSED*/) const {}
};

void set_break_parent(Node* node, Node* parent_switch) {
    if(node == NULL) return;
    std::visit(set_break_parent_visitor{parent_switch}, *node);
}

Node* enum_defn(Node* enumIdentifier, std::vector<IdentifierListNode*>& members) {
    assert(enumIdentifier->is<idNodeType>());

    std::vector<std::string> memberNames;
    memberNames.reserve(members.size());
    for(const auto& node: members) { memberNames.emplace_back(node->identifierName->id); }

    // Needed now so the interpreter works correctly..
    auto e = enumNode{enumIdentifier, memberNames};
    auto enumName =  enumIdentifier->as<idNodeType>().id;

    return new Node(e, currentLineNo);
}

Node* enum_use(Node* enumIdentifier, Node* enumMemberIdentifier) {
    auto enumName =  enumIdentifier->as<idNodeType>().id;
    auto enumMemberName =  enumMemberIdentifier->as<idNodeType>().id;

    return new Node(enumUseNode{enumName, enumMemberName, currentLineNo}, currentLineNo);
}

Node* identifierListNode(Node* idNode, bool addNewScope) {
    assert(idNode->is<idNodeType>());
    return new Node(IdentifierListNode(idNode->asPtr<idNodeType>()), currentLineNo ,addNewScope);
}

Node* statementList(Node* statement) {
    return new Node(StatementList(statement), currentLineNo);
}

Node* statementList() {
    return new Node(StatementList(), currentLineNo);
}


Node* exprListNode(Node* exprCode) {
    return new Node(ExprListNode(exprCode), currentLineNo);
}

Node* functionCall(Node* fnIdentifier, Node* exprListTail) {
    assert(exprListTail->is<ExprListNode>());
    assert(fnIdentifier->is<idNodeType>());

    auto exprList = exprListTail->as<ExprListNode>().toVec();
    auto fnName = fnIdentifier->as<idNodeType>().id;

    return new Node(FunctionCall{fnName, exprList, currentLineNo}, currentLineNo);
}
