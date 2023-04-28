#include <stdio.h>
#include "cl.h"
#include "y.tab.h"

static int lbl;

int ex(nodeType *p) {
    int lbl1, lbl2;

    if (!p) return 0;
    switch(p->type) {
    case typeCon:       
        printf("\tpush\t%d\n", p->con.value); 
        break;
    case typeId:        
        printf("\tpush\t%c\n", p->id.i + 'a'); 
        break;
    case typeOpr:
        switch(p->opr.oper) {
        case WHILE:
            // op[0] is the condition
            // op[1] is the statement list
            
            // label1:
            printf("L%03d:\n", lbl1 = lbl++);

            //      check condition
            ex(p->opr.op[0]);
            
            //      if condition doesn't hold, jump to label2
            printf("\tjz\tL%03d\n", lbl2 = lbl++);

            //      code ...
            ex(p->opr.op[1]);

            //      jump to label1
            printf("\tjmp\tL%03d\n", lbl1);

            //  label2:
            printf("L%03d:\n", lbl2);
            break;
        case DO:
            // op[0] here should be the statement list (code inside the do-while block)
            // op[1] should be the condition

            // label:
            printf("L%03d:\n", lbl1 = lbl++);

            //      code....
            ex(p->opr.op[0]);

            //      condition check 
            ex(p->opr.op[1]);

            //      jump to label if condition holds
            printf("\tjz\tL%03d\n", lbl1);

            break;
        case IF:
            ex(p->opr.op[0]);
            switch(p->opr.nops) {
                /* if op[0] { op[1] } else { op[2] } */
                case 3:
                    printf("\tjz\tL%03d\n", lbl1 = lbl++);
                    ex(p->opr.op[1]);
                    printf("\tjmp\tL%03d\n", lbl2 = lbl++);
                    printf("L%03d:\n", lbl1);
                    ex(p->opr.op[2]);
                    printf("L%03d:\n", lbl2);
                    break;

                /* if op[0] { op[1] } */
                case 2:
                    /* if */
                    printf("\tjz\tL%03d\n", lbl1 = lbl++);
                    ex(p->opr.op[1]);
                    printf("L%03d:\n", lbl1);
                    break;

                /* if op[0]; */
                // Shouldn't be too common, but I think we should handle it.
                case 1:
                    printf("\tjz\tL%03d\n", lbl1 = lbl++);
                    break;
            }
            break;
        case FOR:
            // op[0] shouldd be the initialzation expression, called only once.
            // op[1] should be the loop condition, checked before every iteration.
            // op[2] should be the loop post-iteration condition, called after every iteration.
            // op[3] should be the loop body.

            // Should go like this:
            //      loop initialization expression  (op[0])
            //  label1:
            //      check loop condition    (op[1])
            //      if it doesn't hold jump to label2
            //
            //      code... (op[3])
            //
            //      loop post-iteration code (op[2])
            //      jump to label1
            //  label2:
            

            ex(p->opr.op[0]);

            printf("L%03d:\n", lbl1 = lbl++);

            ex(p->opr.op[1]);

            printf("\tjnz\tL%03d\n", lbl2 = lbl++);

            ex(p->opr.op[3]);
            
            ex(p->opr.op[2]);

            printf("\tjmp\tL%03d\n", lbl1);

            printf("L%03d:\n", lbl2);
            break;
        case PRINT:     
            ex(p->opr.op[0]);
            printf("\tprint\n");
            break;
        case '=':       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case PA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case SA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case MA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case DA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case RA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case LSA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case RSA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case ANDA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case EORA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case IORA:       
            ex(p->opr.op[1]);
            printf("\tpop\t%c\n", p->opr.op[0]->id.i + 'a');
            break;
        case PP:    
            ex(p->opr.op[0]);
            printf("\tinc1pre\n");
            break;
        case MM:    
            ex(p->opr.op[0]);
            printf("\tdec1pre\n");
            break;
        case UPLUS:    
            ex(p->opr.op[0]);
            printf("\tpos\n");
            break;
        case UMINUS:    
            ex(p->opr.op[0]);
            printf("\tneg\n");
            break;
        case '!':    
            ex(p->opr.op[0]);
            printf("\tnot\n");
            break;
        case '~':    
            ex(p->opr.op[0]);
            printf("\tbitNOT\n");
            break;
       default:
            ex(p->opr.op[0]);
            ex(p->opr.op[1]);
            switch(p->opr.oper) {
            case '&':   printf("\tbitAND\n"); break;
            case '|':   printf("\tbitOR\n");  break;
            case '^':   printf("\tbitXOR\n"); break;
            case LS:    printf("\tLshift\n"); break;
            case RS:    printf("\tRshift\n"); break;
            case '+':   printf("\tadd\n");    break;
            case '-':   printf("\tsub\n");    break; 
            case '*':   printf("\tmul\n");    break;
            case '/':   printf("\tdiv\n");    break;
            case '%':   printf("\tmod\n");    break;
            case AND:   printf("\tand\n");    break;
            case OR:    printf("\tor\n");     break;
            case '<':   printf("\tcompLT\n"); break;
            case '>':   printf("\tcompGT\n"); break;
            case GE:    printf("\tcompGE\n"); break;
            case LE:    printf("\tcompLE\n"); break;
            case NE:    printf("\tcompNE\n"); break;
            case EQ:    printf("\tcompEQ\n"); break;
            }
        }
    }
    return 0;
}
