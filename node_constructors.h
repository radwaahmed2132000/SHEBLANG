#pragma once
#include "cl.h"

extern int yylineno;

nodeType *con(int iValue);
nodeType *con(float fValue);
nodeType *con(bool bValue);
nodeType *con(char cValue);
nodeType *con(char* sValue);

nodeType *opr(int oper, int nops, ...);
nodeType *id(const char* id);

nodeType *sw(nodeType* var, nodeType* caseListTail);
nodeType *cs(nodeType* labelExpr, nodeType* caseBody);
nodeType *br();

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body);
nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body);
nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body);

nodeType* varDecl(nodeType* type, nodeType* name);
nodeType* varDefn(nodeType* decl, nodeType* initExpr, bool isConstant);

nodeType* fn(nodeType* name, VarDecl* params, nodeType* return_type, nodeType* statements);

nodeType* identifierListNode(nodeType* idNode, bool addNewScope);
nodeType* enum_defn(nodeType* enumIdentifier, std::vector<IdentifierListNode*>& members);
nodeType* enum_use(nodeType* enumIdentifier, nodeType* enumMemberIdentifier);

nodeType* statementList(nodeType* statement);

nodeType* exprListNode(nodeType* exprCode);
nodeType* functionCall(nodeType* fnIdentifier, nodeType* exprListHeader);

template<typename LinkedListNode>
nodeType* appendToLinkedList(nodeType* prev, nodeType* next) { 
	auto* prevNode = prev->asPtr<LinkedListNode>();
	assert(prevNode != nullptr);

	auto* currNode = next->asPtr<LinkedListNode>();
	assert(currNode != nullptr);

    currNode->prev = prevNode;
	return new nodeType(*currNode, currentLineNo); 
}

template<typename LinkedListNode> 
nodeType* linkedListStump(nodeType* end) {
    auto* stump = end->asPtr<LinkedListNode>();

    // assert only if `end` is not null
    // `end` should only be null for empty parameter lists.
    if(end != nullptr) {
        assert(stump != nullptr);
    } else {
        // In case of Stump:
        stump = new LinkedListNode();
    }

	return new nodeType(*stump, currentLineNo); 
}
