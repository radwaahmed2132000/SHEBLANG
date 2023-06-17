#include "const_expr_checker.h"
#include "cl.h"

struct ConstantExprCheckerVisitor {
    Node* currentNodePtr;

    ConstOpt operator()(conNodeType& con) { return con; }

    ConstOpt operator()(idNodeType& id) const {
        auto [idScope, symbolType] = getSymbolScope(id.id, currentNodePtr->currentScope);

        if(idScope != nullptr && symbolType == SymbolType::Variable) {
            auto& entry = idScope->sym2[id.id];
            if(entry.isConstant) {
                // Should be evaluated by now.
                return entry.getValue();
            }
        }

        return std::nullopt;
    }

    ConstOpt operator()(VarDefn &vd) const {
        auto initOpt = checkConstantExpressions(vd.initExpr);
        if (initOpt.has_value()) {
            auto initVal = initOpt.value();

            auto varName = vd.decl->varName->as<idNodeType>().id;
            auto [idScope, _] = getSymbolScope(varName, currentNodePtr->currentScope);

            if (idScope == nullptr) {
                auto &sym = currentNodePtr->currentScope->sym2;
                sym[varName].setValue(initVal);
            } else {
                errors.push_back(lineError(
                    "Redifinition of variable '%s'.",
                    currentNodePtr->lineNo,
                    varName
                ));
            }

            return initVal;
        }

        return std::nullopt;
    }

    ConstOpt operator() (BinOp& bop) { 
        using enum BinOper;

        auto leftOpt = checkConstantExpressions(bop.lOperand);
        auto rightOpt = checkConstantExpressions(bop.rOperand);

        // Both left and right must be constant for us to continue
        if (!leftOpt.has_value() || !rightOpt.has_value()) return std::nullopt;

        auto left = leftOpt.value();
        auto right = rightOpt.value();

        ConstOpt result = std::nullopt;
        switch (bop.op) {
            case Add:          { result = left + right;  } break;
            case Sub:          { result = left - right;  } break;
            case Mul:          { result = left * right;  } break;
            case Div:          { result = left / right;  } break; 
            case Mod:          { result = left % right;  } break;      
            case BitAnd:       { result = left & right;  } break;
            case BitOr:        { result = left | right;  } break;
            case BitXor:       { result = left ^ right;  } break;
            case LShift:       { result = left << right;  } break;
            case RShift:       { result = left >> right;  } break;
            case LessThan:     { result = left <  right;  } break;      
            case GreaterThan:  { result = left >  right;  } break;
            case Equal:        { result = left == right;  } break;
            case NotEqual:     { result = left != right;  } break;
            case GreaterEqual: { result = left >= right;  } break;
            case LessEqual:    { result = left <= right;  } break;
            case And:          { result = left && right;  } break;
            case Or:           { result = left || right;  } break;
            case Assign:       { /* Do nothing, handled in VarDefn */  } break;
        }

        return result;
    }

    ConstOpt operator()(IfNode& ifNode) {
        auto conditionResult = checkConstantExpressions(ifNode.condition);

        checkConstantExpressions(ifNode.ifCode);
        if(ifNode.elseCode != nullptr) {
            checkConstantExpressions(ifNode.elseCode);
        }

        return conditionResult.has_value();
    }

    ConstOpt operator()(StatementList& sl) {
        return Utils::all_of(sl.statements, checkConstantExpressions);
    }

    // the default case:
    template<typename T> 
    ConstOpt operator()(T const & /*UNUSED*/ ) const { return std::nullopt; }  
};

ConstOpt checkConstantExpressions(Node *p) {    
    if (p == nullptr) return std::nullopt;
    return std::visit(ConstantExprCheckerVisitor{p}, *p);
}