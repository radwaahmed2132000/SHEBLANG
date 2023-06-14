#include <variant>
#include "nodes.h"

#define NODE_TYPES                                                             \
    conNodeType, idNodeType, switchNodeType, caseNodeType, breakNodeType,      \
        FunctionDefn, whileNodeType, forNodeType, doWhileNodeType, VarDecl,    \
        VarDefn, enumNode, IdentifierListNode, enumUseNode, StatementList,     \
        FunctionCall, ExprListNode, BinOp, UnOp, IfNode, ArrayLiteral, Type,    \
        ArrayIndex

struct ScopeSymbolTables; struct Node : std::variant<NODE_TYPES> {
    int lineNo;
    std::string conversionType; 
    bool addNewScope;
    ScopeSymbolTables* currentScope;

    Node() = default;
    Node(std::variant<NODE_TYPES> inner_union, int lineNo);
	Node(std::variant<NODE_TYPES> inner_union, int lineNo, bool addNewScope);

    template<typename T> 
    bool is() const { return std::holds_alternative<T>(*this); }

    template<typename T>
    T* asPtr() { return std::get_if<T>(this); }

    template<typename T>
    T& as() { return std::get<T>(*this); }
};

struct SymbolTableEntry {
    Value value;
    bool isConstant;
    bool isUsed = false;
    int declaredAtLine = -1;
    Type type = Type::Invalid;  
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
};

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
            ss << symbol << '\t' << std::string(entry.type) <<'\t' <<  entry.value << '\t' << entry.declaredAtLine <<  '\t' << entry.isConstant << '\n';
        }

        return ss.str();
    }
};
