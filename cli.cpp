#include <cstddef>
#include <iterator>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <variant>
#include <algorithm>
#include <optional>

#include "cl.h"
#include "result.h"
#include "y.tab.h"

#define BOP_CASE(case_value, oper) \
            case case_value: return ex(opr.op[0]) oper ex(opr.op[1]);
                             
#define UOP_CASE(case_value, oper) \
            case case_value: return oper(ex(opr.op[0]));

#define BOOL_BOP_CASE(case_value, oper) \
            case case_value: return Value(ex(opr.op[0]) oper ex(opr.op[1]));
                             
#define BOOL_UOP_CASE(case_value, oper) \
            case case_value: return Value(oper (ex(opr.op[0])));

#define POST_OP(oper) {                                                \
        auto nameStr = std ::get<idNodeType>(opr.op[0]->un).id;        \
        auto& varEntry = varSymTableEntry(nameStr, p->currentScope);   \
        auto &varRef = varEntry.getRef();                              \
        Value temp = varRef;                                           \
        varRef = varRef oper Value(1);                                 \
        return temp;                                                   \
    }

Value ex(nodeType* p);

Value evaluate_switch(switchNodeType& sw) {
    std::optional<int> matching_case_index = {};
    std::optional<int> default_case_index = {};
    Value var_value = ex(sw.var);

    assert(std::holds_alternative<caseNodeType>(sw.case_list_head->un));

    nodeType* head = sw.case_list_head;
    auto cases = std::get<caseNodeType>(head->un).toVec();

    bool foundCase = false;
    for(int i = 0; i < cases.size(); i++) {
        Value case_value = ex(cases[i]->labelExpr);

        if(cases[i]->isDefault()) {
            default_case_index = i;
        } else if (case_value == var_value || matching_case_index.has_value()) {
            // break once you find a matching case
            matching_case_index = i;
            ex(cases[i]->caseBody);
            break;
        }
    }

    if(!matching_case_index.has_value() &&  default_case_index.has_value()) {
        ex(cases[default_case_index.value()]->caseBody);
    }

    return Value(0);
}


struct ex_visitor {
    // pointer to the node of the current variant
    // since we need scope info from it.
    nodeType* p;

    Value operator()(conNodeType& con) {
        return std::visit(
                Visitor {
                    [](int iValue)         { return Value(iValue); },
                    [](bool bValue)        { return Value(bValue); },
                    [](char cValue)        { return Value(cValue); },
                    [](float fValue)       { return Value(fValue); },
                    [](std::string sValue) { return Value(sValue); },
                },
                con
        );
    }

    Value operator()(idNodeType& identifier) const {
        auto& varSymbolTableEntry = varSymTableEntry(identifier.id, p->currentScope);
        return varSymbolTableEntry.getValue();
    }

    Value operator()(VarDecl& vd) const {
        auto nameStr = std::get<idNodeType>(vd.var_name->un).id;
        auto& varSymbolTableEntry = varSymTableEntry(nameStr, p->currentScope);

        varSymbolTableEntry.setValue(ex(varSymbolTableEntry.initExpr));

        return Value(0);
    }

    Value operator()(VarDefn& vd) const {
        auto nameStr = std::get<idNodeType>(vd.decl->var_name->un).id;
        auto val = ex(vd.initExpr);

        auto& varSymbolTableEntry = varSymTableEntry(nameStr, p->currentScope);
        varSymbolTableEntry.setValue(val);

        return Value(0);
    }

    Value operator()(enumUseNode& eu) const {
        auto& e = enumSymTableEntry(eu.enumName, p->currentScope);
        auto memberIter = std::find(e.enumMembers.begin(), e.enumMembers.end(), eu.memberName);
        int enumMemberValue = std::distance(e.enumMembers.begin(), memberIter);
        return Value(enumMemberValue);
    }

    Value operator()(switchNodeType& sw) {
        return evaluate_switch(sw);
    }

    Value operator()(breakNodeType& br) {
        std::visit(
                Visitor {
                    [](switchNodeType& sw) { sw.break_encountered = true; },
                    [](whileNodeType& w) { w.break_encountered = true; },
                    [](forNodeType& f) { f.break_encountered = true; },
                    [](doWhileNodeType& dw) { dw.break_encountered = true; },
                    [](auto def) {}
                },
                br.parent_switch->un
        );
        return Value(0);
    }

    Value operator()(FunctionCall& fc) {

        // For each argument, find the corresponding parameter in the symbol table,
        // then insert the argument value. 
        auto* currentScope = p->currentScope;
        
        using FnIter = std::unordered_map<std::string, functionNodeType>::iterator;
        FnIter fnIter;
        while(currentScope != nullptr) {
            auto fns = currentScope->functions;
            fnIter = fns.find(fc.functionName);
            if(fnIter != fns.end()) {
                break;
            } else {
                currentScope = currentScope->parentScope;
            }
        }

        // We can dereference the iterator just fine because symantic analysis 
        // succeed unless we can reach the function in its proper scope.
        auto& fnRef = fnIter->second;
        auto fnParams = fnRef.parametersTail->toVec();
        auto* fnScope = fnRef.statements->currentScope;

        for(int i = 0; i < fnParams.size(); i++) {
            if(fnParams[i]->type == nullptr && fnParams[i]->var_name == nullptr) continue;
            fnScope->sym2[fnParams[i]->getName()].setValue(ex(fc.parameterExpressions[i]->exprCode));
        }

        return ex(fnRef.statements);
    }

