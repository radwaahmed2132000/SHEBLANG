#pragma once
#include "cl.h"

extern int yylineno;

Node *con(int iValue);
Node *con(float fValue);
Node *con(bool bValue);
Node *con(char cValue);
Node *con(char* sValue);

Node *id(const char* id);

Node *sw(Node* var, Node* caseListTail);
Node *cs(Node* labelExpr, Node* caseBody);
Node *br();

Node *for_loop(Node* init_statement, Node* loop_condition, Node* post_loop_statement, Node* loop_body);
Node *while_loop(Node* loop_condition, Node* loop_body);
Node *do_while_loop(Node* loop_condition, Node* loop_body);

Node* varDecl(Node* type, Node* name);
Node* varDefn(Node* decl, Node* initExpr, bool isConstant);

Node* fn(Node* name, VarDecl* params, Node* return_type, Node* statements);

Node* identifierListNode(Node* idNode, bool addNewScope);
Node* enum_defn(Node* enumIdentifier, std::vector<IdentifierListNode*>& members);
Node* enum_use(Node* enumIdentifier, Node* enumMemberIdentifier);

Node* statementList(Node* statement);
Node* statementList();

Node* exprListNode(Node* exprCode);
Node* functionCall(Node* fnIdentifier, Node* exprListHeader);

template<typename LinkedListNode>
Node* appendToLinkedList(Node* prev, Node* next) { 
	auto* prevNode = prev->asPtr<LinkedListNode>();
	assert(prevNode != nullptr);

	auto* currNode = next->asPtr<LinkedListNode>();
	assert(currNode != nullptr);

    currNode->prev = prevNode;
    currNode->self = next;
	return new Node(*currNode, currentLineNo); 
}

template<typename LinkedListNode> 
Node* linkedListStump(Node* end) {
    auto* stump = end->asPtr<LinkedListNode>();

    // assert only if `end` is not null
    // `end` should only be null for empty parameter lists.
    if(end != nullptr) {
        assert(stump != nullptr);
    } else {
        // In case of Stump:
        stump = new LinkedListNode();
    }

    stump->self = end;
	return new Node(*stump, currentLineNo); 
}
