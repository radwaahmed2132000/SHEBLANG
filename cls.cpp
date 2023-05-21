#include <variant>
#include <algorithm>
#include <iostream>
#include <optional>
#include <unordered_set>

#include "cl.h"
#include "y.tab.h"
#include "result.h"

/*
  TODO: return line number along with error message
*/

#define LEFT_VALID(oper) \
    Result left = semantic_analysis(oper); \
    if (!left.isSuccess()) { \
        return left; \
    } \

#define RIGHT_VALID(oper) \
    Result right = semantic_analysis(oper); \
    if (!right.isSuccess()) { \
        return right; \
    } \

#define NOT_CONST(oper, lineNo) \ 
    auto lhsName = std::get<idNodeType>(oper->un).id; \
    auto& lEntry = sym2[lhsName]; \
    if(lEntry.isConstant) { \
        return Result::Error("Error in line number: " \
        + std::to_string(lineNo) + \ 
         + " .The left identifier is a constant\n"); \
    } \

#define LEFT_SAME_TYPE_AS_RIGHT(left, right, lineNo) \
    auto leftType = std::get<SuccessType>(left); \
    auto rightType = std::get<SuccessType>(right); \
    if (leftType != rightType) { \
        return Result::Error("Error in line number: " + \
                std::to_string(lineNo) + " .The LHS identifier type: " + \
                std::get<SuccessType>(left) + " doesn't match the RHS Expression type: " \
                + std::get<SuccessType>(right) + "\n"); \
    } \

#define LEFT_TYPE(left) \
    auto leftType = std::get<SuccessType>(left); \
   \

#define RIGHT_TYPE(right) \
    auto rightType = std::get<SuccessType>(right); \

