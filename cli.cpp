#include <iterator>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <variant>
#include <algorithm>
#include <optional>

#include "cl.h"
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
        auto& varRef = sym2[std::get<idNodeType>(opr.op[0]->un).id].getRef();   \
        Value temp = varRef;                                           \
        varRef =  varRef oper Value(1);                                \
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

    for(int i = 0; i < cases.size() && !sw.break_encountered; i++) {
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

    if(!matching_case_index.has_value() && !sw.break_encountered && default_case_index.has_value()) {
        ex(cases[default_case_index.value()]->caseBody);
    }

    return Value(0);
}

// TODO: Implement function logic.
Value evaluate_function(functionNodeType& fn) {
    return Value(0);
}

struct ex_visitor {
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

    Value operator()(idNodeType& identifier) {
        return sym2[identifier.id].getValue();
    }

    Value operator()(VarDecl& vd) {
        auto nameStr = std::get<idNodeType>(vd.var_name->un).id;
        auto& entry = sym2[nameStr];

        if(entry.isConstant) {
            entry.setValue(ex(entry.initExpr));
        }

        return Value(0);
    }

    Value operator()(enumUseNode& eu) {
        auto e = enums[eu.enumName];
        auto memberIter = std::find(e.enumMembers.begin(), e.enumMembers.end(), eu.memberName);
        
        // TODO: move to semantic analysis
        // if(memberIter == e.enumMembers.end()) {
        //     std::cerr << "Enum" << '(' << eu.enumName << ')' << "does not contain a member with the name '" << eu.memberName << "'\n";
        // }

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

    Value operator()(StatementList& sl) {
        auto statements = sl.toVec();

        for(const auto& statement: statements) {
            ex(statement->statementCode);
            if(std::holds_alternative<breakNodeType>(statement->statementCode->un)) {
                break;
            }
        }

        return Value(0);
    }

    Value operator()(functionNodeType& fn) {
        return evaluate_function(fn);
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

    Value operator()(oprNodeType& opr) {
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
                return std::visit(
                    Visitor {
                        [&opr](VarDecl& varDecl) { 
                            auto varNameIdNode = std::get<idNodeType>(varDecl.var_name->un);
                            return sym2[varNameIdNode.id].setValue(ex(opr.op[1])).getValue(); 
                        },
                        [&opr](idNodeType& idNode) { 
                            auto ret = ex(opr.op[1]);
                            return sym2[idNode.id].setValue(ret).getValue(); 
                        },
                        [](auto _default) { std::cout << "Invalid assignment expression"; return Value(0); }
                    } ,
                    opr.op[0]->un
                );
            }

            // TODO: figure out how to return values in the interpreter
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

            case PP: {
                auto &varRef = sym2[std ::get<idNodeType>(opr.op[0]->un).id].getRef();
                Value temp = varRef;
                varRef = varRef + Value(1);
                return temp;
            }
            case MM:   POST_OP(-)
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
    return std::visit(ex_visitor(), p->un);
}

