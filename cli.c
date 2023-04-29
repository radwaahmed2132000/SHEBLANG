#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "cl.h"
#include "y.tab.h"

int ex(nodeType* p);

int evaluate_switch(nodeType* p) {
    int matching_case_index = -1;
    int default_case_index = -1;
    int var_value = ex(p->sw.var);

    assert(p->sw.case_list_head->type == typeCase);
    caseNodeType cases[MAX_SWITCH_CASES];
    int num_cases = 0;

    nodeType* head = p->sw.case_list_head;

    // Since we have each switch case pointing at the one
    // before it, we need to collect this list and walk it
    // in reverse to have the switch case in the proper
    // order.
    do {
        assert(head->type == typeCase);
        cases[num_cases++] = head->cs;
        head = head->cs.prev;
    } while(head != NULL);

    assert(num_cases < MAX_SWITCH_CASES);

    for(int i = num_cases - 1; i >=0 && !p->sw.break_encountered; i--) {
        int case_value = ex(cases[i].self);
        if(cases[i].self->opr.oper == DEFAULT) {
            default_case_index = i;
        } else if (case_value == var_value || (matching_case_index != -1)) {
            // Once you find a matching case, you execute
            // the following cases until you find a break
            // statement or you exhaust all the remaining
            // cases.

            matching_case_index = i;
             ex(cases[i].self->opr.op[1]);
        }
    }

    if(matching_case_index == -1 && !p->sw.break_encountered) {
        ex(cases[default_case_index].self->opr.op[0]);
    }

    return 0;
}

int ex(nodeType *p) {
    if (!p) return 0;
    switch(p->type) {
    case typeCon:       return p->con.value;
    case typeId:        return sym[p->id.i];
    case typeCase:
                        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
                        exit(EXIT_FAILURE);
    case typeSwitch:    evaluate_switch(p); break;
    case typeBreak: 
                        p->br.parent_switch->sw.break_encountered = 1; break;
    case typeOpr:
        switch(p->opr.oper) {
        case WHILE:     while(ex(p->opr.op[0])) ex(p->opr.op[1]); return 0;
        case DO:        do { ex(p->opr.op[1]); } while(ex(p->opr.op[0])); return 0;
        case IF:        if (ex(p->opr.op[0]))
                            ex(p->opr.op[1]);
                        else if (p->opr.nops > 2)
                            ex(p->opr.op[2]);
                        return 0;
        case FOR:       
                        for(ex(p->opr.op[0]); ex(p->opr.op[1]); ex(p->opr.op[2])) {
                            ex(p->opr.op[3]);
                        }
                        return 0;

        case DEFAULT:   break;
        case CASE:      return ex(p->opr.op[0]);

        case PRINT:     printf("%d\n", ex(p->opr.op[0])); return 0;
        case ';':       ex(p->opr.op[0]); return ex(p->opr.op[1]);
        case '=':       return sym[p->opr.op[0]->id.i] = ex(p->opr.op[1]);
        case PA:        return sym[p->opr.op[0]->id.i] += ex(p->opr.op[1]);
        case SA:        return sym[p->opr.op[0]->id.i] -= ex(p->opr.op[1]);
        case MA:        return sym[p->opr.op[0]->id.i] *= ex(p->opr.op[1]);
        case DA:        return sym[p->opr.op[0]->id.i] /= ex(p->opr.op[1]);
        case RA:        return sym[p->opr.op[0]->id.i] %= ex(p->opr.op[1]);
        case LSA:       return sym[p->opr.op[0]->id.i] <<= ex(p->opr.op[1]);
        case RSA:       return sym[p->opr.op[0]->id.i] >>= ex(p->opr.op[1]);
        case ANDA:      return sym[p->opr.op[0]->id.i] &= ex(p->opr.op[1]);
        case EORA:      return sym[p->opr.op[0]->id.i] ^= ex(p->opr.op[1]);
        case IORA:      return sym[p->opr.op[0]->id.i] |= ex(p->opr.op[1]);
        case PP:        return sym[p->opr.op[0]->id.i] = ex(p->opr.op[0]) + 1;
        case MM:        return sym[p->opr.op[0]->id.i] = ex(p->opr.op[0]) - 1;
        case UPLUS:     return ex(p->opr.op[0]);
        case UMINUS:    return -ex(p->opr.op[0]);
        case '!':       return !ex(p->opr.op[0]);
        case '~':       return ~ex(p->opr.op[0]);
        case '&':       return ex(p->opr.op[0]) & ex(p->opr.op[1]);
        case '|':       return ex(p->opr.op[0]) | ex(p->opr.op[1]);
        case '^':       return ex(p->opr.op[0]) ^ ex(p->opr.op[1]);
        case LS:        return ex(p->opr.op[0]) << ex(p->opr.op[1]);
        case RS:        return ex(p->opr.op[0]) >> ex(p->opr.op[1]);
        case '+':       return ex(p->opr.op[0]) + ex(p->opr.op[1]);
        case '-':       return ex(p->opr.op[0]) - ex(p->opr.op[1]);
        case '*':       return ex(p->opr.op[0]) * ex(p->opr.op[1]);
        case '/':       return ex(p->opr.op[0]) / ex(p->opr.op[1]);
        case '%':       return ex(p->opr.op[0]) % ex(p->opr.op[1]);
        case AND:       return ex(p->opr.op[0]) && ex(p->opr.op[1]);
        case OR:        return ex(p->opr.op[0]) || ex(p->opr.op[1]);
        case '<':       return ex(p->opr.op[0]) < ex(p->opr.op[1]);
        case '>':       return ex(p->opr.op[0]) > ex(p->opr.op[1]);
        case GE:        return ex(p->opr.op[0]) >= ex(p->opr.op[1]);
        case LE:        return ex(p->opr.op[0]) <= ex(p->opr.op[1]);
        case NE:        return ex(p->opr.op[0]) != ex(p->opr.op[1]);
        case EQ:        return ex(p->opr.op[0]) == ex(p->opr.op[1]);
        }
    }
    return 0;
}
