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
      errorsOutput.addError ("Error in line number: "\
      + std::to_string(oper->lineNo) ); \
      return left; \
    } \

#define RIGHT_VALID(oper) \
    Result right = semantic_analysis(oper); \
    if (!right.isSuccess()) { \
        errorsOutput.addError ("Error in line number: "\
        + std::to_string(oper->lineNo) ); \
        return right; \
    } \

#define NOT_CONST(oper, lineNo) \
    auto lhsName = std::get<idNodeType>(oper->un).id; \
    auto& lEntry = sym2[lhsName]; \
    if(lEntry.isConstant) {\
        errorsOutput.addError ("Error in line number: "\
        + std::to_string(lineNo) ); \
    } \

/*
  * allows casting between int, float, char & bool it's possible.
  * String casting is not allowed
  * Casting between int & float just either adds a decimal point or removes it
  * Casting between int & char just converts the int to its ASCII equivalent
  * Casting between int & bool just converts 0 to false and everything else to true
  * Casting float to char is the same as casting int to cha-r (truncates the decimal part)
  * Casting float to bool is the same as casting int to bool (0 to false, everything else to true)

  ! The way of casting will depend on the current operator type

  ? If the operator is an assignment operator (=) -> The casting target is the LHS type
  
  ? If the operator is a mathematical operator (++, --, +, -, *, /, %) ->  
  ? The default cast target is to cast to int, unless the other operand is a float, in which case the cast target is float
  ? In the case of the modulo operator, if none of the operands are float or string, the cast to int is allowed.

  ? If the operator is a bitwise operator (&, |, ^, LS, RS) -> The cast target is int
  ? The bitwise operator doesn't work with floats or strings but will function otherwise.
  
  ? If the operator is a logical operator (&&, ||) -> Cast target is bool

  ? If the operator is a relational operator (<, >, <=, >=, ==, !=)
  ? The case target depends on the two operands. If they're both numbers, no need.
  ? If one of the two operands is a number (int or float) and the other isn't, cast to the type of the 
  ? other operand. If both of the operands aren't numbers (char, bool), cast to int.
*/

Result castToTarget(std::string currentType, std::string targetType,
                    nodeType* currentNode, int lineNo) {
  if (currentType == "string" || targetType == "string") {
    return Result::Error("Error in line number: " +
                          std::to_string(lineNo) +
                          " .Cannot cast to or from string\n");
  }
  if (currentType == targetType) {
    return Result::Success(targetType);
  } else {
    currentNode->conversionType = targetType;
    return Result::Success(targetType);
  }
}

Result cast_opr(const std::string& leftType, const std::string& rightType, oprNodeType& opr) {
    if (leftType == "string" || rightType == "string") {
        return Result::Error("Error in line number: " +
                            std::to_string((opr.op[0]->lineNo)) +
                            " .Cannot cast to or from string\n");
    }
    switch (opr.oper) {
        case '=': {
          std::string castResult = leftType;
          opr.op[1]->conversionType = castResult;
        }
        break;
        case '+': case '-': case '*': case '/': {
          std::string castResult = leftType == "float" || rightType == "float" ? "float" : "int";
          if (leftType != castResult) {
              opr.op[0]->conversionType = castResult;
          } else {
              opr.op[1]->conversionType = castResult;
          }
          return Result::Success(castResult);
        }
        break;
        case '%': {
          std::string castResult = "int";
          if (leftType == "float") {
              return Result::Error("Error in line number: " +
                                    std::to_string(opr.op[0]->lineNo) +
                                    " .The LHS in a modulo operation can't be float \n");
          } else if (rightType == "float") {
              return Result::Error("Error in line number: " +
                                    std::to_string(opr.op[1]->lineNo) +
                                    " .The RHS in a modulo operation can't be float \n");
          } else {
              if (leftType != castResult) {
                  opr.op[0]->conversionType = castResult;
              } else {
                  opr.op[1]->conversionType = castResult;
              }
          }
          return Result::Success(castResult);
        }
        break;
        case '&': case '|': case '^': case LS: case RS: {
          std::string castResult = "int";
          if (leftType == "float") {
              return Result::Error("Error in line number: " +
                                    std::to_string(opr.op[0]->lineNo) +
                                    " .The LHS in a bitwise operation can't be float \n");
          } else if (rightType == "float") {
              return Result::Error("Error in line number: " +
                                    std::to_string(opr.op[1]->lineNo) +
                                    " .The RHS in bitwise operation can't be float \n");
          } else {
              if (leftType != castResult) {
                  opr.op[0]->conversionType = castResult;
              } else {
                  opr.op[1]->conversionType = castResult;
              }
          }
          return Result::Success(castResult);
        }
        break;
        case AND: case OR: {
          std::string castResult = "bool";
          if (leftType != castResult) {
              opr.op[0]->conversionType = castResult;
          } else {
              opr.op[1]->conversionType = castResult;
          }
          return Result::Success(castResult);
        }
        break;
        case GE: case LE: case '>': case '<': case EQ: case NE: {
          std::string castResult = "bool";
          if (leftType != "int" && leftType != "float") {
              if (rightType == "int" || rightType == "float") {
                  opr.op[0]->conversionType = rightType;
              } else {
                  opr.op[0]->conversionType = "int";
                  opr.op[1]->conversionType = "int";
              }
          } else if (rightType != "int" && rightType != "float") {
              opr.op[1]->conversionType = leftType;
          } else {/* Do nothing */}
          return Result::Success(castResult);
        }
        break;
        default : {
            return Result::Error("Error in line number: " +
            std::to_string(opr.op[0]->lineNo) + " .Invalid operator\n");
        }
    }
}

