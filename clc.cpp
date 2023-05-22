#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <variant>

#include "cl.h"
#include "y.tab.h"

static int lbl;

Value ex(nodeType *p);

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
int compile_switch(switchNodeType &sw) {

    int matching_case_index = -1;
    int default_case_index = -1;
    bool break_encountered = false;

    std::vector<caseNodeType> cases;
    std::vector<int> labels;

    // Check cli.cpp for an explanation of why we're collecting and reversing.
    nodeType *head = sw.case_list_head;
    do {
        auto cs = std::get<caseNodeType>(head->un);

        cases.push_back(cs);
        labels.push_back(lbl++);

        head = cs.prev;
    } while (head != NULL);
    std::reverse(cases.begin(), cases.end());
    std::reverse(labels.begin(), labels.end());

    sw.exit_label = lbl++;

    for (int i = 0; i < cases.size() && !break_encountered; i++) {
        auto opr = std::get<oprNodeType>(cases[i].self->un);
        if (opr.oper == DEFAULT) {
            default_case_index = i;
        } else {
            auto case_identifier = std::get<idNodeType>(sw.var->un);
            ex(cases[i].self); // Label value
            printf("\tpush\t%s\n",
                   case_identifier.id.c_str()); // variable value
            printf("\tcompEQ\n");
            printf("\tje\tL%03d\n", labels[i]);
        }
    }

    // If there exists a default case, jump to it if no other cases
    // match. Otherwise, jump to the exit label (skip the whole swtich).
    if (default_case_index != -1) {
        printf("\tjmp\tL%03d\n", labels[default_case_index]);
    } else {
        printf("\tjmp\tL%03d\n", sw.exit_label);
    }

    for (int i = 0; i < cases.size() && !break_encountered; i++) {
        printf("L%03d:\n", labels[i]);

        auto opr = std::get<oprNodeType>(cases[i].self->un);

        if (opr.oper == DEFAULT) {
            ex(opr.op[0]);
        } else {
            ex(opr.op[1]);
        }
    }

    printf("L%03d:\n", sw.exit_label);

    return 0;
}

struct compiler_visitor {
    Value operator()(conNodeType &con) { return con; }

    Value operator()(idNodeType &identifier) {
        return Value('[' + identifier.id + ']');
    }

    Value operator()(VarDecl& varDecl) {
        return Value(ex(varDecl.var_name).toString());
    }

    Value operator()(switchNodeType &sw) {
        compile_switch(sw);
        return Value(0);
    }

    Value operator()(breakNodeType &br) {
        auto sw = std::get<switchNodeType>(br.parent_switch->un);
        int exit_label = sw.exit_label;
        printf("\tjmp\tL%03d\n", exit_label);

        return Value(0);
    }

    Value operator()(functionNodeType &fn) { return Value(0); }

    Value operator()(doWhileNodeType &dw) {
        int lbl1;

        // label:
        printf("L%03d:\n", lbl1 = lbl++);

        //      code....
        ex(dw.loop_body);

        //      condition check
        ex(dw.condition);

        //      jump to label if condition holds
        printf("\tjz\tL%03d\n", lbl1);

        return Value(0);
    }

    Value operator()(whileNodeType &w) {
        int lbl1;
        int lbl2;

        // label1:
        printf("L%03d:\n", lbl1 = lbl++);

        //      check condition
        ex(w.condition);

        //      if condition doesn't hold, jump to label2
        printf("\tjz\tL%03d\n", lbl2 = lbl++);

        //      code ...
        ex(w.loop_body);

        //      jump to label1
        printf("\tjmp\tL%03d\n", lbl1);

        //  label2:
        printf("L%03d:\n", lbl2);

        return Value(0);
    }

    Value operator()(forNodeType &f) {
        // Should go like this:
        //      loop initialization expression
        //  label1:
        //      check loop condition
        //      if it doesn't hold jump to label2
        //
        //      code...
        //
        //      loop post-iteration code
        //      jump to label1
        //  label2:

        int lbl1;
        int lbl2;

        ex(f.init_statement);

        printf("L%03d:\n", lbl1 = lbl++);

        ex(f.loop_condition);

        printf("\tjnz\tL%03d\n", lbl2 = lbl++);

        ex(f.loop_body);

        ex(f.post_loop_statement);

        printf("\tjmp\tL%03d\n", lbl1);

        printf("L%03d:\n", lbl2);
    
        return Value(0);
    }

    Value operator()(oprNodeType &opr) {
        int lbl1;
        int lbl2;

        switch (opr.oper) {
        case IF: {
            ex(opr.op[0]);
            switch (opr.op.size()) {
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
        } break;
        case DEFAULT:
        case CASE:
            ex(opr.op[0]);
            break;

        case PRINT:
            printf("\tprint %s\n", ex(opr.op[0]).toString().c_str());
            break;
        case '=': {
            std::string rhs = ex(opr.op[1]).toString();
            std::string lhs = ex(opr.op[0]).toString();

            printf("mov %s, %s\n", lhs.c_str(), rhs.c_str());
        } break;
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

        case '&':
            printf("\tbitAND\n");
            break;
        case '|':
            printf("\tbitOR\n");
            break;
        case '^':
            printf("\tbitXOR\n");
            break;
        case LS:
            printf("\tLshift\n");
            break;
        case RS:
            printf("\tRshift\n");
            break;
        case '+':
            printf("\tadd\n");
            break;
        case '-':
            printf("\tsub\n");
            break;
        case '*':
            printf("\tmul\n");
            break;
        case '/':
            printf("\tdiv\n");
            break;
        case '%':
            printf("\tmod\n");
            break;
        case AND:
            printf("\tand\n");
            break;
        case OR:
            printf("\tor\n");
            break;
        case '<':
            printf("\tcompLT\n");
            break;
        case '>':
            printf("\tcompGT\n");
            break;
        case GE:
            printf("\tcompGE\n");
            break;
        case LE:
            printf("\tcompLE\n");
            break;
        case NE:
            printf("\tcompNE\n");
            break;
        case EQ:
            printf("\tcompEQ\n");
            break;
        default:
            return Value(0);
        }
    
        return Value(0);
    }

    // the default case:
    template <typename T> Value operator()(T const & /*UNUSED*/) const {
        return Value(0);
    }
};

Value ex(nodeType *p) {
    if (p == nullptr) return Value(0);
    return std::visit(compiler_visitor(), p->un);
}
