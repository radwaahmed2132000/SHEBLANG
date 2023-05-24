#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cstddef>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <variant>
#include <optional>
#include <vector>
#include <unordered_set>

#include "cl.h"
#include "y.tab.h"

// Meant to prevent `push 0` when an `ex()` call returns 0.
#define STACK_HACK(var)\
    if(!(std::holds_alternative<std::string>(var) && var.toString() == "0")) { \
        printf("\tpush %s\n", var.toString().c_str()); \
    }  

static int lbl;
#define PUSH_BIN_OPR_PARAMS                                                    \
    auto lhs = ex(opr.op[0]);                                                  \
    STACK_HACK(lhs);                                                           \
    convPushedVar(opr.op[0]);                                                  \
    auto rhs = ex(opr.op[1]);                                                  \
    STACK_HACK(rhs);                                                           \
    convPushedVar(opr.op[1]);                                                  

#define UN_OP(OP_CODE)                                                         \
    auto var = ex(opr.op[0]);                                                  \
    printf("\tpush %s\n", var.toString().c_str());                             \
    convPushedVar(opr.op[0]);                                                  \
    OP_CODE                                                                    \
    if (!var.isLiteral()) {                                                    \
        printf("\tpop %s\n", var.toString().c_str());                          \
    }

#define CONDITION_CHECK(str)                                                   \
    printf("\tpush %s\n", str.c_str());                                        \
    printf("\tpush true\n");                                                   \
    printf("\tcompEQ\n");

void compileBoolExprs(nodeType* ptr) {
    static std::unordered_set<int> boolOpers = { EQ, NE, GE, '>', LE, '<', AND, OR, '!' };
    if (auto *opr = std::get_if<oprNodeType>(&ptr->un);
        opr != nullptr && boolOpers.count(opr->oper) == 0) {
            printf("\tpush true\n");
            printf("\tcompEQ\n");
    }
}

// To handle 
// while(true), while(x) and so on ...
#define BOOL_COMP_HACK_LOOPS_LOOPS(var)                                                    \
    if (auto *con = std::get_if<conNodeType>(&var.condition->un); con) {       \
        CONDITION_CHECK(con->toString())                                       \
    } else if (auto *id = std::get_if<idNodeType>(&var.condition->un); id) {   \
        CONDITION_CHECK(id->id)                                                \
    } else {                                                                   \
        ex(var.condition);                                                     \
        compileBoolExprs(var.condition);\
    }



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
    }
    printf("\tjmp\tL%03d\n", sw.exit_label);

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

void convPushedVar(nodeType* nodePtr) {
    auto nodeType = std::visit(
        Visitor {
            [&](idNodeType& id) {    return varSymTableEntry(id.id, nodePtr->currentScope).type; },
            [](conNodeType& con) { return con.getType(); },
            [](auto _default) { return std::string(""); }
        },
        nodePtr->un
    );

    auto convType = nodePtr->conversionType;

    if(!nodeType.empty() && !convType.empty() && nodeType != convType) {
        auto convTypeCamelCase = convType;
        convTypeCamelCase[0] = std::toupper(convTypeCamelCase[0]);
        printf("\t%sTo%s\n", nodeType.c_str(), convTypeCamelCase.c_str());
    }
}

