#include <stdio.h>
#include "cl.h"
#include "y.tab.h"

extern int switch_var;
extern int break_encountered;


int ex(nodeType *p) {
    if (!p) return 0;
    switch(p->type) {
    case typeCon:       return p->con.value;
    case typeId:        return sym[p->id.i];
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

        case SWITCH:
                        // op[0] should be the variable value
                        // op[1] should be the case list

                        switch_var = ex(p->opr.op[0]);
                        break_encountered = 0;

                        ex(p->opr.op[1]);
                        break;
        case CASE:
                        if(break_encountered) return 0;

                        if(p->opr.nops == 2 && ex(p->opr.op[0]) == switch_var) {
                            // Normal case
                            ex(p->opr.op[1]);
                        } else if(p->opr.nops == 1) {
                            // Default case
                            ex(p->opr.op[0]);
                        }

                        return 0;
        case BREAK:     break_encountered = 1; return 0;
        case PRINT:     printf("%d\n", ex(p->opr.op[0])); return 0;
        case CASE_LIST:
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
