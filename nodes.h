#pragma once

#include <cstdio>
#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <assert.h>
#include <new>
#include <system_error>

#include "value.h"
#include "result.h"

template <typename T>
struct LinkedListNode {
    T * prev = nullptr;
    struct Node* self = nullptr;

    LinkedListNode() = default;
    LinkedListNode(T* p) : prev(p) {}
    
	std::vector<T*> toVec() {
		std::vector<T*> nodes;

		T* current = static_cast<T*>(this);
		while(current != nullptr) {
			nodes.push_back(current);
			current = current->prev;
		}
	
        std::reverse(nodes.begin(), nodes.end());
		return nodes;
	}

    std::vector<struct Node*> toNodes() const {
		std::vector<Node*> nodes;

		T* current = static_cast<T*>(this);
		while(current != nullptr) {
			nodes.push_back(current->self);
			current = current->prev;
		}
	
        std::reverse(nodes.begin(), nodes.end());
		return nodes;
    }
};

/* For constants and literals. */
using conNodeType = Value;

/* identifiers */
typedef struct {
    std::string id;                 /* key/index to sym array */
} idNodeType;

typedef struct caseNodeType: LinkedListNode<caseNodeType> {
    struct  Node* labelExpr, * caseBody;

    caseNodeType(): caseBody(nullptr), LinkedListNode(nullptr) {}
    caseNodeType(Node* labelExpr, Node* caseBody): labelExpr(labelExpr), caseBody(caseBody) { }

    bool isDefault() const { return (labelExpr == nullptr); }
} caseNodeType;

typedef struct {
    int exitLabel;             // Needed for code generation, stores the label that cases jump to on completion 
    struct Node* var;       // The variable we're switching on
    struct Node* caseListTail; // The last case, it contains a pointer the the previous ones.
} switchNodeType;

typedef struct {
    struct Node* parent_switch;
} breakNodeType;

typedef struct {
    bool break_encountered;
    struct Node *condition, *loop_body;
    int exit_label;
} whileNodeType;

// Exactly the same as whileNodeType,
// the main difference is in the behaviour itself.
// But, we can't have two elements of the same type in std::variant, so I had to
// split them.
typedef struct {
    bool break_encountered;
    struct Node *condition, *loop_body;
    int exit_label;
} doWhileNodeType;

typedef struct {
    bool break_encountered;
    struct Node *init_statement, *loop_condition, *post_loop_statement, *loop_body;
    int exit_label;
} forNodeType;

typedef struct IdentifierListNode: LinkedListNode<IdentifierListNode> {
    idNodeType* identifierName;
    IdentifierListNode(): identifierName(nullptr), LinkedListNode(nullptr) {}
    IdentifierListNode(idNodeType* idNode): identifierName(idNode), LinkedListNode(nullptr) {}

} IdentifierListNode;

typedef struct enumNode {
    Node* name;
    std::vector<std::string> enumMembers;
} enumNode;

typedef struct enumUseNode {
    std::string enumName, memberName;
    int lineNo;
} enumUseNode;

typedef struct StatementList {
    std::vector<struct Node*> statements;

    StatementList(): statements({}) {}
    StatementList(Node* statementCode): statements({statementCode}) {}
    void addStatement(Node* statement) { statements.push_back(statement); }

} StatementList;

typedef struct ExprListNode: LinkedListNode<ExprListNode> {
    struct Node* exprCode;

    ExprListNode(): exprCode(nullptr) {}
    ExprListNode(Node* exprCode): exprCode(exprCode) {}

} ExprListNode;

typedef struct FunctionCall {
    std::string functionName;
    std::vector<ExprListNode*> parameterExpressions;    
    int lineNo;
} FunctionCall;

typedef struct VarDecl : LinkedListNode<VarDecl>{
	struct Node *type, *var_name;
    bool isConstant = false;
    struct Node* initExpr = nullptr;

    VarDecl(): type(nullptr), var_name(nullptr), LinkedListNode(nullptr) {}
	VarDecl(Node* type, Node* var_name) : type(type), var_name(var_name) {}
    VarDecl(Node* type, Node* var_name, Node* initExpr, bool isConstant) : type(type), var_name(var_name), initExpr(initExpr), isConstant(isConstant) {}

    std::string getType() const;
    std::string getName() const;
} VarDecl;

