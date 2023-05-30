#include "cl.h"
#include "nodes.h"
#include <vector>

nodeType* BinOp::node(BinOper op, nodeType* lOp, nodeType* rOp) {
    return new nodeType(BinOp{op, lOp, rOp}, currentLineNo);
}

nodeType* BinOp::assign(nodeType* lOp, nodeType* rOp) {
    return new nodeType(BinOp{BinOper::Assign, lOp, rOp}, currentLineNo);
}

nodeType* BinOp::opAssign(BinOper op, nodeType* lOp, nodeType* rOp) {
    auto* innerRightOp = BinOp::node(op, lOp, rOp);
    return new nodeType(BinOp{BinOper::Assign, lOp, innerRightOp}, currentLineNo);
}

nodeType* UnOp::node(UnOper op, nodeType* operand) {
    return new nodeType(UnOp{op, operand}, currentLineNo);
}

nodeType* IfNode::node(nodeType* condition, nodeType* ifCode) {
    return new nodeType(IfNode{condition, ifCode, nullptr}, currentLineNo);
}

nodeType* IfNode::node(nodeType* condition, nodeType* ifCode, nodeType* elseCode) {
    return new nodeType(IfNode{condition, ifCode, elseCode}, currentLineNo);
}

nodeType* FunctionDefn::node(nodeType* name, nodeType* paramsTail, nodeType* return_type, nodeType* statements) {
    assert(name->is<idNodeType>());
    assert(return_type->is<idNodeType>());
    assert(statements->is<StatementList>());

    return new nodeType(FunctionDefn{return_type, name, paramsTail, statements}, currentLineNo);
}

std::vector<VarDecl*> FunctionDefn::getParameters() const {
    auto* paramsTail = parametersTail->asPtr<VarDecl>();
    return paramsTail->toVec();
}

std::vector<nodeType*> FunctionDefn::getParametersAsNodes() const {
    auto decls = getParameters();
    std::vector<nodeType*> ptrs;
    ptrs.reserve(decls.size());
    for(auto* decl: decls) {
        ptrs.push_back(decl->self);
    }
    return ptrs;
}