#define EXISTING_TYPE(type, lineNo) \
     static std::unordered_set<std::string> builtinTypes = {"float", "int", "char", "string", "bool"}; \
     if (builtinTypes.find(type) == builtinTypes.end()) { \
            return Result::Error("Error in line number: " + \
             std::to_string(lineNo) +" .The type of the variable " \
             + type + " is not valid\n"); \
      } \

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
        */
       
        /* Check that the type of the variable is valid */
        auto type = std::get<idNodeType>(vd.type->un).id;
        EXISTING_TYPE(type, vd.type->lineNo);
        
        /* Check if the variable is already declared */
        auto nameStr = std::get<idNodeType>(vd.var_name->un).id;

        if (sym2.find(nameStr) != sym2.end()) {
            return Result::Error("Error in line number: " +
             std::to_string(vd.type->lineNo) + " .The variable " +
             nameStr + " is already declared\n");
        }

        /* Add the variable name & type as a new entry in the symbol table */
        SymbolTableEntry entry = SymbolTableEntry();
        entry.type = type;
        entry.isConstant = false;
        entry.initExpr = nullptr;
        sym2[nameStr] = entry;

       return Result::Success(entry.type);
    }

    Result operator()(VarDefn& vd) {

      /* Check if the variable is declared */
      nodeType *nt = new nodeType(VarDecl(vd.decl->type, vd.decl->var_name), vd.decl->type->lineNo);
      Result decl = semantic_analysis(nt); 
      if (!decl.isSuccess()) {
          return decl;
      }
      std::string declType = std::get<SuccessType>(decl);

      /* Check that the initial expression is valid if it exits */
        if (vd.initExpr != nullptr) {
          Result init = semantic_analysis(vd.initExpr);
          if (!init.isSuccess()) {
              return init;
          }
          /* Check that the left & right types are matching */
          if (declType != std::get<SuccessType>(init)) {
              return Result::Error("Error in line number: " +
               std::to_string(vd.initExpr->lineNo) + " .The LHS identifier type: " + declType
               + " doesn't match the RHS Expression type: "
              + std::get<SuccessType>(init) + "\n");
          }
        } else {
            /* The initialization expression doesn't exist*/
            if (vd.isConstant) {
                return Result::Error("Error in line number: " +
                 std::to_string(vd.decl->var_name->lineNo) + " .The constant variable " +
                 std::get<idNodeType>(vd.decl->var_name->un).id + " has no value assigned to it\n");
            }
        }

      return Result::Success(declType);
    }

    Result operator()(idNodeType& identifier)
    { 
        /*
            TODO: Add scope checking when scoping is added 
        */

        /* Check that the identifier is declared */
        if (sym2.find(identifier.id) != sym2.end()) {
            /* Check that the identifier type is valid */
            EXISTING_TYPE(sym2[identifier.id].type, identifier.lineNo);

            return Result::Success(sym2[identifier.id].type);
        }
        else {
            return Result::Error("Error in line number: " + 
            std::to_string(identifier.lineNo) + " .Identifier "
            + identifier.id + " is not declared\n");
        }
    }

    Result operator()(caseNodeType& identifier) { return Result::Success("success"); }

    Result operator()(switchNodeType& sw) { return Result::Success("success"); }

    Result operator()(breakNodeType& br) { return Result::Success("success"); }

    Result operator()(functionNodeType& fn) { return Result::Success("success"); }

    Result operator()(doWhileNodeType& dw) {

      auto condition= semantic_analysis(dw.condition) ;
      auto loop= semantic_analysis( dw.loop_body);  
      if (condition.isSuccess() && loop.isSuccess())
      {
        auto conditionType =std::get<SuccessType>(condition); 
        if(conditionType=="bool")
          return Result::Success(conditionType); 
        else
          return Result::Error("Error in line number: " +
            std::to_string(dw.condition->lineNo) +
            " .The condition in do while loop is not a boolean");
      }
      else {
        if (!condition.isSuccess())
          return condition; 
        else 
          return loop;
      }
    }

    Result operator()(whileNodeType& w) { 
      auto loop= semantic_analysis( w.loop_body); 
      auto condition= semantic_analysis( w.condition);   
      if (condition.isSuccess() && loop.isSuccess())
      {
        auto conditionType =std::get<SuccessType>(condition); 
        if(conditionType=="bool")
          return Result::Success(conditionType); 
        else
          return Result::Error("Error in line number: " +
            std::to_string(w.condition->lineNo) +
            " .The condition in do while loop is not a boolean");
      }
      else {
        if (!condition.isSuccess())
          return condition; 
        else 
          return loop;
      }
     }

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

        case ';': {
            /* Check that the first statement is semantically correct */
            Result first = semantic_analysis(opr.op[0]);
            if (!first.isSuccess()) {
                return first;
            }

            /* Check that the rest of the statements are semantically correct */
            Result second = semantic_analysis(opr.op[1]);
            if (!second.isSuccess()) {
                return second;
            }

            return Result::Success(std::get<SuccessType>(second));
        }

        case '=': {
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0], opr.op[0]->lineNo); 
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo); // * gives leftType & rightType
          return Result::Success(leftType);
        }
        break;

        /* Mathematical Operators */
        /*
            TODO: Alow float & int interoperability  
        */

        case PP: case MM: {
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0], opr.op[0]->lineNo); 
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          LEFT_TYPE(left);
          if (leftType != "int" && leftType != "float") {
                return Result::Error("Error in line number: " +
                std::to_string(opr.op[0]->lineNo) + " .The LHS identifier type: " +
                leftType + " is not a valid type for a mathematical operation\n");
          }
        }
      break;
      case AND : case OR :   {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo); // * gives leftType & rightType
        /* Check that the left and right are bool */
        if (leftType != "bool" ) {
          return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The LHS identifier type: " 
          + leftType + " is not a valid type for a logical operation\n");
        }
        if (rightType != "bool" ) {
          return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The RHS identifier type: " 
          + rightType + " is not a valid type for a logical operation\n");
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
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo); // * gives leftType & rightType
        /* Check that the left and right are either both integers or both float */
        if (leftType != "int" && leftType != "float") {
          return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The LHS identifier type: " 
          + leftType + " is not a valid type for a mathematical operation\n");
        }
        if (rightType != "int" && rightType != "float") {
          return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The RHS identifier type: " 
          + rightType + " is not a valid type for a mathematical operation\n");
        }
        if (opr.oper== '%')
        {
          if (leftType != "int" || rightType != "int") {
            return Result::Error("Error in line number: " + 
            std::to_string(opr.op[0]->lineNo) +
            " .The LHS and RHS must be integers in module operation \n");
          }
        }
        return Result::Success(leftType);
      }
      break;
      case GE: case LE:   case '>': case '<' :{
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo); // * gives leftType & rightType
          /* Check that the left and right are either both integers or both float */
          if (leftType != "int" && leftType != "float") {
            return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The LHS identifier type: " 
          + leftType + " is not a valid type for a mathematical operation\n");
          }
          if (rightType != "int" && rightType != "float") {
            return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The RHS identifier type: " 
          + rightType + " is not a valid type for a mathematical operation\n");
          }
            return Result::Success("bool");
        }
      break;
      case EQ: case NE:  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo); // * gives leftType & rightType 
        return Result::Success("bool");
      }
      break;
      case '&': case '^': case '|': case LS: case RS:  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        LEFT_TYPE(left);
        RIGHT_TYPE(right);
        if (leftType != "int" ) {
          return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The LHS identifier type: " 
          + leftType + " is not a valid type for a bitwise operation\n");
        }
        if (rightType != "int" ) {
          return Result::Error("Error in line number: " +
          std::to_string(opr.op[0]->lineNo) + " .The LHS identifier type: " 
          + rightType + " is not a valid type for a bitwise operation\n");
        }

        return Result::Success(leftType);
      }
      break;

      
      case PRINT: {
        /* Check that the right expression is valid */
        RIGHT_VALID(opr.op[0]); // * gives right
        return right;
      }
      break;  
      case IF :
          {
           Result condition = semantic_analysis(opr.op[0]);
            if (condition.isSuccess() ) {
              auto conditionType =std::get<SuccessType>(condition);
              if(conditionType=="bool")
                return Result::Success(conditionType);
              else
              return Result::Error("Error in line number: " +
              std::to_string(opr.op[0]->lineNo) +
              " .Condition in an if statement is not boolean");
            } else{
                return condition;
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


