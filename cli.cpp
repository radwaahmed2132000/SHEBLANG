#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <variant>

#include "cl.h"
#include "y.tab.h"

int ex(nodeType* p);

int evaluate_switch(nodeType* p) {
    auto sw = std::get<switchNodeType>(p->un);

    int matching_case_index = -1;
    int default_case_index = -1;
    int var_value = ex(sw.var);

    assert(sw.case_list_head->type == typeCase);
    caseNodeType cases[MAX_SWITCH_CASES];
    int num_cases = 0;

    nodeType* head = sw.case_list_head;

    // Since we have each switch case pointing at the one
    // before it, we need to collect this list and walk it
    // in reverse to have the switch case in the proper
    // order.
    do {
        /* assert(head->type == typeCase); */
        cases[num_cases++] = std::get<caseNodeType>(head->un);
        head = std::get<caseNodeType>(head->un).prev;
    } while(head != NULL);

    assert(num_cases < MAX_SWITCH_CASES);

    for(int i = num_cases - 1; i >=0 && !sw.break_encountered; i--) {
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

int ex(nodeType *p) {
    if (!p) return 0;
    switch(p->type) {
        case typeCon:       return std::get<conNodeType>(p->un).value;
        case typeId:        return sym2[std::get<idNodeType>(p->un).id];
    case typeCase:
                        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
                        exit(EXIT_FAILURE);
    case typeSwitch:    evaluate_switch(p); break;
    case typeBreak: {
                        auto br = std::get<breakNodeType>(p->un);
                        auto parent_switch = std::get<switchNodeType>(br.parent_switch->un);
                        parent_switch.break_encountered = 1;
                    } break;
    case typeOpr: {
        auto opr = std::get<oprNodeType>(p->un);
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

            case PRINT:     printf("%d\n", ex(opr.op[0])); return 0;
            case ';':       ex(opr.op[0]); return ex(opr.op[1]);
            case '=':       return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ex(opr.op[1]);
            case PA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] += ex(opr.op[1]);
            case SA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] -= ex(opr.op[1]);
            case MA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] *= ex(opr.op[1]);
            case DA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] /= ex(opr.op[1]);
            case RA:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] %= ex(opr.op[1]);
            case LSA:       return sym2[std::get<idNodeType>(opr.op[0]->un).id] <<= ex(opr.op[1]);
            case RSA:       return sym2[std::get<idNodeType>(opr.op[0]->un).id] >>= ex(opr.op[1]);
            case ANDA:      return sym2[std::get<idNodeType>(opr.op[0]->un).id] &= ex(opr.op[1]);
            case EORA:      return sym2[std::get<idNodeType>(opr.op[0]->un).id] ^= ex(opr.op[1]);
            case IORA:      return sym2[std::get<idNodeType>(opr.op[0]->un).id] |= ex(opr.op[1]);
            case PP:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ex(opr.op[0]) + 1;
            case MM:        return sym2[std::get<idNodeType>(opr.op[0]->un).id] = ex(opr.op[0]) - 1;
            case UPLUS:     return ex(opr.op[0]);
            case UMINUS:    return -ex(opr.op[0]);
            case '!':       return !ex(opr.op[0]);
            case '~':       return ~ex(opr.op[0]);
            case '&':       return ex(opr.op[0]) & ex(opr.op[1]);
            case '|':       return ex(opr.op[0]) | ex(opr.op[1]);
            case '^':       return ex(opr.op[0]) ^ ex(opr.op[1]);
            case LS:        return ex(opr.op[0]) << ex(opr.op[1]);
            case RS:        return ex(opr.op[0]) >> ex(opr.op[1]);
            case '+':       return ex(opr.op[0]) + ex(opr.op[1]);
            case '-':       return ex(opr.op[0]) - ex(opr.op[1]);
            case '*':       return ex(opr.op[0]) * ex(opr.op[1]);
            case '/':       return ex(opr.op[0]) / ex(opr.op[1]);
            case '%':       return ex(opr.op[0]) % ex(opr.op[1]);
            case AND:       return ex(opr.op[0]) && ex(opr.op[1]);
            case OR:        return ex(opr.op[0]) || ex(opr.op[1]);
            case '<':       return ex(opr.op[0]) < ex(opr.op[1]);
            case '>':       return ex(opr.op[0]) > ex(opr.op[1]);
            case GE:        return ex(opr.op[0]) >= ex(opr.op[1]);
            case LE:        return ex(opr.op[0]) <= ex(opr.op[1]);
            case NE:        return ex(opr.op[0]) != ex(opr.op[1]);
            case EQ:        return ex(opr.op[0]) == ex(opr.op[1]);
        }
        }
    }
    return 0;
}
