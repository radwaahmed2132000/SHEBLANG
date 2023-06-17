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
#include <vector>

#include "cl.h"
#include "nodes.h"
#include "result.h"
#include "parser.h"
#include "value.h"
#include "template_utils.h"

#define BOP_CASE(case_value, oper) \
            case case_value: return ex(bop.lOperand) oper ex(bop.rOperand);
                             
#define BOOL_BOP_CASE(case_value, oper) \
            case case_value: return Value(ex(bop.lOperand) oper ex(bop.rOperand));
                             
Value postOp(UnOp &uop, Node *p, std::function<Value(Value&)> op) {
    auto nameStr = uop.operand->as<idNodeType>().id;
    auto &varEntry = getVarEntry(nameStr, p->currentScope);
    auto &varRef = varEntry.getRef();
    Value temp = varRef;
    varRef = op(varRef);
    return temp;
}

ControlFlow assignToVar(BinOp& bop, Node* p) {
    using std::optional, std::string, std::make_optional, std::visit;
    using namespace std::string_literals;

    // Evaluate the RHS
    auto ret = ex(bop.rOperand);

    // If we're indexing into an array (`foo[0] = bar`) Get a reference to the
    // array in the symbol table, then update the specified index.
    if(auto *ai = bop.lOperand->asPtr<ArrayIndex>(); ai) {
        auto arrName = ai->arrayExpr->as<idNodeType>().id;
        auto arrIndex = std::get<int>(ex(ai->indexExpr).val());
        getVarEntry(arrName, p->currentScope).getRef().setIndex(arrIndex, ret);
    } else {

        // Normal variables just update the specified value in the symbol table.
        using OptRef = OptRef<SymbolTableEntry>;
        auto sym = [&](std::string &name) -> OptRef {
            return OptRef::Some(getVarEntry(name, p->currentScope));
        };

        // Get the variable name based on the LHS's type.
        auto varRefOpt = visit(
            Visitor{
                [&](VarDecl &varDecl)   { return sym(varDecl.varName->as<idNodeType>().id); },
                [&](VarDefn &varDefn)   { return sym(varDefn.decl->varName->as<idNodeType>().id); },
                [&](idNodeType &idNode) { return sym(idNode.id); },
                [](auto _default)       { return OptRef::None; }
            },
            *bop.lOperand
        );

        // TODO: Return something else to signify an error?
        if (!varRefOpt.has_value()) {
            std::cerr << "Invalid left hand side for assignment expression\n";
            return ControlFlow(0, ControlFlow::Type::Continue);
        }

        varRefOpt.value().get().setValue(ret.val());
    }

    return ret;
}

