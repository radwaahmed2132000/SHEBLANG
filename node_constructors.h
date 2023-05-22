#pragma once
#include "cl.h"

nodeType *con(int iValue);
nodeType *con(float fValue);
nodeType *con(bool bValue);
nodeType *con(char cValue);
nodeType *con(char* sValue);

nodeType *opr(int oper, int nops, ...);
nodeType *id(const char* id);

nodeType *sw(nodeType* var, nodeType* case_list_head);
nodeType *cs(nodeType* labelExpr, nodeType* caseBody);
nodeType *br();

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body);
nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body);
nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body);

nodeType* varDecl(nodeType* type, nodeType* name);
nodeType* varDefn(nodeType* decl, nodeType* initExpr, bool isConstant);

nodeType* fn(nodeType* name, std::vector<VarDecl*>& params, nodeType* return_type, nodeType* statements);
nodeType* fn_call(nodeType* name);

nodeType* identifierListNode(nodeType* idNode);
nodeType* enum_defn(nodeType* enumIdentifier, std::vector<IdentifierListNode*>& members);
nodeType* enum_use(nodeType* enumIdentifier, nodeType* enumMemberIdentifier);

nodeType* statementList(nodeType* statement);

template<typename LinkedListNode>
nodeType* appendToLinkedList(nodeType* prev, nodeType* next) { 
	auto* prevNode = std::get_if<LinkedListNode>(&prev->un);
	assert(prevNode != nullptr);

	auto* currNode = std::get_if<LinkedListNode>(&next->un);
	assert(currNode != nullptr);

    currNode->prev = prevNode;
	return new nodeType(*currNode, 0); 
}

template<typename LinkedListNode> 
nodeType* linkedListStump(nodeType* end) {
    auto* stump = std::get_if<LinkedListNode>(&end->un);

    // assert only if `end` is not null
    // `end` should only be null for empty parameter lists.
    if(end != nullptr) {
        assert(stump != nullptr);
    } else {
        // Stump
        stump = new LinkedListNode();
    }

	return new nodeType(*stump,0); 
}