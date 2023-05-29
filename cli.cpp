#include <cstddef>
#include <functional>
#include <iterator>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <variant>
#include <algorithm>
#include <optional>

#include "cl.h"
#include "nodes.h"
#include "result.h"
#include "parser.h"

#define BOP_CASE(case_value, oper) \
            case case_value: return ex(bop.lOperand) oper ex(bop.rOperand);
                             
#define BOOL_BOP_CASE(case_value, oper) \
            case case_value: return Value(ex(bop.lOperand) oper ex(bop.rOperand));
                             
Value postOp(UnOp &uop, nodeType *p, std::function<Value(Value&)> op) {
    auto nameStr = uop.operand->as<idNodeType>().id;
    auto &varEntry = getSymEntry(nameStr, p->currentScope);
    auto &varRef = varEntry.getRef();
    Value temp = varRef;
    varRef = op(varRef);
    return temp;
}

Value ex(nodeType* p);

Value assignToVar(BinOp& bop, nodeType* p) {
    using std::optional, std::string, std::make_optional, std::visit;
    using namespace std::string_literals;

    // Get the variable name based on the LHS's type.
    optional<string> varNameOpt = visit(
            Visitor {
                [](VarDecl& varDecl)              { return make_optional(varDecl.var_name->as<idNodeType>().id); },
                [](VarDefn& varDefn)              { return make_optional(varDefn.decl->var_name->as<idNodeType>().id); },
                [](idNodeType& idNode)            { return make_optional(idNode.id); },
                [](auto _default) -> optional<string> {  return std::nullopt; }
            } ,
            *bop.lOperand
        );

    if(!varNameOpt.has_value()) { std::cout << "Invalid assignment expression"; }
    auto varName = varNameOpt.value();

    // Get the symbol table entry of the scope containing the variable.
    auto& symTable = getSymEntry(varName, p->currentScope);

    // Assign the value in the scope table.
    auto ret = visit(
        Visitor{
            [&](VarDecl &varDecl) { return ex(bop.rOperand); },
            [&](VarDefn &varDefn) { return ex(varDefn.initExpr); },
            [&](idNodeType &idNode) { return ex(bop.rOperand); },
            [](auto _default) { return Value(0); } // Should never reach here
        },
        *bop.lOperand
    );

    symTable.setValue(ret);

    return ret;
}

Value evaluate_switch(switchNodeType& sw) {
    std::optional<int> matching_case_index = {};
    std::optional<int> default_case_index = {};
    Value var_value = ex(sw.var);

    assert(sw.caseListTail->is<caseNodeType>());

    nodeType* head = sw.caseListTail;
    auto cases =  head->as<caseNodeType>().toVec();

    bool foundCase = false;
    for(int i = 0; i < cases.size(); i++) {
        Value case_value = ex(cases[i]->labelExpr);

        if(cases[i]->isDefault()) {
            default_case_index = i;
        } else if (case_value == var_value || matching_case_index.has_value()) {
            // break once you find a matching case
            matching_case_index = i;
            ex(cases[i]->caseBody);
            break;
        }
    }

    if(!matching_case_index.has_value() &&  default_case_index.has_value()) {
        ex(cases[default_case_index.value()]->caseBody);
    }

    return Value(0);
}


struct ex_visitor {
    // pointer to the node of the current variant
    // since we need scope info from it.
    nodeType* p;

    Value operator()(conNodeType& con) {
        return con;
    }

    Value operator()(idNodeType& identifier) const {
        auto& varSymbolTableEntry = getSymEntry(identifier.id, p->currentScope);
        return varSymbolTableEntry.getValue();
    }

    Value operator()(VarDecl& vd) const {
        auto nameStr = vd.var_name->as<idNodeType>().id;
        auto& varSymbolTableEntry = getSymEntry(nameStr, p->currentScope);

        varSymbolTableEntry.setValue(ex(varSymbolTableEntry.initExpr));

        return Value(0);
    }

    Value operator()(VarDefn& vd) const {
        auto nameStr = vd.decl->var_name->as<idNodeType>().id;
        auto val = ex(vd.initExpr);

        auto& varSymbolTableEntry = getSymEntry(nameStr, p->currentScope);
        varSymbolTableEntry.setValue(val);

        return Value(0);
    }

    Value operator()(enumUseNode& eu) const {
        auto& e = getEnumEntry(eu.enumName, p->currentScope);
        auto memberIter = std::find(e.enumMembers.begin(), e.enumMembers.end(), eu.memberName);
        int enumMemberValue = std::distance(e.enumMembers.begin(), memberIter);
        return Value(enumMemberValue);
    }

