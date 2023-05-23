#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <variant>
#include <optional>

#include "cl.h"
#include "y.tab.h"

static int lbl;
#define PUSH_BIN_OPR_PARAMS              \
    auto lhs = ex(opr.op[0]).toString(); \
    auto rhs = ex(opr.op[1]).toString(); \
    printf("\tpush %s\n", lhs.c_str());      \
    printf("\tpush %s\n", rhs.c_str());

#define UN_OP(OP_CODE)                  \
    auto var = ex(opr.op[0]).toString();\
    printf("\tpush %s\n", var.c_str());     \
    OP_CODE                             \
    printf("\tpop %s\n", var.c_str());      



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
// Check cli.cpp for the reference interpreter implementation of this function.
// The compiler function should follow a similar structure.
// TODO: Update this to work with registers.
// TODO: Make sure the output is correct.
int compile_switch(switchNodeType &sw) {

    std::optional<int> matching_case_index = {};
    std::optional<int> default_case_index = {};
    
    Value var_name = ex(sw.var);

    // Check cli.cpp for an explanation of why we're collecting and reversing.
    nodeType* head = sw.case_list_head;

    auto cases = std::get<caseNodeType>(head->un).toVec();
    std::vector<int> labels; 
    labels.reserve(cases.size());
    for(int i = 0; i < cases.size(); i++) { labels.push_back(lbl++); }

    sw.exit_label = lbl++;

    for (int i = 0; i < cases.size(); i++) {
        if(cases[i]->isDefault()) { default_case_index = i; continue; }

        printf("\tpush\t%s\n", var_name.toString().c_str()); // variable value
        printf("\tpush\t%s\n", ex(cases[i]->labelExpr).toString().c_str()); // label value
        printf("\tcompEQ\n");
        printf("\tje\tL%03d\n", labels[i]);
    }

    // If there exists a default case, jump to it if no other cases
    // match. Otherwise, jump to the exit label (skip the whole swtich).
    if (default_case_index.has_value()) {
        ex(cases[default_case_index.value()]->caseBody);
        printf("\tjmp\tL%03d\n", sw.exit_label);
    }

    // Emit the code that corresponds to each case.
    for(int i = 0; i < cases.size(); i++) {
        if(cases[i]->isDefault()) { default_case_index = i; continue; }

        printf("L%03d:\n", labels[i]);
        ex(cases[i]->caseBody);
        printf("\tjmp\tL%03d\n", sw.exit_label);
    }

    printf("L%03d:\n", sw.exit_label);

    return 0;
}

struct compiler_visitor {

    Value operator()(idNodeType &identifier) {
        return Value(identifier.id);
    }

    Value operator()(conNodeType& c) {
        return Value(c);
    }

    Value operator()(VarDecl& varDecl) {
        return Value(ex(varDecl.var_name).toString());
    }

    Value operator()(VarDefn& vd) {
        printf("\tpush %s\n", ex(vd.initExpr).toString().c_str());
        printf("\tpop %s\n", ex(vd.decl->var_name).toString().c_str());
        return Value(0);
    }

    Value operator()(caseNodeType& c) {
        auto caseLabelValue = ex(c.labelExpr);
        return Value(0);
    }

    Value operator()(StatementList& sl) {
        for(const auto& s: sl.toVec()) {
            ex(s->statementCode);
        }

        return Value(0);
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

    Value operator()(functionNodeType &fn) {
        auto name = std::get<idNodeType>(fn.name->un).id;
        printf("%s:\n", name.c_str());
        ex(fn.statements);

        return Value(0);
    }

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

        case PRINT: {
            // HACK
            auto ret = ex(opr.op[0]);
            if(std::holds_alternative<std::string>(ret) && ret.toString() == "0") {
                printf("\tprint \n");
            } else {
                printf("\tpush %s\n", ret.toString().c_str());
                printf("\tprint \n");
            }
        } break;
        case '=': {
            std::string lhs = ex(opr.op[0]).toString();
            ex(opr.op[1]);

            printf("\tpop %s\n", lhs.c_str());
            return Value(lhs);
        } break;

        case PP: { UN_OP(printf("\tinc1pre\n");) } break;
        case MM: { UN_OP(printf("\tdec1pre\n");) } break;
        case UPLUS: { UN_OP(printf("\tpos\n");) } break;
        case UMINUS: { UN_OP(printf("\tneg\n");) } break;
        case '!': { UN_OP(printf("\tnot\n");) } break;
        case '~': { UN_OP(printf("\tbitNOT\n");) } break;

        case '&': {
            PUSH_BIN_OPR_PARAMS
            printf("\tbitAND\n");
        } break;
        case '|': {
            PUSH_BIN_OPR_PARAMS
            printf("\tbitOR\n");
        } break;
        case '^': {
            PUSH_BIN_OPR_PARAMS
            printf("\tbitXOR\n");
            break;
        }
        case LS: {
            PUSH_BIN_OPR_PARAMS
            printf("\tLshift\n");
        } break;
        case RS: {
            PUSH_BIN_OPR_PARAMS
            printf("\tRshift\n");
        } break;
        case '+': {
            PUSH_BIN_OPR_PARAMS
            printf("\tadd\n");
        } break;
        case '-': {
            PUSH_BIN_OPR_PARAMS
            printf("\tsub\n");
        } break;
        case '*': {
            PUSH_BIN_OPR_PARAMS
            printf("\tmul\n");
        } break;
        case '/': {
            PUSH_BIN_OPR_PARAMS
            printf("\tdiv\n");
        } break;
        case '%': {
            PUSH_BIN_OPR_PARAMS
            printf("\tmod\n");
        } break;
        case AND: {
            PUSH_BIN_OPR_PARAMS
            printf("\tand\n");
        } break;
        case OR: {
            PUSH_BIN_OPR_PARAMS
            printf("\tor\n");
        } break;
        case '<': {
            PUSH_BIN_OPR_PARAMS
            printf("\tcompLT\n");
        } break;
        case '>': {
            PUSH_BIN_OPR_PARAMS
            printf("\tcompGT\n");
        } break;
        case GE: {
            PUSH_BIN_OPR_PARAMS
            printf("\tcompGE\n");
        } break;
        case LE: {
            PUSH_BIN_OPR_PARAMS
            printf("\tcompLE\n");
        } break;
        case NE: {
            PUSH_BIN_OPR_PARAMS
            printf("\tcompNE\n");
        } break;
        case EQ: {
            PUSH_BIN_OPR_PARAMS
            printf("\tcompEQ\n");
        } break;
        default:
            return Value(0);
        }

        return Value(std::string("0"));
    }

    template <typename T> Value operator()(T const & /*UNUSED*/) const {  return Value(0);}
    
};

Value ex(nodeType *p) {
    if (p == nullptr) return Value(0);
    return std::visit(compiler_visitor(), p->un);
}
