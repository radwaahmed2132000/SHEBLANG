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
#include "nodes.h"
#include "parser.h"

static int lbl;

// TODO: Use ControlFlow to improve compiler code (remove STACK_HACK, etc...).

// Meant to prevent `push 0` when an `ex()` call returns 0.
#define STACK_HACK(var)\
    if(!(std::holds_alternative<std::string>(var) && var.toString() == "0")) { \
        printf("\tpush %s\n", var.toString().c_str()); \
    }

#define PUSH_BIN_OPR_PARAMS                                                    \
    auto lhs = ex(bop.lOperand).val;                                           \
    STACK_HACK(lhs);                                                           \
    convertPushedVar(bop.lOperand);                                            \
    auto rhs = ex(bop.rOperand).val;                                           \
    STACK_HACK(rhs);                                                           \
    convertPushedVar(bop.rOperand);

#define UN_OP(OP_CODE)                                                         \
    auto var = ex(uop.operand).val;                                            \
    printf("\tpush %s\n", var.toString().c_str());                             \
    convertPushedVar(uop.operand);                                             \
    OP_CODE                                                                    \
    if (!var.isLiteral()) {                                                    \
        printf("\tpop %s\n", var.toString().c_str());                          \
    }

#define CONDITION_CHECK(str)                                                   \
    printf("\tpush %s\n", str.c_str());                                        \
    printf("\tpush true\n");                                                   \
    printf("\tcompEQ\n");

// To handle 
// while(true), while(x) and so on ...
#define BOOL_COMP_HACK_LOOPS_LOOPS(var)                                        \
    if (auto *con = std::get_if<conNodeType>(var.condition); con) {            \
        CONDITION_CHECK(con->toString())                                       \
    } else if (auto *id = std::get_if<idNodeType>(var.condition); id) {        \
        CONDITION_CHECK(id->id)                                                \
    } else {                                                                   \
        ex(var.condition);                                                     \
        compileBoolExprs(var.condition);                                       \
    }


// Checks if the given unary or binary expression is not boolean. If not, then we need to 
// push true and compare with that.
// Example:
//      int a = 1;
//      if(a) { ... }
//
// This should compile down to something like:
//      push a 
//      pop a
//      push a
//      intToBool
//      push true   // <-
//      compEQ      // <- Handled by this function
//      jnz ...
//
// While something like:
//      int a = 2; int b = 1;
//      if(a > b) { ... }
//
// Compiles down to something like this:
//      push 2
//      pop a
//      push 1
//      pop b
//      push a  // <- 
//      push b  // <- Don't require intervention by this function!
//      compGT  // <-
//      jnz ...
void compileBoolExprs(Node* ptr) {
    using enum BinOper; 
    using enum UnOper;

    static std::unordered_set<BinOper> boolBinOpers = { Equal, NotEqual, GreaterEqual,GreaterThan, LessEqual, LessThan, And, Or};
    
    auto *uop = std::get_if<UnOp>(ptr); 
    auto notUnaryNeg = uop != nullptr && uop->op != BoolNeg;

    auto *bop = std::get_if<BinOp>(ptr); 
    auto notBinaryBool = bop != nullptr && boolBinOpers.count(bop->op) == 0;

    if (notBinaryBool || notUnaryNeg) {
        printf("\tpush true\n");
        printf("\tcompEQ\n");
    }
}

ControlFlow ex(Node *p);

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
int compile_switch(switchNodeType &sw) {

    std::optional<int> matching_case_index = {};
    std::optional<int> default_case_index = {};
    
    Value var_name = ex(sw.var);

    // Check cli.cpp for an explanation of why we're collecting and reversing.
    auto cases = sw.caseListTail->as<caseNodeType>().toVec();
    std::vector<int> labels; 
    labels.reserve(cases.size());
    for(int i = 0; i < cases.size(); i++) { labels.push_back(lbl++); }

    sw.exitLabel = lbl++;

    for (int i = 0; i < cases.size(); i++) {
        if(cases[i]->isDefault()) { default_case_index = i; continue; }

        auto labelRet = ex(cases[i]->labelExpr);

        printf("\tpush\t%s\n", var_name.toString().c_str()); // variable value
        printf("\tpush\t%s\n", labelRet.val.toString().c_str()); // label value
        printf("\tcompEQ\n");
        printf("\tje\tL%03d\n", labels[i]);
    }

    // If there exists a default case, jump to it if no other cases
    // match. Otherwise, jump to the exit label (skip the whole swtich).
    if (default_case_index.has_value()) {
        ex(cases[default_case_index.value()]->caseBody);
    }
    printf("\tjmp\tL%03d\n", sw.exitLabel);

    // Emit the code that corresponds to each case.
    for(int i = 0; i < cases.size(); i++) {
        if(cases[i]->isDefault()) { default_case_index = i; continue; }

        printf("L%03d:\n", labels[i]);
        ex(cases[i]->caseBody);
        printf("\tjmp\tL%03d\n", sw.exitLabel);
    }

    printf("L%03d:\n", sw.exitLabel);

    return 0;
}

