#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <variant>
#include <algorithm>

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
        auto& varRef = sym[std::get<idNodeType>(opr.op[0]->un).id];   \
        Value temp = varRef;                                           \
        varRef =  varRef oper Value(1);                                \
        return temp;                                                   \
    }

Value ex(nodeType* p);

Value evaluate_switch(switchNodeType& sw) {
    int matching_case_index = -1;
    int default_case_index = -1;
    Value var_value = ex(sw.var);

    assert(std::holds_alternative<caseNodeType>(sw.case_list_head->un));
    std::vector<caseNodeType> cases;

    nodeType* head = sw.case_list_head;

    // Since we have each switch case pointing at the one
    // before it, we need to collect this list and walk it
    // in reverse to have the switch case in the proper
    // order.
    do {
        cases.push_back(std::get<caseNodeType>(head->un));
        head = std::get<caseNodeType>(head->un).prev;
    } while(head != NULL);
    std::reverse(cases.begin(), cases.end());

    for(int i = 0; i < cases.size() && !sw.break_encountered; i++) {
        Value case_value = ex(cases[i].self);
        auto opr = std::get<oprNodeType>(cases[i].self->un);

        if(opr.oper == DEFAULT) {
            default_case_index = i;
        } else if (case_value == var_value || (matching_case_index != -1)) {
            // Once you find a matching case, you execute
            // the following cases until you find a break
            // statement or you exhaust all the remaining
            // cases.

            matching_case_index = i;
            ex(opr.op[1]);
        }
    }

    if(matching_case_index == -1 && !sw.break_encountered) {
        auto opr = std::get<oprNodeType>(cases[default_case_index].self->un);
        ex(opr.op[0]);
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
        // if(m.find(key) == m.end())
        if(sym.find(identifier.id) == sym.end())
        {
            std::string temp = "Undeclared variable "+identifier.id+" !!!\n";
            printf("%s",temp.c_str());
            return Value(0);
        }
        return sym[identifier.id];
    }

    Value operator()(caseNodeType& identifier) {
        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
        exit(EXIT_FAILURE);
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
                return Value(0);
            }

            case ';':  {
                if(!opr.op.empty()) {

                     ex(opr.op[0]);

                     // Check if the current statement is `break`
                     // If so, just execute it (so it can notify its parent
                     // switch, while, for, do-while, etc...), but not the
                     // remaining statements, and exit early.
                     bool should_break = std::visit(
                             Visitor {
                                 [](oprNodeType& oprNode) {
                                    if(oprNode.oper == ';') {
                                    return std::holds_alternative<breakNodeType>(oprNode.op[0]->un) 
                                    ||  std::holds_alternative<breakNodeType>(oprNode.op[1]->un) ;
                                    }
                                    return false;
                                 },
                                 [](auto def) { return false; }
                                 },
                                 opr.op[0]->un
                             );

                     if (should_break)
                       return Value(0);
                }

                if(opr.op.size() > 1) 
                    return ex(opr.op[1]);

                return Value(0);

           }
            case '=':       
            {
                std::string key = std::get<idNodeType>(opr.op[0]->un).id;
                Value val = ex(opr.op[1]);
                if(sym.find(key) == sym.end())
                {
                    
                    std::string temp = "Assigning new variable "+key+" with new value !!!\n";
                    printf("%s",temp.c_str());
                }
                return sym[key] = val;
            }

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
                auto &varRef = sym[std ::get<idNodeType>(opr.op[0]->un).id];
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

