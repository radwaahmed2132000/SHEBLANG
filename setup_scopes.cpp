#include <iostream>
#include <variant>
#include "cl.h"

int ScopeSymbolTables::tableCount = 0;

bool startSymbolTable = true;

struct setup_scopes_visitor {
    nodeType* currentNodePtr;

    void operator()(conNodeType& con) {
    }

    void operator()(idNodeType& identifier) {
    }

    void operator()(VarDecl& vd) { 
        // std::cerr << "decl";
    }

    void operator()(VarDefn &vd) { 
        // std::cerr << "defn"; 
    }

    void operator()(enumUseNode& eu) { }

    void operator()(caseNodeType &cs) { 
        auto cases = cs.toVec(); 
        for(auto* c: cases) {
            c->caseBody->currentScope = currentNodePtr->currentScope;
            c->labelExpr->currentScope = currentNodePtr->currentScope;
            setup_scopes(c->caseBody);
        }
    }

    void operator()(switchNodeType& sw) {
        sw.case_list_head->currentScope = currentNodePtr->currentScope;
        setup_scopes(sw.case_list_head);
    }

    void operator()(breakNodeType& br) {
        br.parent_switch->currentScope = currentNodePtr->currentScope;
    }

    void operator()(FunctionCall& fc) {
        
    }

    void operator()(StatementList& sl) {
        if(startSymbolTable)
        {
            currentNodePtr->currentScope = new ScopeSymbolTables();
            // This new global symbol table has parentTable = nullptr
            startSymbolTable = false;
        }
        if(currentNodePtr->addNewScope)
        {
            auto temp = currentNodePtr->currentScope;
            currentNodePtr->currentScope = new ScopeSymbolTables();
            currentNodePtr->currentScope->parentScope = temp;
        }
        for(auto& c: sl.toVec()) {
            c->statementCode->currentScope = currentNodePtr->currentScope;
            setup_scopes(c->statementCode);
        }
    }

    void operator()(functionNodeType& fn) {
        fn.statements->currentScope = currentNodePtr->currentScope;
        setup_scopes(fn.statements);
    }

    void operator()(doWhileNodeType& dw) {
    }

    void operator()(whileNodeType& w) {
    }

    void operator()(forNodeType& f) {
    }

    void operator()(oprNodeType& opr) {
        // std::cerr << "opr";
    }

    // the default case:
    template<typename T> 
    void operator()(T const & /*UNUSED*/ ) const {}

};

void setup_scopes(nodeType* p) {
    if(p == nullptr) { std::cerr << "null pointer encountered in setup_scopes\n"; }
    std::visit(setup_scopes_visitor{p}, p->un);
}