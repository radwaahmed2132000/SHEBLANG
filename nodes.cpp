#include "cl.h"
#include "nodes.h"
#include <vector>

Node* BinOp::node(BinOper op, Node* lOp, Node* rOp) {
    return new Node(BinOp{op, lOp, rOp}, currentLineNo);
}

Node* BinOp::assign(Node* lOp, Node* rOp) {
    return new Node(BinOp{BinOper::Assign, lOp, rOp}, currentLineNo);
}

Node* BinOp::opAssign(BinOper op, Node* lOp, Node* rOp) {
    auto* innerRightOp = BinOp::node(op, lOp, rOp);
    return new Node(BinOp{BinOper::Assign, lOp, innerRightOp}, currentLineNo);
}

Node* UnOp::node(UnOper op, Node* operand) {
    return new Node(UnOp{op, operand}, currentLineNo);
}

Node* IfNode::node(Node* condition, Node* ifCode) {
    return new Node(IfNode{condition, ifCode, nullptr}, currentLineNo);
}

Node* IfNode::node(Node* condition, Node* ifCode, Node* elseCode) {
    return new Node(IfNode{condition, ifCode, elseCode}, currentLineNo);
}

Node* FunctionDefn::node(Node* name, Node* paramsTail, Node* return_type, Node* statements) {
    assert(name->is<idNodeType>());
    assert(return_type->is<idNodeType>());
    assert(statements->is<StatementList>());

    return new Node(FunctionDefn{return_type, name, paramsTail, statements}, currentLineNo);
}

std::vector<VarDecl*> FunctionDefn::getParameters() const {
    auto* paramsTail = parametersTail->asPtr<VarDecl>();
    return paramsTail->toVec();
}

std::vector<Node*> FunctionDefn::getParametersAsNodes() const {
    auto decls = getParameters();
    std::vector<Node*> ptrs;
    ptrs.reserve(decls.size());
    for(auto* decl: decls) {
        ptrs.push_back(decl->self);
    }
    return ptrs;
}