void convertPushedVar(Node* nodePtr) {
    auto Node = std::visit(
        Visitor{
            [&](idNodeType &id) { return getSymEntry(id.id, nodePtr->currentScope).type; },
            [](conNodeType &con) { return con.getType(); },
            [](auto _default) { return std::string(""); }},
        *nodePtr
    );

    auto convType = nodePtr->conversionType;

    if(!Node.empty() && !convType.empty() && Node != convType) {
        auto convTypeCamelCase = convType;
        convTypeCamelCase[0] = std::toupper(convTypeCamelCase[0]);
        printf("\t%sTo%s\n", Node.c_str(), convTypeCamelCase.c_str());
    }
}

struct compiler_visitor {
    Node* p;

    ControlFlow operator()(idNodeType &identifier) {
        return Value(identifier.id);
    }

    ControlFlow operator()(conNodeType& c) {
        return Value(c);
    }

    ControlFlow operator()(VarDecl& varDecl) {
        return Value(ex(varDecl.var_name).val.toString());
    }

    ControlFlow operator()(VarDefn& vd) {
        auto initRet = ex(vd.initExpr).val;

        STACK_HACK(initRet);
        convertPushedVar(vd.initExpr);

        auto varNameRet = ex(vd.decl->var_name).val;
        printf("\tpop %s\n", varNameRet.toString().c_str());
        return Value(0);
    }

    ControlFlow operator()(caseNodeType& c) {
        auto caseLabelValue = ex(c.labelExpr);
        return Value(0);
    }

    ControlFlow operator()(enumUseNode& eu) const {
        auto members =  getEnumEntry(eu.enumName, p->currentScope).enumMembers;
        int memberValue = std::distance(members.begin(), std::find(members.begin(), members.end(), eu.memberName));
        return Value(memberValue);
    }

    ControlFlow operator()(StatementList& sl) {
        for (const auto &s : sl.statements) {
            auto ret = ex(s);
            if (ret.type == ControlFlow::Type::Return) break;
        }

        return Value(0);
    }

    ControlFlow operator()(switchNodeType &sw) {
        compile_switch(sw);
        return Value(0);
    }

    ControlFlow operator()(breakNodeType &br) {
        int exit_label = std::visit(
            Visitor {
                [](whileNodeType& w) { return w.exit_label; },
                [](forNodeType& f) { return f.exit_label; },
                [](doWhileNodeType& dw) { return dw.exit_label; },
                [](auto _default) { return 0; }
            },
            *br.parent_switch
        );
        
        printf("\tjmp\tL%03d\n", exit_label);

        return Value(0);
    }

    ControlFlow operator()(FunctionDefn &fn) {
        auto name =  fn.name->as<idNodeType>().id;

        printf("%s:\n", name.c_str());
        for(const auto& param: fn.getParameters()) {
            // See the fixme in this same overload (functionNodeType) setup_scopes.cpp
            // for why we continue.
            if(param->var_name == nullptr && param->type == nullptr) continue; 

            printf("\tpop %s\n", param->getName().c_str());
        }
        ex(fn.statements);

        printf("\tret\n");
        
        return Value(0);
    }

    ControlFlow operator()(FunctionCall& fc) {
        // ASSUMPTION: parameters are either conNodeType or idNodeType
        // Not sure about other function calls..
        
        // Push the parameters in their reverse order
        // So popping on the other side can be done in order.
        std::vector<ExprListNode*> paramsReversed(fc.parameterExpressions.size());
        std::reverse_copy(fc.parameterExpressions.begin(), fc.parameterExpressions.end(), paramsReversed.begin());

        for(const auto& param: paramsReversed) {
            if(param->exprCode == nullptr) continue;

            auto exprRet = ex(param->exprCode).val;
            STACK_HACK(exprRet);
        }
        printf("\tcall %s\n", fc.functionName.c_str());

        // HACK
        return Value(std::string("0"));
    }

