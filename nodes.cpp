#include "cl.h"
#include "nodes.h"

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
