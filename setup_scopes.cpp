#include <cstddef>
#include <iostream>
#include <fstream>
#include <variant>
#include <vector>
#include "cl.h"
#include "nodes.h"
#include "parser.h"

int ScopeSymbolTables::tableCount = 0;

bool startSymbolTable = true;

std::vector<ScopeSymbolTables*> allSymbolTables; // Add all the 

void printSymbolTables(){

    std::ofstream outfile;
    outfile.open("symtbl.log"); // open the file
    if (outfile.is_open()){
        /* Loop on all the SymbolTables */
        for (int i = 0; i < allSymbolTables.size(); i++) {
            outfile<<"Tables of Scope "+std::to_string(allSymbolTables[i]->tableId)<<std::endl;
            outfile<<"Identifiers Symbol Table"<<std::endl;
            outfile << "symKey\tsymVal\tdeclarationLine\ttype\tisConstant"<<std::endl;

            /* Identifier Symbol Table */
            outfile << allSymbolTables[i]->symbolsToString();

            // outfile<<"Functions Symbol Table"<<std::endl;
            // outfile<<"function_name, return_type, parameter_list"<<std::endl;
            // /* Function Symbol Table */
            // for (auto iter = allSymbolTables[i]->functions.begin(); iter != allSymbolTables[i]->functions.end(); ++iter)
            // {
                // auto function_name = iter->first;
                // outfile<<function_name<<", ";
                // auto return_type = std::get<idNodeType>(iter->second.return_type->un).id;
                // outfile<<return_type<<", "<<std::endl;
                // auto parameter_list = iter->second.parametersTail
                // std::get<caseNodeType>(head->un).toVec();
                // outfile<<iter->first<<", "<<iter->second.getValue()<<", "<<iter->second.declaredAtLine;
                // outfile<<", "<<iter->second.type<<", "<<iter->second.isConstant<<std::endl;
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


struct setup_scopes_visitor {
    Node* currentNodePtr;

    void operator()(conNodeType& con) {
        // Nothing needed here.
    }

    void operator()(VarDecl& vd) const { 
        vd.type->currentScope = currentNodePtr->currentScope;
        vd.varName->currentScope = currentNodePtr->currentScope;

        setup_scopes(vd.type);
        setup_scopes(vd.varName);
    }

    void operator()(VarDefn &vd) const {
        vd.decl->self->currentScope = currentNodePtr->currentScope;
        setup_scopes(vd.decl->self); 

        vd.initExpr->currentScope = currentNodePtr->currentScope;
        setup_scopes(vd.initExpr);        
    }

    void operator()(enumNode& en) const {
        // Nothing needed here.
        en.name->currentScope = currentNodePtr->currentScope;
        setup_scopes(en.name);
    }

    void operator()(caseNodeType &cs) const {
        if(currentNodePtr->addNewScope)
        {
            auto* temp = currentNodePtr->currentScope;
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

    void operator()(switchNodeType& sw) const {
        sw.var->currentScope = currentNodePtr->currentScope;
        setup_scopes(sw.var);
        sw.caseListTail->currentScope = currentNodePtr->currentScope;
        setup_scopes(sw.caseListTail);
    }

    void operator()(FunctionCall& fc) const {
        /* Loop over all expressions & add scopes */
        for(auto& c: fc.parameterExpressions) {
            if (c == nullptr || c->exprCode == nullptr) { continue; }
            c->exprCode->currentScope = currentNodePtr->currentScope;
            setup_scopes(c->exprCode);
        }  
    }

    void operator()(StatementList& sl) const {
        if(startSymbolTable)
        {
            currentNodePtr->currentScope = new ScopeSymbolTables();
            // This new global symbol table has parentTable = nullptr
            allSymbolTables.push_back(currentNodePtr->currentScope);
            startSymbolTable = false;
        }

        if(currentNodePtr->addNewScope)
        {
            auto* temp = currentNodePtr->currentScope;
            currentNodePtr->currentScope = new ScopeSymbolTables();
            currentNodePtr->currentScope->parentScope = temp;
            allSymbolTables.push_back(currentNodePtr->currentScope);
        }

        for(auto& c: sl.statements) {
            c->currentScope = currentNodePtr->currentScope;
            setup_scopes(c);
        }
    }

    void operator()(FunctionDefn& fn) const {
        // TODO: Revise this when doing the function logic. 

        // Create an extra scope for the statements of the function.
        auto* functionScope = new ScopeSymbolTables();
        functionScope->parentScope = currentNodePtr->currentScope;
        fn.statements->currentScope = functionScope;

        // FIXME: If a function takes no parameters, the list will have one
        // element (the last one) with null data.
        for(auto* param: fn.getParameters()) {
            if(param->varName == nullptr && param->type == nullptr) break;

            param->varName->currentScope = functionScope;
            param->type->currentScope = functionScope;
        }

        fn.name->currentScope = functionScope;
        fn.return_type->currentScope = functionScope;

        // Recursively update the scope of each child statement to use the new function scope.
        if(fn.statements->is<StatementList>()) {
            for (auto &c : fn.statements->asPtr<StatementList>()->statements) {
                c->currentScope = functionScope;
                setup_scopes(c);
            }
        } else {
            setup_scopes(fn.statements);
        }
    }

    void operator()(doWhileNodeType& dw) const {
        if( dw.loop_body->is<StatementList>())
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

    void operator()(whileNodeType& w) const {
        if(w.loop_body->is<StatementList>())
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

    void operator()(forNodeType& f) const {
        if(f.loop_body->is<StatementList>())
        {
            f.init_statement->currentScope = currentNodePtr->currentScope;
            f.loop_condition->currentScope = currentNodePtr->currentScope;
            f.post_loop_statement->currentScope = currentNodePtr->currentScope;
            f.loop_body->currentScope = currentNodePtr->currentScope;
        }
        else
        {
            auto* newTable = new ScopeSymbolTables();
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

    void operator()(BinOp& bin) const {
        bin.lOperand->currentScope = currentNodePtr->currentScope;
        bin.rOperand->currentScope = currentNodePtr->currentScope;
        setup_scopes(bin.lOperand);
        setup_scopes(bin.rOperand);
    }

    void operator()(UnOp& uop) const {
        uop.operand->currentScope = currentNodePtr->currentScope;
        setup_scopes(uop.operand);
    }
    
    void operator()(IfNode& ifNode) const {
        ifNode.condition->currentScope = currentNodePtr->currentScope;
        ifNode.ifCode->currentScope = currentNodePtr->currentScope;

        setup_scopes(ifNode.condition);
        setup_scopes(ifNode.ifCode);

        if(ifNode.elseCode != nullptr) {
            ifNode.elseCode->currentScope = currentNodePtr->currentScope;
            setup_scopes(ifNode.elseCode);
        }
    }

    void operator()(ArrayLiteral& al) const {
        for(auto* expr: al.expressions) {
            expr->currentScope = currentNodePtr->currentScope;
            setup_scopes(expr);
        }
    }

    void operator()(ArrayIndex& al) const {
        al.indexExpr->currentScope = currentNodePtr->currentScope;
        al.arrayExpr->currentScope = currentNodePtr->currentScope;
    }

    // the default case:
    template<typename T> 
    void operator()(T const & /*UNUSED*/ ) const {}

};

void setup_scopes(Node* p) {
    if(p == nullptr) { std::cerr << "null pointer encountered in setup_scopes\n"; }
    std::visit(setup_scopes_visitor{p}, *p);
}
