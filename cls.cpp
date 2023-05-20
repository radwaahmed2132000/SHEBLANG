#include <variant>
#include <algorithm>
#include <iostream>
#include <optional>

#include "cl.h"
#include "y.tab.h"
#include "result.h"

struct semantic_analysis_visitor {
    
    Result operator()(conNodeType& con) { 
        auto type = std::visit(
            Visitor {
                    [](int iValue)         { return "int"; },
                    [](float fValue)       { return "float"; },
                    [](bool bValue)        { return "bool"; },
                    [](char cValue)        { return "char"; },
                    [](std::string sValue) { return "string"; },
            },
            con
        );
        return Result::Success(type);
    }

    Result operator()(VarDecl& vd) { 
        /*
            TODO: Disallow redeclaration of variables within the same scope 
        */
    }

    Result operator()(idNodeType& identifier)
    { 
        /*
            TODO: Add scope checking when scoping is added 
        */

        if (sym2.find(identifier.id) != sym2.end()) {
            return Result::Success(sym2[identifier.id].type);
        }
        else {
            return Result::Error("Error: Identifier " + identifier.id + " is not declared\n");
        }
    }

    Result operator()(caseNodeType& identifier) { return Result::Success("success"); }

    Result operator()(switchNodeType& sw) { return Result::Success("success"); }

    Result operator()(breakNodeType& br) { return Result::Success("success"); }

    Result operator()(functionNodeType& fn) { return Result::Success("success"); }

    Result operator()(doWhileNodeType& dw) { return Result::Success("success"); }

    Result operator()(whileNodeType& w) { return Result::Success("success"); }

    Result operator()(forNodeType& f) { return Result::Success("success"); }

    Result operator()(oprNodeType& opr) {   
        /* Things that need to be checked */
        /*
            TODO: Implement the follow check when the function identifier is added to the symbol table
            * Check that the right expression is not a function identifier but a function call *
            * Check that the left identifier is not a function *

            * Check that the two expressions on the left and on the right are of the same type *
            
            ! Do some operator specific checks ! 
        */
        switch (opr.oper) {

        case '=': {
            /* Check that the left identifier is declared */
            Result left = std::visit(semantic_analysis_visitor(), opr.op[0]->un);
            
            if (!left.isSuccess()) {
                return left;
            }

            /* Check that the left identifier is not a constant */
            auto lhsName = std::get<idNodeType>(opr.op[0]->un).id;
            auto& lEntry = sym2[lhsName];
            if(lEntry.isConstant) {
                return Result::Error("Error: The left identifier is a constant\n");
            }

            /* Check that the right expression is semantically valid */
            Result right = std::visit(semantic_analysis_visitor(), opr.op[1]->un);

            if (!right.isSuccess()) {
                return right;
            }

            /*  Check that the two expressions on the left & on the right are of the same type */
            if (std::get<SuccessType>(left) == std::get<SuccessType>(right)) {
                return left;
            }
            else {
                return Result::Error("Error: The LHS identifier type: " +
                 std::get<SuccessType>(left) + " doesn't match the RHS Expression type"
                 + std::get<SuccessType>(right) + "\n");
            }  
        }
        break;
        
        default:
            return Result::Success("success"); 
            break;
        }
    }

    // the default case:
    template<typename T> 
    Result operator()(T const & /*UNUSED*/ ) const { return Result::Success("success"); } 
};

Result semantic_analysis(nodeType *p) {    
    // std::cout << "Semantic analysis running\n";
    if (p == nullptr) return Result::Success("success");
    return std::visit(semantic_analysis_visitor(), p->un);
}