    Value operator()(switchNodeType& sw) {
        return evaluate_switch(sw);
    }

    Value operator()(breakNodeType& br) {
        std::visit(
                Visitor {
                    [](whileNodeType& w) { w.break_encountered = true; },
                    [](forNodeType& f) { f.break_encountered = true; },
                    [](doWhileNodeType& dw) { dw.break_encountered = true; },
                    [](auto def) {}
                },
                *br.parent_switch
        );
        return Value(0);
    }

    Value operator()(FunctionCall& fc) const {
        // TODO: Function calls

        // For each argument, find the corresponding parameter in the symbol table,
        // then insert the argument value. 
        
        // We can dereference the iterator just fine because symantic analysis 
        // succeed unless we can reach the function in its proper scope.
        auto& fnRef = getFnEntry(fc.functionName, p->currentScope);
        auto fnParams = fnRef.parametersTail->toVec();
        auto* fnScope = fnRef.statements->currentScope;

        for(int i = 0; i < fnParams.size(); i++) {
            if(fnParams[i]->type == nullptr && fnParams[i]->var_name == nullptr) continue;
            fnScope->sym2[fnParams[i]->getName()].setValue(ex(fc.parameterExpressions[i]->exprCode));
        }

        return ex(fnRef.statements);
    }

    Value operator()(StatementList& sl) {
        for(const auto& statement: sl.statements) {
            auto ret = ex(statement);

            bool isBreak = statement->is<breakNodeType>();

            const auto *retOpr = statement->asPtr<UnOp>();
            bool isReturn = (retOpr != nullptr && retOpr->op == UnOper::Return);

            if(isBreak) { return Value(0); } 
            if(isReturn) { return ret; }
        }

        return Value(0);
    }

    Value operator()(doWhileNodeType& dw) {
        do {
            ex(dw.loop_body); 
        } while(ex(dw.condition) && !dw.break_encountered); 

        return Value(0);
    }

    Value operator()(whileNodeType& w) {
        while(ex(w.condition) && !w.break_encountered) 
            ex(w.loop_body); 

        return Value(0);
    }

    Value operator()(forNodeType& f) {
        for(ex(f.init_statement); ex(f.loop_condition) && !f.break_encountered; ex(f.post_loop_statement)) {
            ex(f.loop_body);
        }
        return Value(0);
    }

    Value operator()(BinOp& bop) const {
        using enum BinOper;

        switch(bop.op) {
            case Assign: return assignToVar(bop, p);

            BOP_CASE(Add,+)
            BOP_CASE(Sub,-)
            BOP_CASE(Mul, *)
            BOP_CASE(Div,/) 
            BOP_CASE(BitAnd,&) 
            BOP_CASE(BitOr,|) 
            BOP_CASE(BitXor,^)
            BOP_CASE(LShift,<<)
            BOP_CASE(RShift,>>)
            BOP_CASE(Mod,%) 
            BOOL_BOP_CASE(LessThan,<)
            BOOL_BOP_CASE(GreaterThan, >)
            BOOL_BOP_CASE(And, &&)
            BOOL_BOP_CASE(Or, ||)
            BOOL_BOP_CASE(GreaterEqual,>=)        
            BOOL_BOP_CASE(LessEqual,<=)        
            BOOL_BOP_CASE(NotEqual,!=)        
            BOOL_BOP_CASE(Equal,==)        
        }
    }

    Value operator()(UnOp& uop) {
        using enum UnOper;

        switch (uop.op) {
        case Print: {
            Value exprValue = ex(uop.operand);
            std::cout << exprValue.toString() << '\n';
            return Value(0);
        }

        case Return:
            return ex(uop.operand);

        case BoolNeg:
            return Value(!(ex(uop.operand)));

        case BitToggle:
            return ~(ex(uop.operand));

        case Plus:
            return +(ex(uop.operand));

        case Minus:
            return -(ex(uop.operand));

        case Increment:
            return postOp(uop, p, [](Value &val) { return val + Value(1); });

        case Decrement:
            return postOp(uop, p, [](Value &val) { return val - Value(1); });
        }
    }

    Value operator()(IfNode& ifNode) const {
        if (ex(ifNode.condition)) {
            ex(ifNode.ifCode);
        } else if (ifNode.elseCode != nullptr) {
            ex(ifNode.elseCode);
        }

        return Value(0);
    }

    // the default case:
    template<typename T> 
    Value operator()(T const & /*UNUSED*/ ) const { return Value(0);} 
};

Value ex(nodeType *p) {
    if (p == nullptr) return Value(0);
    return std::visit(ex_visitor{p}, *p);
}

