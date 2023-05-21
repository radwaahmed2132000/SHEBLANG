#include <algorithm>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <variant>

#include "cl.h"
#include "y.tab.h"

static int lbl;

float ex(nodeType* p);

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
    bool break_encountered = false;

    auto sw = std::get<switchNodeType>(p->un);

    std::vector<caseNodeType> cases;
    std::vector<int> labels;

    // Check cli.cpp for an explanation of why we're collecting and reversing.
    nodeType* head = sw.case_list_head;
    do {
        auto cs = std::get<caseNodeType>(head->un);

        cases.push_back(cs);
        labels.push_back(lbl++);

        head = cs.prev;
    } while(head != NULL);
    std::reverse(cases.begin(), cases.end());
    std::reverse(labels.begin(), labels.end());

    sw.exit_label = lbl++;

    for(int i = 0; i < cases.size() && !break_encountered ; i++) { 
        auto opr = std::get<oprNodeType>(cases[i].self->un);
        if(opr.oper == DEFAULT) {
            default_case_index = i;
        } else {
            auto case_identifier = std::get<idNodeType>(sw.var->un);
            ex(cases[i].self);                                    // Label value
            printf("\tpush\t%s\n", case_identifier.id.c_str());   // variable value
            printf("\tcompEQ\n");
            printf("\tje\tL%03d\n", labels[i]);
        }
    }

    // If there exists a default case, jump to it if no other cases
    // match. Otherwise, jump to the exit label (skip the whole swtich).
    if(default_case_index != -1) {
        printf("\tjmp\tL%03d\n", labels[default_case_index]);
    } else {
        printf("\tjmp\tL%03d\n", sw.exit_label);
    }

    for(int i = 0; i < cases.size() && !break_encountered ; i++) { 
        printf("L%03d:\n", labels[i]);

        auto opr = std::get<oprNodeType>(cases[i].self->un);

        if(opr.oper == DEFAULT) {
            ex(opr.op[0]);
        } else {
            ex(opr.op[1]);
        }
    }

    printf("L%03d:\n", sw.exit_label);

    return 0;
}

// TODO: convert this to use `Value`
// TODO: convert this to use quadruples (registers) instead of a stack.
float ex(nodeType *p) {
    int lbl1, lbl2;

    if (!p) return 0;
    switch(p->type) {
        case typeCon:{

            auto con = p->un;
            std::visit(
                Visitor {
                    [](float fValue)       { printf("\tpush\t%f\n", fValue);       return fValue;},
                    [](auto arg) {}
                },
                con
            );
            return 0;
        } break;
        case typeId: {
             auto identifierNode = std::get<idNodeType>(p->un);
             printf("\tpush\t%s\n", identifierNode.id.c_str()); 
             return /* sym[identifierNode.id]; */ 0;
        } break;
    case typeCase: {
        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
        exit(EXIT_FAILURE);
    }
    case typeSwitch: compile_switch(p); break;
    case typeBreak: {
         auto br = std::get<breakNodeType>(p->un);
         auto sw = std::get<switchNodeType>(br.parent_switch->un);
         int exit_label = sw.exit_label;
         printf("\tjmp\tL%03d\n", exit_label);
    } break;
    case typeOpr:
        auto opr = std::get<oprNodeType>(p->un);
        switch(opr.oper) {
        case WHILE:
            // op[0] is the condition
            // op[1] is the statement list
            
            // label1:
            printf("L%03d:\n", lbl1 = lbl++);

            //      check condition
            ex(opr.op[0]);
            
            //      if condition doesn't hold, jump to label2
            printf("\tjz\tL%03d\n", lbl2 = lbl++);

            //      code ...
            ex(opr.op[1]);

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
            ex(opr.op[0]);

            //      condition check 
            ex(opr.op[1]);

            //      jump to label if condition holds
            printf("\tjz\tL%03d\n", lbl1);

            break;
        case IF:
            ex(opr.op[0]);
            switch(opr.op.size()) {
                /* if op[0] { op[1] } else { op[2] } */
                case 3:
                    printf("\tjz\tL%03d\n", lbl1 = lbl++);
                    ex(opr.op[1]);
                    printf("\tjmp\tL%03d\n", lbl2 = lbl++);
                    printf("L%03d:\n", lbl1);
                    ex(opr.op[2]);
                    printf("L%03d:\n", lbl2);
                    break;

                /* if op[0] { op[1] } */
                case 2:
                    /* if */
                    printf("\tjz\tL%03d\n", lbl1 = lbl++);
                    ex(opr.op[1]);
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
            

            ex(opr.op[0]);

            printf("L%03d:\n", lbl1 = lbl++);

            ex(opr.op[1]);

            printf("\tjnz\tL%03d\n", lbl2 = lbl++);

            ex(opr.op[3]);
            
            ex(opr.op[2]);

            printf("\tjmp\tL%03d\n", lbl1);

            printf("L%03d:\n", lbl2);
            break;
        case DEFAULT:   
        case CASE:     
            ex(opr.op[0]); break;
        case PRINT:     
            ex(opr.op[0]);
            printf("\tprint\n");
            break;
        case '=':       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case PA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case SA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case MA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case DA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case RA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case LSA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case RSA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case ANDA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case EORA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case IORA:       
            ex(opr.op[1]);
            printf("\tpop\t%s\n", std::get<idNodeType>(opr.op[0]->un).id.c_str());
            break;
        case PP:    
            ex(opr.op[0]);
            printf("\tinc1pre\n");
            break;
        case MM:    
            ex(opr.op[0]);
            printf("\tdec1pre\n");
            break;
        case UPLUS:    
            ex(opr.op[0]);
            printf("\tpos\n");
            break;
        case UMINUS:    
            ex(opr.op[0]);
            printf("\tneg\n");
            break;
        case '!':    
            ex(opr.op[0]);
            printf("\tnot\n");
            break;
        case '~':    
            ex(opr.op[0]);
            printf("\tbitNOT\n");
            break;
       default:
            if(!opr.op.empty()) ex(opr.op[0]);
            if(opr.op.size() > 1) ex(opr.op[1]);

            switch(opr.oper) {
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
