#include <variant>
#include <algorithm>
#include <iostream>
#include <optional>

#include "cl.h"
#include "y.tab.h"
#include "result.h"

#define LEFT_VALID(oper) \
    Result left = std::visit(semantic_analysis_visitor(), oper->un); \
    if (!left.isSuccess()) { \
        return left; \
    } \

#define RIGHT_VALID(oper) \
    Result right = std::visit(semantic_analysis_visitor(), oper->un); \
    if (!right.isSuccess()) { \
        return right; \
    } \

#define NOT_CONST(oper) \
    auto lhsName = std::get<idNodeType>(oper->un).id; \
    auto& lEntry = sym2[lhsName]; \
    if(lEntry.isConstant) { \
        return Result::Error("Error: The left identifier is a constant\n"); \
    } \

#define LEFT_SAME_TYPE_AS_RIGHT(left, right) \
    auto leftType = std::get<SuccessType>(left); \
    auto rightType = std::get<SuccessType>(right); \
    if (leftType != rightType) { \
        return Result::Error("Error: The LHS identifier type: " + \
                 std::get<SuccessType>(left) + " doesn't match the RHS Expression type" \
                 + std::get<SuccessType>(right) + "\n"); \
    } \

#define TYPE_VAR(left) \
    auto leftType = std::get<SuccessType>(left); \
   \

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
            
            TODO: Don't allow the declaration of a variable with the same name as a function

            ! Ask Tarek about this when he wakes up.

            TODO: Check that the variable even has a type

            TODO: Check that the type of the variable is valid
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
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0]); 
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right); // * gives leftType & rightType
          return Result::Success(leftType);
        }
        break;

        /* Mathematical Operators */
        /*
            TODO: Alow float & int interoperability  
        */

        case PA: case SA: case MA: case DA: case RA: {
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0]); 
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right); // * gives leftType & rightType
          /* Check that the left and right are either both integers or both float */
          if (leftType != "int" && leftType != "float") {
            return Result::Error("Error: The LHS identifier type: " + leftType + " is not a valid type for a mathematical operation\n");
          }
          if (rightType != "int" && rightType != "float") {
            return Result::Error("Error: The RHS identifier type: " + rightType + " is not a valid type for a mathematical operation\n");
          }
          if (opr.oper== RA)
          {
            if (leftType != "int" || rightType != "int") {
              return Result::Error("Error: The LHS and RHS must be integers in mod  operation \n");
            }
          }
          return Result::Success(leftType);
        }
        break;

        case PP: case MM:
        {
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0]); 
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          TYPE_VAR(left);
          if (leftType != "int" && leftType != "float") {
                return Result::Error("Error: The LHS identifier type: " + leftType + " is not a valid type for a mathematical operation\n");
          }
        }
      break;
      case AND : case OR :   {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right); // * gives leftType & rightType
        /* Check that the left and right are bool */
        if (leftType != "bool" ) {
          return Result::Error("Error: The LHS identifier type: " + leftType + " is not a valid type for a logical operation\n");
        }
        if (rightType != "bool" ) {
          return Result::Error("Error: The RHS identifier type: " + rightType + " is not a valid type for a logical operation\n");
        }
        return Result::Success(leftType);
        }
      break;
        
      case '+': case '-': case '*': case '/': case '%':  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right); // * gives leftType & rightType
        /* Check that the left and right are either both integers or both float */
        if (leftType != "int" && leftType != "float") {
          return Result::Error("Error: The LHS identifier type: " + leftType + " is not a valid type for a mathematical operation\n");
        }
        if (rightType != "int" && rightType != "float") {
          return Result::Error("Error: The RHS identifier type: " + rightType + " is not a valid type for a mathematical operation\n");
        }
        if (opr.oper== '%')
        {
          if (leftType != "int" || rightType != "int") {
            return Result::Error("Error: The LHS and RHS must be integers in mod  operation \n");
                
          }
        }
        return Result::Success(leftType);
      }
      break;
      case GE: case LE:  {
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right); // * gives leftType & rightType
          /* Check that the left and right are either both integers or both float */
          if (leftType != "int" && leftType != "float") {
            return Result::Error("Error: The LHS identifier type: " + leftType + " is not a valid type for a mathematical operation\n");
          }
          if (rightType != "int" && rightType != "float") {
            return Result::Error("Error: The RHS identifier type: " + rightType + " is not a valid type for a mathematical operation\n");
          }
            return Result::Success(leftType);
        }
      break;
      case EQ: case NE:  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right); // * gives leftType & rightType 
        return Result::Success(leftType);
      }
      break;  
      case ANDA: case EORA: case IORA: case LSA: case RSA:  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        TYPE_VAR(left);
        if (leftType != "int" ) {
          return Result::Error("Error: The LHS identifier type: " + leftType + " is not a valid type for a mathematical operation\n");
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


