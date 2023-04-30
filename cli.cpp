#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <variant>
#include <algorithm>

#include "cl.h"
#include "y.tab.h"

float ex(nodeType* p);

int evaluate_switch(switchNodeType& sw) {
    int matching_case_index = -1;
    int default_case_index = -1;
    int var_value = ex(sw.var);

    assert(sw.case_list_head->type == typeCase);
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
        int case_value = ex(cases[i].self);
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

    return 0;
}

struct ex_visitor {
    float operator()(conNodeType& con) {
        switch(con.conType) {
            case intType: return con.iValue;
            case floatType: return con.fValue;
            case boolType: return (float)con.bValue;
            default:
                    return 0;
        }
    }

    float operator()(idNodeType& identifier) {
        return sym2[identifier.id];
    }

    float operator()(caseNodeType& identifier) {
        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
        exit(EXIT_FAILURE);
    }

    float operator()(switchNodeType& sw) {
        return evaluate_switch(sw);
    }

    float operator()(breakNodeType& br) {
        auto parent_switch = std::get<switchNodeType>(br.parent_switch->un);
        parent_switch.break_encountered = true;
        return 0;
    }

    float operator()(oprNodeType& opr) {
        switch(opr.oper) {
            case WHILE:     while(ex(opr.op[0])) ex(opr.op[1]); return 0;
            case DO:        do { ex(opr.op[1]); } while(ex(opr.op[0])); return 0;
            case IF:        if (ex(opr.op[0]))
                                ex(opr.op[1]);
                            else if (opr.op.size() > 2)
                                ex(opr.op[2]);
                            return 0;
            case FOR:       
                            for(ex(opr.op[0]); ex(opr.op[1]); ex(opr.op[2])) {
                                ex(opr.op[3]);
                            }
                            return 0;

            case DEFAULT:   break;
            case CASE:      return ex(opr.op[0]);

            case PRINT:  
            {
                // auto val = std::get<conNodeType>(opr.op[0]->un);
                printf("%f\n", ex(opr.op[0]));
                // if(val.conType == intType) {
                //     printf("%d\n", ((int)ex(opr.op[0])));
                // }    else if(val.conType == floatType)    {
                //     printf("%f\n", ex(opr.op[0]));
                // }    else if(val.conType == boolType) {
                //         if(ex(opr.op[0]) == 0)
                //             printf("false\n");
                //         else if(ex(opr.op[0]) == 1)
                //             printf("true\n");
                // }    
                /* else if(p->con.conType == charType) {
                        printf("%c\n", itoa(ex(p->opr.op[0])));
                }    
                 else if(type ==  stringType) {
                    p->con.sValue = sValue;
                } */
                return 0;
            }
            case ';':       ex(opr.op[0]); return ex(opr.op[1]);
            case '=':       return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ex(opr.op[1]);
            case PA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] += ex(opr.op[1]);
            case SA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] -= ex(opr.op[1]);
            case MA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] *= ex(opr.op[1]);
            case DA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] /= ex(opr.op[1]);
            case RA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ((int)sym2[std::get<idNodeType>(opr.op[0]->un).id]) % ((int)ex(opr.op[1]));
            case LSA:       return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ((int)sym2[std::get<idNodeType>(opr.op[0]->un).id]) << ((int)ex(opr.op[1]));
            case RSA:       return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ((int)sym2[std::get<idNodeType>(opr.op[0]->un).id]) >> ((int)ex(opr.op[1]));
            case ANDA:      return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ((int)sym2[std::get<idNodeType>(opr.op[0]->un).id]) & ((int)ex(opr.op[1]));
            case EORA:      return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ((int)sym2[std::get<idNodeType>(opr.op[0]->un).id]) ^ ((int)ex(opr.op[1]));
            case IORA:      return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ((int)sym2[std::get<idNodeType>(opr.op[0]->un).id]) | ((int)ex(opr.op[1]));
            case PP:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ex(opr.op[0]) + 1;
            case MM:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ex(opr.op[0]) - 1;
            case UPLUS:     return ex(opr.op[0]);
            case UMINUS:    return -ex(opr.op[0]);
            case '!':       return !ex(opr.op[0]);
            case '~':       return ~((int)ex(opr.op[0]));
            case '&':       return ((int)ex(opr.op[0])) & ((int)ex(opr.op[1]));
            case '|':       return ((int)ex(opr.op[0])) | ((int)ex(opr.op[1]));
            case '^':       return ((int)ex(opr.op[0])) ^ ((int)ex(opr.op[1]));
            case LS:        return ((int)ex(opr.op[0])) << ((int)ex(opr.op[1]));
            case RS:        return ((int)ex(opr.op[0])) >> ((int)ex(opr.op[1]));
            case '+':       return ex(opr.op[0]) + ex(opr.op[1]);
            case '-':       return ex(opr.op[0]) - ex(opr.op[1]);
            case '*':       return ex(opr.op[0]) * ex(opr.op[1]);
            case '/':       return ex(opr.op[0]) / ex(opr.op[1]);
            case '%':       return ((int)ex(opr.op[0])) % ((int)ex(opr.op[1]));
            case AND:       return ex(opr.op[0]) && ex(opr.op[1]);
            case OR:        return ex(opr.op[0]) || ex(opr.op[1]);
            case '<':       return ex(opr.op[0]) < ex(opr.op[1]);
            case '>':       return ex(opr.op[0]) > ex(opr.op[1]);
            case GE:        return ex(opr.op[0]) >= ex(opr.op[1]);
            case LE:        return ex(opr.op[0]) <= ex(opr.op[1]);
            case NE:        return ex(opr.op[0]) != ex(opr.op[1]);
            case EQ:        return ex(opr.op[0]) == ex(opr.op[1]);
        }

        return 0;
    }

    // the default case:
    template<typename T> float operator()(T const &) const { return 0; } 

};

float ex(nodeType *p) {
    if (p == nullptr) return 0;
    return std::visit(ex_visitor(), p->un);
}