#define LEFT_SAME_TYPE_AS_RIGHT(left, right, lineNo, opr) \
    std::string leftType = std::get<SuccessType>(left); \
    std::string rightType = std::get<SuccessType>(right); \
    std::string finalType = leftType; \
    if (leftType != rightType) { \
      Result castResult = cast_opr(leftType, rightType, opr); \
      if (!castResult.isSuccess()) { \
        errorsOutput.addError(std::get<ErrorType>(castResult)[0]); \
      } else { \
          finalType = std::get<SuccessType>(castResult); \
        } \
    } \

#define LEFT_TYPE(left) \
    auto leftType = std::get<SuccessType>(left); \

#define RIGHT_TYPE(right) \
    auto rightType = std::get<SuccessType>(right); \

#define EXISTING_TYPE(type, lineNo) \
     static std::unordered_set<std::string> builtinTypes = {"float", "int", "char", "string", "bool"}; \
     if (builtinTypes.find(type) == builtinTypes.end()) { \
          errorsOutput.addError ("Error in line number: " + \
             std::to_string(lineNo) +" .The data type \"" \
             + type + "\" is not valid\n"); \
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
        int startingSize = errorsOutput.sizeError;
        /* Check that the type of the variable is valid */
        auto type = std::get<idNodeType>(vd.type->un).id;
        EXISTING_TYPE(type, vd.type->lineNo);
        
        /* Check if the variable is already declared */
        auto nameStr = std::get<idNodeType>(vd.var_name->un).id;

        if (sym2.find(nameStr) != sym2.end()) {
          errorsOutput.addError("Error in line number: " +
            std::to_string(vd.type->lineNo) + " .The variable " +
            nameStr + " is already declared\n");
        } else {
          /* Add the variable name & type as a new entry in the symbol table */
          SymbolTableEntry entry = SymbolTableEntry();
          entry.type = type;
          entry.isConstant = false;
          entry.initExpr = nullptr;
          sym2[nameStr] = entry;
          return Result::Success(entry.type);
        }
        return Result::Error("error");
    }

    Result operator()(VarDefn& vd) {
      int startingSize = errorsOutput.sizeError;
      /* Check if the variable is declared */
      nodeType *nt = new nodeType(VarDecl(vd.decl->type, vd.decl->var_name), vd.decl->type->lineNo);
      Result decl = semantic_analysis(nt); 
      if (!decl.isSuccess()) {
        errorsOutput.addError("Error in line number: " +
                              std::to_string(vd.initExpr->lineNo) );
      } 

      std::string declType = decl.isSuccess() ? (std::string)std::get<SuccessType>(decl) : "";

      /* Check that the initial expression is valid if it exits */
      if (vd.initExpr != nullptr) {
        Result init = semantic_analysis(vd.initExpr);
        if (!init.isSuccess()) {
          errorsOutput.addError("Error in line number: " +
          std::to_string(vd.initExpr->lineNo) );
        }

        /* Check that the left & right types are matching */
        if ((declType != "") && declType != std::get<SuccessType>(init)) {
          errorsOutput.addError("Error in line number: " +
            std::to_string(vd.initExpr->lineNo) + " .The LHS identifier type: " + declType
            + " doesn't match the RHS Expression type: "
          + std::get<SuccessType>(init) + "\n");
        }
      } else {
        /* The initialization expression doesn't exist*/
        if (vd.isConstant) {
              errorsOutput.addError("Error in line number: " +
              std::to_string(vd.decl->var_name->lineNo) + " .The constant variable " +
              std::get<idNodeType>(vd.decl->var_name->un).id + " has no value assigned to it\n");
        }
      }
      if(startingSize != errorsOutput.sizeError)
      {
        return Result::Error("error");
      }
      return Result::Success(declType);
    }

    Result operator()(idNodeType& identifier)
    { 
        /*
            TODO: Add scope checking when scoping is added 
        */
        int startingSize = errorsOutput.sizeError;
        /* Check that the identifier is declared */
        if (sym2.find(identifier.id) != sym2.end()) {
            /* Check that the identifier type is valid */
            EXISTING_TYPE(sym2[identifier.id].type, identifier.lineNo);

            return Result::Success(sym2[identifier.id].type);
        }
        else {
            errorsOutput.addError("Error in line number: " + 
            std::to_string(identifier.lineNo) + " .Identifier "
            + identifier.id + " is not declared\n");
        }
      if(startingSize != errorsOutput.sizeError)
      {
        return Result::Error("error");
      }
      return Result::Success("success"); /* Useless */
    }

    Result operator()(caseNodeType& identifier) { return Result::Success("success"); } // TODO
      
    Result operator()(switchNodeType& sw) { 
      // TODO:  checks if there are duplicate case identifiers - Not sure if this is doable using semantic analysis 
      int startingSize = errorsOutput.sizeError;
      auto var =  semantic_analysis(sw.var);
      std::string varType="";
      if(var.isSuccess()){
        varType=std::get<SuccessType>(var); 
      } else {
        errorsOutput.addError("Error in line number: " + std::to_string(sw.var->lineNo) +
                              " .The switch variable is not valid\n");
      }
      nodeType* head = sw.case_list_head;
      auto cases = std::get<caseNodeType>(head->un).toVec();
      for(int i = 0; i < cases.size(); i++) { 
        auto labelExpr = semantic_analysis(cases[i]->labelExpr);
        /* Label Expression must be of the same type as the switch variable & must be a constant or a literal */
        if (!labelExpr.isSuccess()) {
          errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->labelExpr->lineNo) +
                                " .The label expression of the case statement is not valid\n");
        } else {
          /* Make sure of the type, constancy or literally or the expression */
          std::string typeCaseExpr = std::get<SuccessType>(labelExpr);
          if (varType != "" && typeCaseExpr != varType) {
            errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->labelExpr->lineNo) +
                                  " .The label expression of the case statement is not of the same type as the switch variable\n");
          }
          bool constant = false;
          bool literal = false;
          if (std::holds_alternative<idNodeType>(cases[i]->labelExpr->un)) {
            constant = sym2[std::get<idNodeType>(cases[i]->labelExpr->un).id].isConstant;
          } else if (std::holds_alternative<conNodeType>(cases[i]->labelExpr->un)) {
            literal = true;
          }
          if (!constant && !literal) {
            errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->labelExpr->lineNo) +
                                  " .The label expression of the case statement must be a constant or a literal\n");
          }
        }
        auto caseBody = semantic_analysis(cases[i]->caseBody);
        if(!caseBody.isSuccess()) {
          errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->caseBody->lineNo) +
                                " .The case body of the case statement is not valid\n");
        }      
      }
      if(startingSize != errorsOutput.sizeError)
      {
        return Result::Error("error");
      }
      return Result::Success(varType);
    } 

    Result operator()(breakNodeType& br) { return Result::Success("success"); } // TODO

    Result operator()(enumNode& en) { return Result::Success("success"); } // TODO

    Result operator()(enumUseNode& eu) { return Result::Success("success"); } // TODO

    /* For Enums (list of identifiers) */
    Result operator()(IdentifierListNode& il) { return Result::Success("success"); } // TODO

    Result operator()(functionNodeType& fn) { return Result::Success("success"); } // TODO


    /* 
      Entry point, we get a list of statements & iterate over each of them & make sure they are
      Semantically correct
    */
    Result operator()(StatementList& sl) {
        auto statements = sl.toVec();

        Result ret = Result::Success("");
        for(const auto& statement: statements) {
            auto statementError = semantic_analysis(statement->statementCode);

            /* 
              If the child statement returned an error, convert the result
              we'll return to an error if it's not already, then add all child
              statement errors to the returned result.
            */

            if(auto* e = std::get_if<ErrorType>(&statementError); e) {
                if(ret.isSuccess()) {
                    ret = statementError;
                }
                ret.mergeErrors(*e);
            }
        }
        return ret;
    }
    
    Result operator()(doWhileNodeType& dw) {
      int startingSize = errorsOutput.sizeError;

      auto condition = semantic_analysis(dw.condition);
      if (!condition.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(dw.condition->lineNo) +
                              " .The condition in do while loop is not valid\n");
      } else {
        std::string conditionType = std::get<SuccessType>(condition);
        if (conditionType != "bool") {
          Result conditionCastResult = castToTarget(conditionType, "bool",
                                                  dw.condition, dw.condition->lineNo);
          if (!conditionCastResult.isSuccess()) {
            errorsOutput.addError(std::get<ErrorType>(conditionCastResult)[0]);
          }
        }
      }

      auto loop = semantic_analysis(dw.loop_body); 
      if (!loop.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(dw.loop_body->lineNo) +
                              " .The body in do while loop is not valid\n");
      } 
      
      if(startingSize != errorsOutput.sizeError)
      {
        return Result::Error("error");
      }
      
      return Result::Success(std::get<SuccessType>(condition));
    }

    Result operator()(whileNodeType& w) { 
      int startingSize = errorsOutput.sizeError;
      auto condition = semantic_analysis( w.condition);
      if (!condition.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(w.condition->lineNo) +
                              " .The condition in while loop is not valid\n");
      } else {
        std::string conditionType = std::get<SuccessType>(condition);
        if (conditionType != "bool") {
          Result conditionCastResult = castToTarget(conditionType, "bool",
                                                  w.condition, w.condition->lineNo);
          if (!conditionCastResult.isSuccess()) {
            errorsOutput.addError(std::get<ErrorType>(conditionCastResult)[0]);
          }
        }
      }  
      auto loop = semantic_analysis( w.loop_body);
      if (!loop.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(w.loop_body->lineNo) +
                              " .The body in while loop is not valid\n");
      }

      if(startingSize != errorsOutput.sizeError)
      {
        return Result::Error("error");
      }

      return Result::Success(std::get<SuccessType>(condition));
     }

    Result operator()(forNodeType& f) { 
      int startingSize = errorsOutput.sizeError;
      
      auto init_statement = semantic_analysis( f.init_statement);
      if (!init_statement.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(f.init_statement->lineNo) +
                              " .The initialization statement in the for loop is not valid\n");
      }
      
      auto condition = semantic_analysis(f.loop_condition);
      if (!condition.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(f.loop_condition->lineNo) +
                              " .The condition in the for loop is not valid\n");
      } else {
        std::string conditionType = std::get<SuccessType>(condition);
        if (conditionType != "bool") {
          Result conditionCastResult = castToTarget(conditionType, "bool",
                                                  f.loop_condition, f.loop_condition->lineNo);
          if (!conditionCastResult.isSuccess()) {
            errorsOutput.addError(std::get<ErrorType>(conditionCastResult)[0]);
          }
        }
      }

      auto loop = semantic_analysis(f.loop_body);
      if (!loop.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(f.loop_body->lineNo) +
                              " .The body in the for loop is not valid\n");
      }
      
      auto post_statement = semantic_analysis(f.post_loop_statement);
      if (!post_statement.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(f.post_loop_statement->lineNo) +
                              " .The post statement in the for loop is not valid\n");
      }
             
      if(startingSize != errorsOutput.sizeError)
      {
          return Result::Error("error");
      }             
     return Result::Success(std::get<SuccessType>(condition));
    }
           

    Result operator()(oprNodeType& opr) {   
        /* Things that need to be checked */
        /*
            TODO: Implement the follow check when the function identifier is added to the symbol table
            * Check that the right expression is not a function identifier but a function call *
            * Check that the left identifier is not a function *
            
            ! Do some operator specific checks ! 
        */
        int startingSize = errorsOutput.sizeError;
        switch (opr.oper) {

        case '=': {
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0], opr.op[0]->lineNo); 
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo, opr); // * gives leftType & rightType & finalType

          if (startingSize != errorsOutput.sizeError) {
            return Result::Error("error");
          }
          
          return Result::Success(finalType);
        }
        break;

        /* Mathematical Operators */
        
        case PP: case MM: {
          /* Check that the left identifier is not a constant */
          NOT_CONST(opr.op[0], opr.op[0]->lineNo); 
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          LEFT_TYPE(left);

          /* Manual casting without macro */
          if (leftType == "string") {
              errorsOutput.addError("Error in line number: " +
              std::to_string(opr.op[0]->lineNo) + " .Cannot cast to or from string\n");  
          } else if (leftType == "int" || leftType == "float") {
              return Result::Success(leftType);
          } else if (leftType == "char") {
              return Result::Success("int");
          } else {
              errorsOutput.addError("Error in line number: " +
              std::to_string(opr.op[0]->lineNo) + " .Can't Increment/Decrement booleans\n");
          }

          if (startingSize != errorsOutput.sizeError) {
            return Result::Error("error");
          }
          
          return Result::Success(leftType);
        }
      break;
      case AND : case OR : {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo, opr); // * gives leftType & rightType & finalType
        /* Check that the left and right are bool */
        
        if (startingSize != errorsOutput.sizeError) {
          return Result::Error("error");
        }

        return Result::Success(finalType);
        }
        break;
        
      case '+': case '-': case '*': case '/': case '%':  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo, opr); // * gives leftType & rightType & finalType
        /* Check that the left and right are either both integers or both float */

        if (startingSize != errorsOutput.sizeError) {
          return Result::Error("error");
        }

        return Result::Success(finalType);
      }
      break;
      case GE: case LE:   case '>': case '<' :{
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          /* Check that the right expression is semantically valid */
          RIGHT_VALID(opr.op[1]); // * gives right
          /*  Check that the two expressions on the left & on the right are of the same type */
          LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo, opr); // * gives leftType & rightType & finalType

          if (startingSize != errorsOutput.sizeError) {
            return Result::Error("error");
          }

          return Result::Success(finalType);
        }
      break;
      
      case EQ: case NE:  {
        /* Check that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        /*  Check that the two expressions on the left & on the right are of the same type */
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo, opr); // * gives leftType & rightType & finalType
        
        if (startingSize != errorsOutput.sizeError) {
          return Result::Error("error");
        }

        return Result::Success(finalType);
      }
      break;
      case '&': case '^': case '|': case LS: case RS:  {
        /* Che3 && 4ck that the left expression is valid (identifier is declared) */
        LEFT_VALID(opr.op[0]); // * gives left
        /* Check that the right expression is semantically valid */
        RIGHT_VALID(opr.op[1]); // * gives right
        LEFT_SAME_TYPE_AS_RIGHT(left, right, opr.op[0]->lineNo, opr); // * gives leftType & rightType & finalType

        if (startingSize != errorsOutput.sizeError) {
          return Result::Error("error");
        }

        return Result::Success(finalType);
      }
      break;

      case PRINT: {
        /* Check that the right expression is valid */
        RIGHT_VALID(opr.op[0]); // * gives right
        return right;
      }
      break;  
      case IF : {
        Result condition = semantic_analysis(opr.op[0]);
        if (!condition.isSuccess()) {
          errorsOutput.addError("Error in line number: " +
                                std::to_string(opr.op[0]->lineNo) +
                                " .Condition in an if statement is invalid");
        } else {
          auto conditionType = std::get<SuccessType>(condition);
          if(conditionType != "bool") {
            Result conditionCastResult = castToTarget(conditionType, "bool", opr.op[0], opr.op[0]->lineNo);
            if (!conditionCastResult.isSuccess()) {
              errorsOutput.addError(std::get<ErrorType>(conditionCastResult)[0]);
            }
          }
        }

        Result ifBody = semantic_analysis(opr.op[1]);
        if (!ifBody.isSuccess()) {
          errorsOutput.addError("Error in line number: " +
                                std::to_string(opr.op[1]->lineNo) +
                                " .Body of an if statement is invalid");
        }

        if (startingSize != errorsOutput.sizeError) {
          return Result::Error("error");
        }

        return Result::Success(std::get<SuccessType>(condition));
      }
      break;
      default:
          return Result::Success("success"); 
          break;
      }
    if(startingSize != errorsOutput.sizeError)
    {
        return Result::Error("error");
    } 
    return Result::Success("success"); 
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


