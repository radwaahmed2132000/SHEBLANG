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
