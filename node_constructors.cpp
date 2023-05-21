#include <cassert>
#include <stdarg.h>

#include <variant>
#include <vector>

#include "cl.h"
#include "node_constructors.h"
#include "y.tab.h"

#define CON_INIT(ptr_name, value) new nodeType(std::variant<NODE_TYPES>(conNodeType{value}))

nodeType *con(int iValue)   { return CON_INIT(p, iValue); }
nodeType *con(float fValue) { return CON_INIT(p, fValue); }
nodeType *con(bool bValue)  { return CON_INIT(p, bValue); }
nodeType *con(char cValue)  { return CON_INIT(p, cValue); }
nodeType *con(char* sValue) { return CON_INIT(p, std::string(sValue)); }

// Create an operator node with a variable number of inputs (usually 2)
// Note that `oper` is an enum value (or #ifdef), you can find the definintions in `y.tab.h`.
nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    int i;

    /* copy information */
    auto opr = oprNodeType{};

    opr.oper = oper;
    va_start(ap, nops);
    for (i = 0; i < nops; i++)
        opr.op.push_back(va_arg(ap, nodeType*));
    va_end(ap);

    return new nodeType(opr);
}

// Create an identifier node.
nodeType *id(const char* id) {
    /* copy information */
    auto idNode = idNodeType{std::string(id)};
    return new nodeType(idNode);
}

nodeType *sw(nodeType* var, nodeType* case_list_head) {
    auto switchNode = std::variant<NODE_TYPES>(switchNodeType{ 0, false, var, case_list_head });
    return new nodeType(switchNode);
}

nodeType *cs(nodeType* self, nodeType* prev) {
    auto cs = caseNodeType{};
    std::visit(
        Visitor {
                [&cs, self](oprNodeType&)  { cs.self = self; cs.prev = NULL; },
                [&cs, self, prev](caseNodeType&) { cs.prev = self; cs.self = prev; },
                [](auto) {}
        },
        self->un
    );

    return new nodeType(cs);
}

nodeType *br() {
    auto breakNode = std::variant<NODE_TYPES>(breakNodeType{NULL});
    return new nodeType(breakNode);
}

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body) {
    auto forNode = forNodeType{false, init_statement, loop_condition, post_loop_statement, loop_body};
    return new nodeType(forNode);
}

nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body) {
    auto whileNode = whileNodeType{false, loop_condition, loop_body};
    return new nodeType(whileNode);
}

nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body) {
    auto doWhileNode = std::variant<NODE_TYPES>(doWhileNodeType{false, loop_condition, loop_body});
    return new nodeType(doWhileNode);
}

nodeType* varDecl(nodeType* type, nodeType* name) {
    auto nameStr = std::get<idNodeType>(name->un).id;
    auto typeStr = std::get<idNodeType>(type->un).id;
    sym2[nameStr] = SymbolTableEntry(false, typeStr);

    return new nodeType(VarDecl(type, name));
}

nodeType* constVarDefn(nodeType* type, nodeType* name, nodeType* initExpr) {
    auto nameStr = std::get<idNodeType>(name->un).id;
    auto typeStr = std::get<idNodeType>(type->un).id;
    sym2[nameStr] = SymbolTableEntry(initExpr, true, typeStr);

    return new nodeType(VarDecl(type, name));
}


nodeType* fn(nodeType* name, std::vector<VarDecl*>& params, nodeType* return_type, nodeType* statements) {
    assert(std::holds_alternative<idNodeType>(name->un));
    assert(std::holds_alternative<idNodeType>(return_type->un));
    assert(std::holds_alternative<oprNodeType>(statements->un));

    auto stmts = std::get<oprNodeType>(statements->un);
    assert(stmts.oper == ';' || stmts.oper == RETURN);

    auto fn_name = std::get<idNodeType>(name->un).id;
    auto fn = functionNodeType{return_type, name, params, statements};
    functions[fn_name] = fn;

    return new nodeType(fn);
}

nodeType* fn_call(nodeType* name) {
    return new nodeType(functionNodeType{nullptr, name});
}

nodeType* fnParamList(nodeType* end) { 
    auto* varDecl = std::get_if<VarDecl>(&end->un);

    // assert only if `end` is not null
    // `end` should only be null for empty parameter lists.
    if(end != nullptr) {
        assert(varDecl != nullptr);
    } else {
        // Stump
        varDecl = new VarDecl{nullptr, nullptr};
    }

	return new nodeType(*varDecl); 
}

nodeType* fnParamList(nodeType* prev, nodeType* next) { 
	auto* prevVarDecl = std::get_if<VarDecl>(&prev->un);
	assert(prevVarDecl != nullptr);

	auto* currValDecl = std::get_if<VarDecl>(&next->un);
	assert(currValDecl != nullptr);

    currValDecl->prev = prevVarDecl;
	return new nodeType(*currValDecl); 
}

struct set_break_parent_visitor {
        nodeType* parent_switch;

        void operator()(breakNodeType& b) const {
               b.parent_switch = parent_switch;
        }

        void operator()(oprNodeType& opr) const {
                for(int i = 0; i < opr.op.size(); i++) {
                        set_break_parent(opr.op[i], parent_switch);
                }
        }

        void operator()(caseNodeType& c) const {
               set_break_parent(c.self, parent_switch);
               set_break_parent(c.prev, parent_switch);
        }

       // the default case:
       template<typename T> void operator()(T const & /*UNUSED*/) const { } 
};

void set_break_parent(nodeType* node, nodeType* parent_switch) {
        if(node == NULL) return;
        
        std::visit(set_break_parent_visitor{parent_switch}, node->un);
}

std::string VarDecl::getType() const { return std::get<idNodeType>(type->un).id; } 
std::string VarDecl::getName() const { return std::get<idNodeType>(var_name->un).id; } 
