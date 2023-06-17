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
    assert(return_type->is<Type>());
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

Node* ArrayLiteral::node(Node* exprTail) {
    std::vector<Node*> expressions;
    for(auto* currNode = std::get_if<ExprListNode>(exprTail); currNode != nullptr; currNode = currNode->prev) {
        expressions.push_back(currNode->exprCode);
    }

    std::reverse(expressions.begin(), expressions.end());

    // Account for empty arrays with one null node.
    if(expressions.size() == 1 && expressions[0] == nullptr) {
        expressions.pop_back();
    }

    return new Node(ArrayLiteral{expressions}, currentLineNo);
}

Node* VarDecl::node(Node* type, Node* varName, bool isArray) {
    auto vd = VarDecl{
        .type = type,
        .varName = varName,
        .isArray = isArray
    };

    // Smelly circular references...
    auto* vdNode = new Node(vd, currentLineNo);
    vdNode->as<VarDecl>().self = vdNode;

    return vdNode;
}

Node* Type::variable(std::string typeStr, IsArray isArray) {
    int depth = 0;

    switch (isArray) {
    case IsArray::DontCare:
        depth = 0;
        break;

    case IsArray::Yes:
        depth = 1;
        break;
    }

    return new Node(Type(typeStr, isArray, depth), currentLineNo);
}


Node* Type::variable(Node* variableType, IsArray isArray) {
    auto typeStr = variableType->as<idNodeType>().id;
    
    return Type::variable(typeStr, isArray);
}

Node* Type::increaseDepth(Node* innerArrayType) {
    assert(innerArrayType->asPtr<Type>()->isArray);
    innerArrayType->asPtr<Type>()->depth += 1;
    return innerArrayType;
}

Node* ArrayIndex::node(Node* arr, Node* index) {
    return new Node(ArrayIndex{arr, index}, currentLineNo);
}
