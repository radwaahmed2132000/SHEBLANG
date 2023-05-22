#include <iostream>
#include <variant>
#include "cl.h"

struct setup_scopes_visitor {
    nodeType* currentNodePtr;

    void operator()(conNodeType& con) {
    }

    void operator()(idNodeType& identifier) {
    }

    void operator()(VarDecl& vd) {
    }

    void operator()(VarDefn& vd) {
    }

    void operator()(enumUseNode& eu) {
    }

    void operator()(switchNodeType& sw) {
    }

    void operator()(breakNodeType& br) {
    }

    void operator()(FunctionCall& fc) {
    }

    void operator()(StatementList& sl) {
        for(auto& c: sl.toVec()) {
            c->statementCode->parentScope = &currentNodePtr->currentScope;
        }
    }

    void operator()(functionNodeType& fn) {
    }

    void operator()(doWhileNodeType& dw) {
    }

    void operator()(whileNodeType& w) {
    }

    void operator()(forNodeType& f) {
    }

    void operator()(oprNodeType& opr) {
    }

    // the default case:
    template<typename T> 
    void operator()(T const & /*UNUSED*/ ) const {}

};

void setup_scopes(nodeType* p) {
    if(p == nullptr) { std::cerr << "null pointer encountered in setup_scopes\n"; }
    std::visit(setup_scopes_visitor{p}, p->un);
}