    Value operator()(StatementList& sl) {
        auto statements = sl.toVec();

        for(const auto& statement: statements) {
            auto ret = ex(statement->statementCode);
            if(std::holds_alternative<breakNodeType>(statement->statementCode->un)) {
                return Value(0);
            } else if (
                auto *opr = std::get_if<oprNodeType>( &(statement->statementCode->un));
                opr != nullptr && opr->oper == RETURN
            ) {
                return ret;
            }
        }

        return Value(0);
    }

    Value operator()(doWhileNodeType& dw) {
        do {
            ex(dw.loop_body); 
        } while(ex(dw.condition) && !dw.break_encountered); 

        return Value(0);
    }

    Value operator()(whileNodeType& w) {
        while(ex(w.condition) && !w.break_encountered) 
            ex(w.loop_body); 

        return Value(0);
    }

    Value operator()(forNodeType& f) {
        for(ex(f.init_statement); ex(f.loop_condition) && !f.break_encountered; ex(f.post_loop_statement)) {
            ex(f.loop_body);
        }
        return Value(0);
    }

    Value operator()(oprNodeType& opr) const {
        switch(opr.oper) {
            case IF: {
                 if (ex(opr.op[0]))
                     ex(opr.op[1]);
                 else if (opr.op.size() > 2)
                     ex(opr.op[2]);
                 return Value(0);
            }

            case DEFAULT:   break;
            case CASE:      return ex(opr.op[0]);

            case PRINT:  
            {
                Value exprValue = ex(opr.op[0]);
                std::visit(
                    Visitor {
                        [](int iValue)         { printf("%d\n", iValue); },
                        [](bool bValue)        { printf("%s\n", bValue? "true": "false"); },
                        [](char cValue)        { printf("%c\n", cValue); },
                        [](float fValue)       { printf("%f\n", fValue); },
                        [](std::string sValue) { printf("%s\n", sValue.c_str()); },
                    },
                    exprValue
                );
                std::cout << std::flush;
                return Value(0);
            }
            case '=':       {

                using std::optional, std::string, std::make_optional, std::visit, std::get;
                using namespace std::string_literals;

                // Get the variable name based on the LHS's type.
                optional<string> varNameOpt = visit(
                    Visitor {
                        [&opr](VarDecl& varDecl)              { return make_optional(get<idNodeType>(varDecl.var_name->un).id); },
                        [&opr](VarDefn& varDefn)              { return make_optional(get<idNodeType>(varDefn.decl->var_name->un).id); },
                        [&opr](idNodeType& idNode)            { return make_optional(idNode.id); },
                        [](auto _default) -> optional<string> {  return std::nullopt; }
                    } ,
                    opr.op[0]->un
                );

                if(!varNameOpt.has_value()) { std::cout << "Invalid assignment expression"; }
                auto varName = varNameOpt.value();

                // Get the symbol table entry of the scope containing the variable.
                auto& symTable = varSymTableEntry(varName, p->currentScope);

                // Assign the value in the scope table.
                return visit(
                    Visitor {
                        [&](VarDecl& varDecl) { return symTable.setValue(ex(opr.op[1])).getValue(); },
                        [&](VarDefn& varDefn) { return symTable.setValue(ex(varDefn.initExpr)).getValue(); },
                        [&](idNodeType& idNode) { return symTable.setValue(ex(opr.op[1])).getValue(); },
                        [](auto _default) { return Value(0); }  // Should never reach here
                    } ,
                    opr.op[0]->un
                );
            }

            case RETURN:
                return ex(opr.op[0]);

            BOP_CASE('+',+)
            BOP_CASE('-',-)
            BOP_CASE('*',*) 
            BOP_CASE('/',/) 

            BOP_CASE('&',&) 
            BOP_CASE('|',|) 
            BOP_CASE('^',^)
            UOP_CASE('~',~) 

            BOP_CASE(LS,<<)
            BOP_CASE(RS,>>)
            BOP_CASE('%',%) 

            BOOL_BOP_CASE('<',<) 
            BOOL_BOP_CASE('>',>) 
            BOOL_BOP_CASE(AND, &&)
            BOOL_BOP_CASE(OR, ||)
            BOOL_BOP_CASE(GE,>=)        
            BOOL_BOP_CASE(LE,<=)        
            BOOL_BOP_CASE(NE,!=)        
            BOOL_BOP_CASE(EQ,==)        
            BOOL_UOP_CASE('!',!) 

            UOP_CASE(UPLUS,+)     
            UOP_CASE(UMINUS,-)

            case PP: POST_OP(+)
            case MM: POST_OP(-)
            default: return Value(0);
        }

        return Value(0);
    }

    // the default case:
    template<typename T> 
    Value operator()(T const & /*UNUSED*/ ) const { return Value(0);} 
};

Value ex(nodeType *p) {
    if (p == nullptr) return Value(0);
    return std::visit(ex_visitor{p}, p->un);
}

