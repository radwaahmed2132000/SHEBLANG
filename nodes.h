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
        auto nulls = std::remove_if(nodes.begin(), nodes.end(), [](T* node){ return node->prev == nullptr && node->self == nullptr; });

        nodes.erase(nulls, nodes.end());
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
	struct Node *type, *varName;
    bool isArray = false;

    struct Type getType() const;
    std::string getName() const;
    static Node* node(Node* type, Node* varName, bool isArray);
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

    operator std::string() const {
        using enum BinOper;

        std::string s;

        switch (op) {
        case Assign:       { s = "Assign";       } break; 
        case Add:          { s = "Add";          } break;
        case Sub:          { s = "Sub";          } break;
        case Mul:          { s = "Mul";          } break;
        case Div:          { s = "Div";          } break;
        case LShift:       { s = "LShift";       } break;
        case RShift:       { s = "RShift";       } break;
        case And:          { s = "And";          } break;
        case Or:           { s = "Or";           } break;
        case BitAnd:       { s = "BitAnd";       } break;
        case BitOr:        { s = "BitOr";        } break;
        case BitXor:       { s = "BitXor";       } break;
        case Mod:          { s = "Mod";          } break;
        case GreaterThan:  { s = "GreaterThan";  } break;
        case LessThan:     { s = "LessThan";     } break;
        case Equal:        { s = "Equal";        } break;
        case NotEqual:     { s = "NotEqual";     } break;
        case GreaterEqual: { s = "GreaterEqual"; } break;
        case LessEqual:    { s = "LessEqual";    } break;
        }

        return s;
    }
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

   static Node* node(Node *condition, Node *ifCode);
   static Node* node(Node *condition, Node *ifCode, Node *elseCode);
};

struct ArrayLiteral {
    std::vector<Node*> expressions;

    static Node* node(Node* exprTail);
};

struct Type {
    enum class IsArray {
        DontCare = 0,
        No = 0,
        Yes = 1,
    };

    std::string innerType;
    IsArray isArray;
    int depth;

    Type(std::string s) :
        innerType(s), isArray(IsArray::DontCare), depth(0) {}
        Type(std::string s, IsArray arr) : innerType(s), isArray(arr), depth(int(arr)) {}
        Type(std::string s, IsArray arr, int d) : innerType(s), isArray(arr), depth(d) {}
        Type() = default;

        inline const static std::string anyTypeStr = "<any>";

        static Type Invalid;
        static Type Any;

        static Node* variable(std::string typeStr, IsArray isArray);
        static Node *variable(Node * variableType, Type::IsArray isArray);

        // For cases like [[int]]. This function will be getting the inner [int]
        static Node *increaseDepth(Node * innerArrayType);

        bool operator==(const Type &other) const {
            bool anyWithMatchingDepth = (this->innerType == anyTypeStr ||
                                         other.innerType == anyTypeStr) &&
                                        (this->depth == other.depth);

            if (anyWithMatchingDepth) return true;

            bool depthAndTypeMatch = other.depth == this->depth &&
                                     other.innerType == this->innerType;
            if (this->isArray == IsArray::DontCare ||
                other.isArray == IsArray::DontCare) {
                return depthAndTypeMatch;
            }

            return other.isArray == this->isArray && depthAndTypeMatch;
    }

    bool operator==(const std::string& other) const { return other == this->innerType; }

    explicit operator std::string() const {
        std::stringstream ss;
        ss << innerType;

        if(isArray == IsArray::Yes) {
            for(int i = 1; i <= depth; i++) { ss << "[]"; }
        }

        return ss.str();
    }
    };

// Must be initialized here since the type is incomplete inside the class definition.
inline Type Type::Invalid = Type("<Invalid>", Type::IsArray::DontCare);
inline Type Type::Any = Type(Type::anyTypeStr, Type::IsArray::DontCare);

struct ArrayIndex {
    // arrayExpr is for when you for example access a 2D array, foo[0][0]
    Node *arrayExpr, *indexExpr;

    static Node* node(Node* arr, Node* index);
};