struct compiler_visitor {
    nodeType* p;

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
        auto ret = ex(vd.initExpr);
        STACK_HACK(ret);
        convPushedVar(vd.initExpr);
        printf("\tpop %s\n", ex(vd.decl->var_name).toString().c_str());
        return Value(0);
    }

    Value operator()(caseNodeType& c) {
        auto caseLabelValue = ex(c.labelExpr);
        return Value(0);
    }

    Value operator()(enumUseNode& eu) {
        auto members =  enumSymTableEntry(eu.enumName, p->currentScope).enumMembers;
        int memberValue = std::distance(members.begin(), std::find(members.begin(), members.end(), eu.memberName));
        return Value(memberValue);
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
        int exit_label = std::visit(
            Visitor {
                [](whileNodeType& w) { return w.exit_label; },
                [](forNodeType& f) { return f.exit_label; },
                [](doWhileNodeType& dw) { return dw.exit_label; },
                [](auto _default) { return 0; }
            },
            br.parent_switch->un
        );
        
        printf("\tjmp\tL%03d\n", exit_label);

        return Value(0);
    }

    Value operator()(functionNodeType &fn) {
        auto name = std::get<idNodeType>(fn.name->un).id;

        printf("%s:\n", name.c_str());
        for(const auto& param: fn.parametersTail->toVec()) {
            // See the fixme in this same overload (functionNodeType) setup_scopes.cpp
            // for why we continue.
            if(param->var_name == nullptr && param->type == nullptr) continue; 

            printf("\tpop %s\n", param->getName().c_str());
        }
        ex(fn.statements);

        printf("\n");
        
        return Value(0);
    }

    Value operator()(FunctionCall& fc) {
        // ASSUMPTION: parameters are either conNodeType or idNodeType
        // Not sure about other function calls..
        
        // Push the parameters in their reverse order
        // So popping on the other side can be done in order.
        std::vector<ExprListNode*> paramsReversed(fc.parameterExpressions.size());
        std::reverse_copy(fc.parameterExpressions.begin(), fc.parameterExpressions.end(), paramsReversed.begin());

        for(const auto& param: paramsReversed) {
            if(param->exprCode == nullptr) continue;

            auto exprResult = ex(param->exprCode);
            STACK_HACK(exprResult);
        }
        printf("\tcall %s\n", fc.functionName.c_str());

        // HACK
        return Value(std::string("0"));
    }

    Value operator()(doWhileNodeType &dw) {
        int loopLabel = lbl++;
        int exitLabel = lbl++;
        dw.exit_label = exitLabel;

        // label:
        printf("L%03d:\n", loopLabel);

        //      code....
        ex(dw.loop_body);

        BOOL_COMP_HACK_LOOPS_LOOPS(dw) 

        //      jump to label if condition holds
        printf("\tjz\tL%03d\n", loopLabel);

        // exit label
        printf("L%03d:\n", exitLabel);

        return Value(0);
    }

    Value operator()(whileNodeType &w) {
        int loopLabel = lbl++;
        int exitLabel = lbl++;

        w.exit_label = exitLabel;

        // label1:
        printf("L%03d:\n", loopLabel);

        BOOL_COMP_HACK_LOOPS_LOOPS(w)

        //      if condition doesn't hold, jump to label2
        printf("\tjnz\tL%03d\n", exitLabel);

        //      code ...
        ex(w.loop_body);

        //      jump to label1
        printf("\tjmp\tL%03d\n", loopLabel);

        //  label2:
        printf("L%03d:\n", exitLabel);

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

        int loopLabel;
        int exitLabel;
        f.exit_label = exitLabel;

        ex(f.init_statement);

        printf("L%03d:\n", loopLabel = lbl++);

        ex(f.loop_condition);

        printf("\tjnz\tL%03d\n", exitLabel = lbl++);

        ex(f.loop_body);

        ex(f.post_loop_statement);

        printf("\tjmp\tL%03d\n", loopLabel);

        printf("L%03d:\n", exitLabel);
    
        return Value(0);
    }

    Value operator()(oprNodeType &opr) {
        int lbl1;
        int lbl2;

        switch (opr.oper) {
        case IF: {
            auto* cond = opr.op[0];

            if (const auto *con = std::get_if<conNodeType>(&cond->un); con) {     
                CONDITION_CHECK(con->toString())                                     
            } else if (const auto *id = std::get_if<idNodeType>(&cond->un); id) { 
                CONDITION_CHECK(id->id)                                              
            } else {                                                                 
                ex(cond);                                                   
            }


            switch (opr.op.size()) {
            /* if op[0] { op[1] } else { op[2] } */
            case 3:
                printf("\tjnz\tL%03d\n", lbl1 = lbl++);
                ex(opr.op[1]);
                printf("\tjmp\tL%03d\n", lbl2 = lbl++);
                printf("L%03d:\n", lbl1);
                ex(opr.op[2]);
                printf("L%03d:\n", lbl2);
                break;

            /* if op[0] { op[1] } */
            case 2:
                /* if */
                printf("\tjnz\tL%03d\n", lbl1 = lbl++);
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
            STACK_HACK(ret);
            printf("\tprint \n");
        } break;
        case '=': {
            std::string lhs = ex(opr.op[0]).toString();
            auto rhs = ex(opr.op[1]);
            STACK_HACK(rhs);
            printf("\tpop %s\n", lhs.c_str());
            return Value(lhs);
        } break;

        case PP: { UN_OP(printf("\tinc\n");) } break;
        case MM: { UN_OP(printf("\tdec\n");) } break;
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
            convPushedVar(p);
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
        case RETURN: {
            if(!opr.op.empty()) {
                auto ret = ex(opr.op[0]);
                STACK_HACK(ret);
            } 
            printf("\tret\n");
        }
        default:
            return Value(0);
        }

        return Value(std::string("0"));
    }

    template <typename T> Value operator()(T const & /*UNUSED*/) const {  return Value(0);}
    
};

Value ex(nodeType *p) {
    if (p == nullptr) return Value(0);
    return std::visit(compiler_visitor{p}, p->un);
}