typedef struct VarDefn {
	VarDecl* decl;
    bool isConstant;
    struct Node* initExpr = nullptr;

	VarDefn(VarDecl* decl, Node* initExpr, bool isConstant) : decl(decl), initExpr(initExpr), isConstant(isConstant) {}
} VarDefn;

struct FunctionDefn {
    struct Node* return_type;
    struct Node* name;
    struct Node* parametersTail;
    struct Node* statements;

    static Node* node(Node* name, Node* paramsTail, Node* return_type, Node* statements);
    std::vector<VarDecl*> getParameters() const;
    std::vector<Node*> getParametersAsNodes() const;
};

struct ReturnNode {
    Node* retExpr;
};

enum class BinOper {
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    LShift,
    RShift,
    And,
    Or,
    BitAnd,
    BitOr,
    BitXor,
    Mod,
    GreaterThan,
    LessThan,
    Equal,
    NotEqual,
    GreaterEqual,
    LessEqual
};

struct BinOp {
    BinOper op;
    Node* lOperand, *rOperand;

    static Node* node(BinOper op, Node* lOp, Node* rOp);
    static Node* assign(Node* lOp, Node* rOp);
    static Node* opAssign(BinOper op, Node* lOp, Node* rOp);
};

enum class UnOper { 
    BoolNeg,
    BitToggle,
    Plus,
    Minus,
    Increment,
    Decrement,
    Print,  // TODO: Move to its own node type
    Return  // TODO: Move to its own node type
};

struct UnOp {
    UnOper op;
    Node* operand;

    static Node* node(UnOper op, Node* operand);
};

struct IfNode {
   Node* condition, *ifCode, *elseCode;

   static Node *node(Node *condition, Node *ifCode);
   static Node * node(Node *condition, Node *ifCode, Node *elseCode);
};

typedef struct SymbolTableEntry {
    Value value;
    bool isConstant;
    bool isUsed = false;
    int declaredAtLine = -1;
    std::string type = "<no type>";  
    Node* initExpr = nullptr;

    SymbolTableEntry() = default;

    SymbolTableEntry(Node* initExpr, bool isConstant, std::string type):
        initExpr(initExpr), isConstant(isConstant), type(type) {}
    
    SymbolTableEntry(bool isConstant, std::string type):
        value(0), isConstant(isConstant), type(type) {}

    SymbolTableEntry& setValue(Value v) {
        value = v;
        return *this;
    }

    Value getValue() const { return value; }

    Value& getRef() { return value; }
} SymbolTableEntry;

struct ScopeSymbolTables {
    static int tableCount;
    int tableId;

    ScopeSymbolTables() {
        tableId = ++tableCount;
        parentScope = nullptr;
    }

    std::unordered_map<std::string, SymbolTableEntry> sym2;
    std::unordered_map<std::string, FunctionDefn> functions;
    std::unordered_map<std::string, enumNode> enums;
    ScopeSymbolTables* parentScope;


    ScopeSymbolTables(const ScopeSymbolTables& other) : sym2(other.sym2), functions(other.functions), enums(other.enums), tableId(++tableCount) { }

    ScopeSymbolTables& operator=(const ScopeSymbolTables& other) {
        if(this != &other)  {
            this->sym2 = other.sym2;
            this->functions = other.functions;
            this->enums = other.enums;
            tableId = ++tableCount;
        }
        return *this;
    }

    std::string symbolsToString() const {
        std::stringstream ss;
        for(const auto& [symbol, entry]: sym2) {
            ss << symbol << '\t' << entry.type <<'\t' <<  entry.value << '\t' << entry.declaredAtLine <<  '\t' << entry.isConstant << '\n';
        }

        return ss.str();
    }
};

