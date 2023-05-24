#include <cstddef>
#include <iostream>
#include <fstream>
#include <variant>
#include <vector>
#include "cl.h"
#include "y.tab.h"

int ScopeSymbolTables::tableCount = 0;

bool startSymbolTable = true;

std::vector<ScopeSymbolTables*> allSymbolTables; // Add all the 

void printSymbolTables(){

    std::ofstream outfile;
    outfile.open("symtbl.log"); // open the file
    if (outfile.is_open()){
        /* Loop on all the SymbolTables */
        for (int i = 0; i < allSymbolTables.size(); i++) {
            outfile<<"Tables of Scope "+std::to_string(i)<<std::endl;
            outfile<<"Identifiers Symbol Table"<<std::endl;
            // /* Loop on all the 3 symbol tables: the sym, functionNode & enumNode tables*/
            outfile<<"sym_key, sym_val, declaration_line, type, isConstant"<<std::endl;
            /* Identifier Symbol Table */
            for (auto iter = allSymbolTables[i]->sym2.begin(); iter != allSymbolTables[i]->sym2.end(); ++iter)
            {
                outfile<<iter->first<<", "<<iter->second.getValue()<<", "<<iter->second.declaredAtLine;
                outfile<<", "<<iter->second.type<<", "<<iter->second.isConstant<<std::endl;
            }
            // outfile<<"Functions Symbol Table"<<std::endl;
            // outfile<<"sym_key, sym_val, declaration_line, type, isConstant"<<std::endl;
            // /* Identifier Symbol Table */
            // for (auto iter = allSymbolTables[i]->functions.begin(); iter != allSymbolTables[i]->functions.end(); ++iter)
            // {
            //     outfile<<iter->first<<", "<<iter->second.getValue()<<", "<<iter->second.declaredAtLine;
            //     outfile<<", "<<iter->second.type<<", "<<iter->second.isConstant<<std::endl;
            // }
            // outfile<<"Enums Symbol Table"<<std::endl;
            // /* Identifier Symbol Table */
            // for (auto iter = allSymbolTables[i]->sym2.begin(); iter != allSymbolTables[i]->sym2.end(); ++iter)
            // {
            //     outfile<<iter->first<<", "<<iter->second.getValue()<<", "<<iter->second.declaredAtLine;
            //     outfile<<", "<<iter->second.type<<", "<<iter->second.isConstant<<std::endl;
            // }
        }
        outfile<<"===="<<std::endl;
        outfile.close();
    }
}

void appendSymbolTable(int i){

}

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

        nt->currentScope = currentNodePtr->currentScope;
        setup_scopes(nt); 

        vd.initExpr->currentScope = currentNodePtr->currentScope;
        setup_scopes(vd.initExpr);        
    }

    void operator()(enumNode& en) {
        // Nothing needed here.
        en.name->currentScope = currentNodePtr->currentScope;
        setup_scopes(en.name);
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
            allSymbolTables.push_back(currentNodePtr->currentScope);
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
        //br.parent_switch->currentScope = currentNodePtr->currentScope;
        //setup_scopes(br.parent_switch); /*FIXME: This might cause issues. Unsure as I'm not even sure breaks are needed*/
    }

    void operator()(FunctionCall& fc) {
        /* Loop over all expressions & add scopes */
        for(auto& c: fc.parameterExpressions) {
            if (c == nullptr || c->exprCode == nullptr) { continue; }
            c->exprCode->currentScope = currentNodePtr->currentScope;
            setup_scopes(c->exprCode);
        }  
    }

    void operator()(StatementList& sl) {
        if(startSymbolTable)
        {
            currentNodePtr->currentScope = new ScopeSymbolTables();
            // This new global symbol table has parentTable = nullptr
            allSymbolTables.push_back(currentNodePtr->currentScope);
            startSymbolTable = false;
        }

        if(currentNodePtr->addNewScope)
        {
            auto temp = currentNodePtr->currentScope;
            currentNodePtr->currentScope = new ScopeSymbolTables();
            currentNodePtr->currentScope->parentScope = temp;
            allSymbolTables.push_back(currentNodePtr->currentScope);
        }

        for(auto& c: sl.toVec()) {
            c->statementCode->currentScope = currentNodePtr->currentScope;
            setup_scopes(c->statementCode);
        }
    }

    void operator()(functionNodeType& fn) {
        // TODO: Revise this when doing the function logic. 

        auto prevScope = currentNodePtr->currentScope;
        currentNodePtr->currentScope = fn.statements->currentScope = new ScopeSymbolTables();
        fn.statements->currentScope->parentScope = currentNodePtr->currentScope->parentScope = prevScope;
        // FIXME: If a function takes no parameters, the list will have one
        // element (the last one) with null data.
        for(auto* param: fn.parametersTail->toVec()) {
            if(param->var_name == nullptr && param->type == nullptr) break;

            param->var_name->currentScope = currentNodePtr->currentScope;
            param->type->currentScope = currentNodePtr->currentScope;
        }
        fn.name->currentScope = currentNodePtr->currentScope;
        fn.return_type->currentScope = currentNodePtr->currentScope;
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
            allSymbolTables.push_back(dw.loop_body->currentScope);
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
            allSymbolTables.push_back(w.loop_body->currentScope);
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
            allSymbolTables.push_back(newTable);
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
        for (auto *op : opr.op) {
            op->currentScope = currentNodePtr->currentScope;
            setup_scopes(op);
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