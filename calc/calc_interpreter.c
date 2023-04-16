#include <stdio.h>
#include "calc_helpers.h"
#include "calc.tab.h"

int evaluate(ParserNode* p) {
    if(!p) return 0;
    switch(p->type) {
        case TypeConstant:   return p->constant.value;
        case TypeIdentifier: return sym[p->identifier.symbol_table_index];
        case TypeOperator:
            switch(p->opr.oper) {
                case WHILE: 
                    while(evaluate(p->opr.operands[0]))
                        evaluate(p->opr.operands[1]);
                    return 0;

                case IF:
                    if(evaluate(p->opr.operands[0]))
                        evaluate(p->opr.operands[1]);
                    else if (p->opr.num_operands > 2)
                        evaluate(p->opr.operands[2]);
                    return 0;

                case PRINT:
                    printf("%d\n", evaluate(p->opr.operands[0]));
                    return 0;

                case ';':
                    evaluate(p->opr.operands[0]);
                    return evaluate(p->opr.operands[1]);

                case '='   : return sym[p->opr.operands[0]->identifier.symbol_table_index] = evaluate(p->opr.operands[1]);
                case UMINUS: return -evaluate(p->opr.operands[0]);
                case '+'   : return evaluate(p->opr.operands[0]) + evaluate(p->opr.operands[1]);
                case '-'   : return evaluate(p->opr.operands[0]) - evaluate(p->opr.operands[1]);
                case '*'   : return evaluate(p->opr.operands[0]) * evaluate(p->opr.operands[1]);
                case '/'   : return evaluate(p->opr.operands[0]) / evaluate(p->opr.operands[1]);
                case '<'   : return evaluate(p->opr.operands[0]) < evaluate(p->opr.operands[1]);
                case '>'   : return evaluate(p->opr.operands[0]) > evaluate(p->opr.operands[1]);
                case GE    : return evaluate(p->opr.operands[0]) >= evaluate(p->opr.operands[1]);
                case LE    : return evaluate(p->opr.operands[0]) <= evaluate(p->opr.operands[1]);
                case NE    : return evaluate(p->opr.operands[0]) != evaluate(p->opr.operands[1]);
                case EQ    : return evaluate(p->opr.operands[0]) == evaluate(p->opr.operands[1]);
            }
    }

    return 0;
}
