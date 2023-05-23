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
        identifier.scopeNodePtr = currentNodePtr;
    }

    void operator()(VarDecl& vd) { 
        vd.type->currentScope = currentNodePtr->currentScope;
        vd.var_name->currentScope = currentNodePtr->currentScope;

        setup_scopes(vd.type);
        setup_scopes(vd.var_name);
    }

    void operator()(VarDefn &vd) {
        nodeType *nt = new nodeType(VarDecl(vd.decl->type, vd.decl->var_name), vd.decl->type->lineNo);
        setup_scopes(nt); 
        vd.initExpr->currentScope = currentNodePtr->currentScope;
        setup_scopes(vd.initExpr);        
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
            if (c->labelExpr != nullptr)
            {
                c->labelExpr->currentScope = currentNodePtr->currentScope;
                setup_scopes(c->labelExpr);
            }
            setup_scopes(c->caseBody);
        }
    }

    void operator()(switchNodeType& sw) {
        sw.var->currentScope = currentNodePtr->currentScope;
        setup_scopes(sw.var);
        sw.case_list_head->currentScope = currentNodePtr->currentScope;
        setup_scopes(sw.case_list_head);
    }

    void operator()(breakNodeType& br) {
        br.parent_switch->currentScope = currentNodePtr->currentScope;
        setup_scopes(br.parent_switch); /*FIXME: This might cause issues. Unsure as I'm not even sure breaks are needed*/
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
        // TODO: Revise this when doing the function logic. 
        fn.return_type->currentScope = currentNodePtr->currentScope;
        setup_scopes(fn.return_type);
        fn.name->currentScope = currentNodePtr->currentScope;
        setup_scopes(fn.name);
        fn.statements->currentScope = currentNodePtr->currentScope;
        setup_scopes(fn.statements);
        for (auto& p : fn.parameters) {
            nodeType* nt = new nodeType(VarDecl(p->type, p->var_name), p->type->lineNo);
            setup_scopes(nt);
        }
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
        setup_scopes(dw.condition);
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
        setup_scopes(w.condition);
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
        setup_scopes(f.init_statement);
        setup_scopes(f.loop_condition);
        setup_scopes(f.post_loop_statement);
        setup_scopes(f.loop_body);
    }

    void operator()(oprNodeType& opr) {
        switch(opr.oper) {
            // case IF: {
            //     if(opr.op.size() > 2)
            //     {
            //         opr.op[0]->currentScope = currentNodePtr->currentScope;
            //         opr.op[1]->currentScope = currentNodePtr->currentScope;
            //         opr.op[2]->currentScope = currentNodePtr->currentScope;
            //         setup_scopes(opr.op[0]);
            //         setup_scopes(opr.op[1]);
            //         setup_scopes(opr.op[2]);
            //     }
            //     else if(opr.op.size() > 1)
            //     {
            //         opr.op[0]->currentScope = currentNodePtr->currentScope;
            //         opr.op[1]->currentScope = currentNodePtr->currentScope;
            //         setup_scopes(opr.op[0]);
            //         setup_scopes(opr.op[1]);
            //     }
            // }
            case RETURN:
                opr.op[0]->currentScope = currentNodePtr->currentScope;
            default: {
                for(auto& c: opr.op) {
                    c->currentScope = currentNodePtr->currentScope;
                    setup_scopes(c);
                }
            } 
            return;
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