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
        std::cerr << "decl";
    }

    void operator()(VarDefn &vd) { 
        std::cerr << "defn"; 
    }

    void operator()(enumUseNode& eu) { }

    void operator()(caseNodeType &cs) { 
        auto cases = cs.toVec(); 
        for(auto* c: cases) {
            c->caseBody->parentScope = &currentNodePtr->currentScope;
            c->labelExpr->parentScope = &currentNodePtr->currentScope;
            setup_scopes(c->caseBody);
        }
    }

    void operator()(switchNodeType& sw) {
        sw.case_list_head->parentScope = &currentNodePtr->currentScope;

        setup_scopes(sw.case_list_head);
    }

    void operator()(breakNodeType& br) {
    }

    void operator()(FunctionCall& fc) {
        
    }

    void operator()(StatementList& sl) {
        for(auto& c: sl.toVec()) {
            c->statementCode->parentScope = &currentNodePtr->currentScope;
            setup_scopes(c->statementCode);
        }
    }

    void operator()(functionNodeType& fn) {
        fn.statements->parentScope = &currentNodePtr->currentScope;
        setup_scopes(fn.statements);
    }

    void operator()(doWhileNodeType& dw) {
    }

    void operator()(whileNodeType& w) {
    }

    void operator()(forNodeType& f) {
    }

    void operator()(oprNodeType& opr) {
        std::cerr << "opr";
    }

    // the default case:
    template<typename T> 
    void operator()(T const & /*UNUSED*/ ) const {}

};

void setup_scopes(nodeType* p) {
    if(p == nullptr) { std::cerr << "null pointer encountered in setup_scopes\n"; }
    std::visit(setup_scopes_visitor{p}, p->un);
}