#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "cl.h"
#include "y.tab.h"

static int lbl;

int ex(nodeType* p);

//     switch(j) {
//      default:
//          i *= j;
//      case 0:
//          i += 1;
//      case 10:
//          i /= 2;
//  }
//  
//  compiles down to:
//
//         test    eax, eax
//         je      .L2
//         cmp     eax, 10
//         je      .L3
//         mov     edx, DWORD PTR i[rip]
//         mov     eax, DWORD PTR j[rip]
//         imul    eax, edx
//         mov     DWORD PTR i[rip], eax
// .L2:
//         mov     eax, DWORD PTR i[rip]
//         add     eax, 1
//         mov     DWORD PTR i[rip], eax
// .L3:
//         mov     eax, DWORD PTR i[rip]
//         mov     edx, eax
//         shr     edx, 31
//         add     eax, edx
//         sar     eax
//
int compile_switch(nodeType* p) {

    int matching_case_index = -1;
    int default_case_index = -1;
    int break_encountered = 0;

    assert(p->sw.case_list_head->type == typeCase);
    caseNodeType cases[MAX_SWITCH_CASES];
    int labels[MAX_SWITCH_CASES];
    int num_cases = 0;

    nodeType* head = p->sw.case_list_head;
    do {
        assert(head->type == typeCase);
        cases[num_cases] = head->cs;
        labels[num_cases] = lbl++;
        num_cases++;
        head = head->cs.prev;
    } while(head != NULL);

    p->sw.exit_label = lbl++;

    assert(num_cases < MAX_SWITCH_CASES);

    for(int i = num_cases - 1; i >=0 && !break_encountered ; i--) { 
        if(cases[i].self->opr.oper == DEFAULT) {
            default_case_index = i;
        } else {
            ex(cases[i].self);                       // Label value
            printf("\tpush\t%c\n", p->sw.var->id.i + 'a');   // variable value
            printf("\tcompEQ\n");
            printf("\tje\tL%03d\n", labels[i]);
        }
    }

    // If there exists a default case, jump to it if no other cases
    // match. Otherwise, jump to the exit label (skip the whole swtich).
    if(default_case_index != -1) {
        printf("\tjmp\tL%03d\n", labels[default_case_index]);
    } else {
        printf("\tjmp\tL%03d\n", p->sw.exit_label);
    }

    for(int i = num_cases - 1; i >=0 && !break_encountered ; i--) { 
        printf("L%03d:\n", labels[i]);

        if(cases[i].self->opr.oper == DEFAULT) {
            ex(cases[i].self->opr.op[0]);
        } else {
            ex(cases[i].self->opr.op[1]);
        }
    }

    printf("L%03d:\n", p->sw.exit_label);

    return 0;
}

int ex(nodeType *p) {
    int lbl1, lbl2;

    if (!p) return 0;
    switch(p->type) {
    case typeCon:       
        printf("\tpush\t%d\n", p->con.value); 
        break;
    case typeId:        
        printf("\tpush\t%c\n", p->id.i + 'a'); 
        return sym[p->id.i];
    case typeCase:
        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
        exit(EXIT_FAILURE);
    case typeSwitch: compile_switch(p); break;
    case typeBreak: 
         int exit_label = p->br.parent_switch->sw.exit_label;
         printf("\tjmp\tL%03d\n", exit_label);
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
        case DEFAULT:   
        case CASE:     
            printf("\tpush\t%d\n", p->opr.op[0]->con.value);
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
