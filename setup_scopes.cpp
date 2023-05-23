#include <iostream>
#include <variant>
#include "cl.h"
#include "y.tab.h"

int ScopeSymbolTables::tableCount = 0;

bool startSymbolTable = true;

struct setup_scopes_visitor {
    nodeType* currentNodePtr;

    void operator()(conNodeType& con) {
        // Nothing needed here.
    }

    void operator()(idNodeType& identifier) {
        // Nothing needed here.
    }

    void operator()(VarDecl& vd) { 
        vd.type->currentScope = currentNodePtr->currentScope;
        vd.var_name->currentScope = currentNodePtr->currentScope;
    }

    void operator()(VarDefn &vd) { 
        vd.initExpr->currentScope = currentNodePtr->currentScope;
    }

    void operator()(enumUseNode& eu) {
        // Nothing needed here.
    }

    void operator()(caseNodeType &cs) {
        if(currentNodePtr->addNewScope)
        {
            auto temp = currentNodePtr->currentScope;
            currentNodePtr->currentScope = new ScopeSymbolTables();
            currentNodePtr->currentScope->parentScope = temp;
        } 
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
     // TODO: Implement this when doing the function logic.   
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
        if(std::holds_alternative<StatementList>(dw.loop_body->un))
        {
            dw.condition->currentScope = currentNodePtr->currentScope;
            dw.loop_body->currentScope = currentNodePtr->currentScope;
        }
        else
        {
            dw.condition->currentScope = currentNodePtr->currentScope;
            dw.loop_body->currentScope = new ScopeSymbolTables();
            dw.loop_body->currentScope->parentScope = currentNodePtr->currentScope;
        }
        setup_scopes(dw.loop_body);
    }

    void operator()(whileNodeType& w) {
        if(std::holds_alternative<StatementList>(w.loop_body->un))
        {
            w.condition->currentScope = currentNodePtr->currentScope;
            w.loop_body->currentScope = currentNodePtr->currentScope;
        }
        else
        {
            w.condition->currentScope = currentNodePtr->currentScope;
            w.loop_body->currentScope = new ScopeSymbolTables();
            w.loop_body->currentScope->parentScope = currentNodePtr->currentScope;
        }
        setup_scopes(w.loop_body);
    }

    void operator()(forNodeType& f) {
        if(std::holds_alternative<StatementList>(f.loop_body->un))
        {
            f.init_statement->currentScope = currentNodePtr->currentScope;
            f.loop_condition->currentScope = currentNodePtr->currentScope;
            f.post_loop_statement->currentScope = currentNodePtr->currentScope;
            f.loop_body->currentScope = currentNodePtr->currentScope;
        }
        else
        {
            auto newTable = new ScopeSymbolTables();
            newTable->parentScope = currentNodePtr->currentScope;
            f.init_statement->currentScope = newTable;
            f.loop_condition->currentScope = newTable;
            f.post_loop_statement->currentScope = newTable;
            f.loop_body->currentScope = newTable;
        }
        setup_scopes(f.loop_body);
    }

    void operator()(oprNodeType& opr) {
        switch(opr.oper) {
            case IF: {
                if(opr.op.size() > 2)
                {
                    opr.op[0]->currentScope = currentNodePtr->currentScope;
                    opr.op[1]->currentScope = currentNodePtr->currentScope;
                    opr.op[2]->currentScope = currentNodePtr->currentScope;
                    setup_scopes(opr.op[1]);
                    setup_scopes(opr.op[2]);
                }
                else if(opr.op.size() > 1)
                {
                    opr.op[0]->currentScope = currentNodePtr->currentScope;
                    opr.op[1]->currentScope = currentNodePtr->currentScope;
                    setup_scopes(opr.op[1]);
                }
            }
            default: return;
        }
    }

    // the default case:
    template<typename T> 
    void operator()(T const & /*UNUSED*/ ) const {}

};

void setup_scopes(nodeType* p) {
    if(p == nullptr) { std::cerr << "null pointer encountered in setup_scopes\n"; }
    std::visit(setup_scopes_visitor{p}, p->un);
}