ControlFlow evaluate_switch(switchNodeType& sw) {
    std::optional<int> matching_case_index = {};
    std::optional<int> default_case_index = {};
    auto var_value = ex(sw.var);

    assert(sw.caseListTail->is<caseNodeType>());

    Node* head = sw.caseListTail;
    auto cases =  head->as<caseNodeType>().toVec();

    bool foundCase = false;
    for(int i = 0; i < cases.size(); i++) {
        auto case_value = ex(cases[i]->labelExpr);

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
    Node* p;

    ControlFlow operator()(conNodeType& con) { return ControlFlow(con); }

    ControlFlow operator()(ArrayLiteral& al) {
        std::vector<Value> arrayElements;
        arrayElements.reserve(al.expressions.size());
        for(const auto& expr: al.expressions) {
            arrayElements.push_back(ex(expr).val());
        }

        return Value(PrimitiveArray{arrayElements});
    }

    ControlFlow operator()(idNodeType& identifier) const {
        auto& varSymbolTableEntry = getVarEntry(identifier.id, p->currentScope);
        return varSymbolTableEntry.getValue();
    }

    ControlFlow operator()(VarDefn& vd) const {
        auto nameStr = vd.decl->varName->as<idNodeType>().id;
        auto val = ex(vd.initExpr);

        auto& varSymbolTableEntry = getVarEntry(nameStr, p->currentScope);
        varSymbolTableEntry.setValue(val);

        return Value(0);
    }

    ControlFlow operator()(enumUseNode& eu) const {
        auto& e = getEnumEntry(eu.enumName, p->currentScope);
        auto memberIter = std::find(e.enumMembers.begin(), e.enumMembers.end(), eu.memberName);
        int enumMemberValue = std::distance(e.enumMembers.begin(), memberIter);
        return Value(enumMemberValue);
    }

    ControlFlow operator()(switchNodeType& sw) {
        return evaluate_switch(sw);
    }

    ControlFlow operator()(breakNodeType& br) {
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

    ControlFlow operator()(FunctionCall& fc) const {
        // TODO: Function calls

        // For each argument, find the corresponding parameter in the symbol table,
        // then insert the argument value. 
        
        // We can dereference the iterator just fine because symantic analysis 
        // succeed unless we can reach the function in its proper scope.
        auto& fnRef = getFnEntry(fc.functionName, p->currentScope);
        auto fnParams = fnRef.getParameters();

        ScopeSymbolTables fnFrame(*fnRef.statements->currentScope);      // Make a copy of the original function scope after parameter binding.
        ScopeSymbolTables originalScope(fnFrame);

        for(int i = 0; i < fnParams.size(); i++) {
            if(fnParams[i]->type == nullptr && fnParams[i]->varName == nullptr) continue;

            auto paramName = fnParams[i]->getName();
            auto paramValue = ex(fc.parameterExpressions[i]->exprCode);

            fnFrame.sym2[paramName].setValue(paramValue);
        }

        *fnRef.statements->currentScope = fnFrame;
        // std::cout << fnFrame.symbolsToString() << '\n';
        auto fnRet = ex(fnRef.statements);

        // Restore the original function scope (i.e. pop the frame off the stack)
        *fnRef.statements->currentScope = originalScope;

        return fnRet;
    }

    ControlFlow operator()(StatementList& sl) {
        for(const auto& statement: sl.statements) {
            auto ret = ex(statement);

            if(ret.type == ControlFlow::Type::Return) {
                return ret;
            }

            bool isBreak = statement->is<breakNodeType>();
            if(isBreak) { return Value(0); } 
        }

        return Value(0);
    }

    ControlFlow operator()(doWhileNodeType& dw) {
        do {
            ex(dw.loop_body); 
        } while(bool(ex(dw.condition)) && !dw.break_encountered); 

        return Value(0);
    }

    ControlFlow operator()(whileNodeType& w) {
        while(bool(ex(w.condition)) && !w.break_encountered) 
            ex(w.loop_body); 

        return Value(0);
    }

    ControlFlow operator()(forNodeType& f) {
        for(ex(f.init_statement); bool(ex(f.loop_condition)) && !f.break_encountered; ex(f.post_loop_statement)) {
            ex(f.loop_body);
        }
        return Value(0);
    }

    ControlFlow operator()(BinOp& bop) const {
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
            case Mod: {
                auto l = ex(bop.lOperand);
                auto r = ex(bop.rOperand);

                return l % r;
            }
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

    ControlFlow operator()(UnOp& uop) {
        using enum UnOper;

        switch (uop.op) {
        case Print: {
            Value exprValue = ex(uop.operand);
            std::cerr << std::string(exprValue) << '\n';
            return Value(0);
        }

        case Return:
            return ControlFlow::Return(ex(uop.operand));

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

    ControlFlow operator()(IfNode& ifNode) const {
        if (ex(ifNode.condition)) {
            return ex(ifNode.ifCode);
        } else if (ifNode.elseCode != nullptr) {
            return ex(ifNode.elseCode);
        } else {
            // TODO: Return something else?
            return ControlFlow(Value(0));
        }
    }

    ControlFlow operator()(ArrayIndex& ai) const {
        auto arr = ex(ai.arrayExpr).val();
        auto ind = std::get<int>(ex(ai.indexExpr).val());
        auto* arrPtr = std::get_if<PrimitiveArray>(&arr);
        
        // Should be guaranteed that the array expression evaluates to an array.
        assert(arrPtr);

        if(ind < 0 || ind > arrPtr->size()) {
            std::cerr << fmt(
                "Attempt to index outside array bounds %s\n",
                ind < 0 ? fmt("(%d < 0)", ind)
                        : fmt("(%d > array length (%d))", ind, arrPtr->size())
            );
            // TODO: Return some sort of error?
            return Value(0);
        }

        // Semantic analysis should guarantee that index expressions are integral.
        return (*arrPtr)[ind];
    }

    // the default case:
    template<typename T> 
    ControlFlow operator()(T const & /*UNUSED*/ ) const { return Value(0);} 
};

ControlFlow ex(Node *p) {
    if (p == nullptr) return Value(0);
    return std::visit(ex_visitor{p}, *p);
}