    ControlFlow operator()(doWhileNodeType &dw) {
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

    ControlFlow operator()(whileNodeType &w) {
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

    ControlFlow operator()(forNodeType &f) {
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

    ControlFlow operator()(BinOp bop) const {
        using enum BinOper;

        if(bop.op == Assign) {
            std::string lhs = ex(bop.lOperand).val.toString();

            auto rhs = ex(bop.rOperand).val;
            STACK_HACK(rhs);

            printf("\tpop %s\n", lhs.c_str());
            return Value(lhs);
        }

        PUSH_BIN_OPR_PARAMS
        switch (bop.op) {
        
        case Assign:     /* Already handled above */ break;
        case BitAnd:       { printf("\tbitAND\n"); } break;
        case BitOr:        { printf("\tbitOR\n");  } break;
        case BitXor:       { printf("\tbitXOR\n"); } break;
        case LShift:       { printf("\tLshift\n"); } break;
        case RShift:       { printf("\tRshift\n"); } break;
        case Add:          { printf("\tadd\n");    } break;
        case Sub:          { printf("\tsub\n");    } break;
        case Mul:          { printf("\tmul\n");    } break;
        case Div:          { printf("\tdiv\n");    } break;
        case Mod:          { printf("\tmod\n");    } break;
        case And:          { printf("\tand\n");    } break;
        case Or:           { printf("\tor\n");     } break;
        case LessThan:     { printf("\tcompLT\n"); } break;
        case GreaterThan:  { printf("\tcompGT\n"); } break;
        case GreaterEqual: { printf("\tcompGE\n"); } break;
        case LessEqual:    { printf("\tcompLE\n"); } break;
        case NotEqual:     { printf("\tcompNE\n"); } break;
        case Equal:        { printf("\tcompEQ\n"); } break;

        }

        return Value(std::string("0"));
    }

    ControlFlow operator()(UnOp uop) const {
        using enum UnOper;

        switch(uop.op) {

        case Print: {
            auto ret = ex(uop.operand).val;
            STACK_HACK(ret);
            printf("\tprint \n");
        } break;

        case Return: {
            if(uop.operand != nullptr) {
                auto ret = ex(uop.operand).val;
                STACK_HACK(ret);
            } 

            // TODO: ControlFlow here doesn't really need to return a value,
            // just needs to signal to the caller that it reached a return statement.
            return ControlFlow::Return(Value(0));
        }

        case Increment: { UN_OP(printf("\tinc\n");) } break;
        case Decrement: { UN_OP(printf("\tdec\n");) } break;
        case Plus:      { UN_OP(printf("\tpos\n");) } break;
        case Minus:     { UN_OP(printf("\tneg\n");) } break;
        case BoolNeg:   { UN_OP(printf("\tnot\n");) } break;
        case BitToggle: { UN_OP(printf("\tbitNOT\n");) } break;

        }

        return Value(std::string("0"));
    }

    ControlFlow operator()(IfNode ifNode) const {
        int lbl1;
        int lbl2;
        auto *cond = ifNode.condition;

        if (const auto *con = std::get_if<conNodeType>(cond); con) {
            CONDITION_CHECK(con->toString())
        } else if (const auto *id = std::get_if<idNodeType>(cond); id) {
            CONDITION_CHECK(id->id)
        } else {
            ex(cond);
        }

        if(ifNode.ifCode != nullptr && ifNode.elseCode != nullptr) {
            printf("\tjnz\tL%03d\n", lbl1 = lbl++);
            ex(ifNode.ifCode);
            printf("\tjmp\tL%03d\n", lbl2 = lbl++);
            printf("L%03d:\n", lbl1);
            ex(ifNode.elseCode);
            printf("L%03d:\n", lbl2);
        } else if (ifNode.elseCode == nullptr) {
            printf("\tjnz\tL%03d\n", lbl1 = lbl++);
            ex(ifNode.ifCode);
            printf("L%03d:\n", lbl1);
        }

        return Value(std::string("0"));
    }

    template <typename T> ControlFlow operator()(T const & /*UNUSED*/) const {  return Value(0);}
    
};

ControlFlow ex(Node *p) {
    if (p == nullptr) return Value(0);
    return std::visit(compiler_visitor{p}, *p);
}
