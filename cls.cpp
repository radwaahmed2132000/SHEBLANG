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

#define RIGHT_VALID(oper) \
    Result right = semantic_analysis(oper); \

// TODO: Make sure sym2 works properly
#define NOT_CONST(oper, lineNo, sym2) \
    auto lhsName = std::get<idNodeType>(oper->un).id; \
      if (sym2.find(lhsName) != sym2.end()) { \
        auto& lEntry = sym2[lhsName]; \
      if(lEntry.isConstant) {\
          errorsOutput.addError ("Error in line number: "\
          + std::to_string(lineNo) + " The LHS of an assignment operation is constant."); \
    } \
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
                          " .Cannot cast to or from string");
  }
  if (currentType == targetType) {
    return Result::Success(targetType);
  } else {
    currentNode->conversionType = targetType;
    return Result::Success(targetType);
  }
}

Result cast_opr(const std::string& leftType, const std::string& rightType, oprNodeType& opr) {
  int startingSize = errorsOutput.sizeError;
    if (leftType == "string" && rightType == "string") {
        if (opr.oper == '=') {
            return Result::Success("string");
        } else if (opr.oper == EQ or opr.oper == NE) {
            return Result::Success("bool");
        } else {
          errorsOutput.addError("Error in line number: " +
                                std::to_string(opr.op[0]->lineNo) +
                                " .Cannot perform this operation on strings");
        }
    } else if (leftType == "string" || rightType == "string")  {
          errorsOutput.addError("Error in line number: " +
                                std::to_string((opr.op[0]->lineNo)) +
                                " .Cannot cast to or from string");
      }
    switch (opr.oper) {
        case '=': {
          std::string castResult = leftType;
          if (startingSize == errorsOutput.sizeError) {
            if (rightType != castResult) {
                opr.op[1]->conversionType = castResult;
            }
          } else {
            return Result::Error("Error");
          }

          return Result::Success(castResult);
        }
        break;
        case '+': case '-': case '*': case '/': {
          std::string castResult = leftType == "float" || rightType == "float" ? "float" : "int";
          if (startingSize == errorsOutput.sizeError) {
            if (leftType != castResult) {
              opr.op[0]->conversionType = castResult;
            } 
            if (rightType != castResult){
              opr.op[1]->conversionType = castResult;
            }
          } else {
            return Result::Error("Error");
          }

          return Result::Success(castResult);
        }
        break;
        case '%': {
          std::string castResult = "int";
          if (leftType == "float" || leftType == "string") {
              errorsOutput.addError("Error in line number: " +
                                    std::to_string(opr.op[0]->lineNo) +
                                    " .The LHS in a modulo operation can't be " + leftType);
          } 
          if (rightType == "float" || rightType == "string") {
              errorsOutput.addError("Error in line number: " +
                                    std::to_string(opr.op[1]->lineNo) +
                                    " .The RHS in a modulo operation can't be " + rightType);
          } 
          if (startingSize == errorsOutput.sizeError) {
            if (leftType != castResult) {
              opr.op[0]->conversionType = castResult;
            } 
            if ( rightType != castResult) {
              opr.op[1]->conversionType = castResult;
            }
          } else {
            return Result::Error("Error");
          }
          
          return Result::Success(castResult);
        }
        break;
        case '&': case '|': case '^': case LS: case RS: {
          std::string castResult = "int";
          if (leftType == "float" || leftType == "string") {
              errorsOutput.addError("Error in line number: " +
                                    std::to_string(opr.op[0]->lineNo) +
                                    " .The LHS in a bitwise operation can't be " + leftType);
          } 
          if (rightType == "float" || rightType == "string") {
              errorsOutput.addError("Error in line number: " +
                                    std::to_string(opr.op[1]->lineNo) +
                                    " .The RHS in bitwise operation can't be " + rightType);
          } 
          if (startingSize == errorsOutput.sizeError) {
              if (leftType != castResult) {
                  opr.op[0]->conversionType = castResult;
              } 
              if (rightType != castResult) {
                opr.op[1]->conversionType = castResult;
              }
          } else {
            return Result::Error("Error");
          }
          
          return Result::Success(castResult);
        }
        break;
        case AND: case OR: {
          std::string castResult = "bool";
          if (startingSize == errorsOutput.sizeError) {
            if (leftType != castResult) {
              opr.op[0]->conversionType = castResult;
            } 
            if (rightType != castResult) {
              opr.op[1]->conversionType = castResult;
            }
          } else {
            return Result::Error("Error");
          }
          return Result::Success(castResult);
        }
        break;
        case GE: case LE: case '>': case '<': case EQ: case NE: {
          std::string castResult = "bool";
          if (startingSize == errorsOutput.sizeError) {
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
          } else {
            return Result::Error("Error");
          }
          return Result::Success(castResult);
        }
        break;
        default : {
            return Result::Error("Error in line number: " +
            std::to_string(opr.op[0]->lineNo) + " .Invalid operator");
        }
    }
}

#define LEFT_SAME_TYPE_AS_RIGHT(left, right, lineNo, opr) \
    if (!left.isSuccess()) { \
        return left; \
    } \
    if (!right.isSuccess()) { \
        return right; \
    } \
    std::string leftType = std::get<SuccessType>(left); \
    std::string rightType = std::get<SuccessType>(right); \
    std::string finalType = leftType; \
    Result castResult = cast_opr(leftType, rightType, opr); \
    if (!castResult.isSuccess()) { \
    } else { \
        finalType = std::get<SuccessType>(castResult); \
      } \

#define LEFT_TYPE(left) \
    auto leftType = std::get<SuccessType>(left); \

#define RIGHT_TYPE(right) \
    auto rightType = std::get<SuccessType>(right); \

#define EXISTING_TYPE(type, lineNo) \
     static std::unordered_set<std::string> builtinTypes = {"float", "int", "char", "string", "bool"}; \
     if (type == "<no type>") { \
          \
     } else if (builtinTypes.find(type) == builtinTypes.end()) { \
          errorsOutput.addError ("Error in line number: " + \
             std::to_string(lineNo) +" .The data type \"" \
             + type + "\" is not valid"); \
      } \

struct semantic_analysis_visitor {
    nodeType* currentNodePtr;
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
            DONE: Disallow redeclaration of variables within the same scope 
            
            TODO: Don't allow the declaration of a variable with the same name as a function
        */
        int startingSize = errorsOutput.sizeError;
        /* Check that the type of the variable is valid */
        auto type = std::get<idNodeType>(vd.type->un).id;

        /* Check that the type & name are valid */

        EXISTING_TYPE(type, vd.type->lineNo);
        
        auto symTable = currentNodePtr->currentScope;
        /* Check if the variable is already declared in this scope */
        auto nameStr = std::get<idNodeType>(vd.var_name->un).id;
        if (symTable->sym2.find(nameStr) != symTable->sym2.end()) {
          errorsOutput.addError("Error in line number: " +
            std::to_string(vd.type->lineNo) + " .The variable " +
            nameStr + " is already declared in this scope.");
        } else {
          /* Add the variable name & type as a new entry in the symbol table */
          SymbolTableEntry entry = SymbolTableEntry();
          entry.type = type;
          entry.isConstant = false;
          entry.initExpr = nullptr;
          symTable->sym2[nameStr] = entry;
          return Result::Success(entry.type);
        }
        return Result::Error("error");
    }

    Result operator()(VarDefn& vd) {
      int startingSize = errorsOutput.sizeError;
      /* Check if the variable is declared */
      nodeType *nt = new nodeType(VarDecl(vd.decl->type, vd.decl->var_name), vd.decl->type->lineNo);
      nt->currentScope = currentNodePtr->currentScope;
      Result decl = semantic_analysis(nt); 
      std::string declType = decl.isSuccess() ? (std::string)std::get<SuccessType>(decl) : "<no type>";

      /* Check that the initial expression is valid if it exits */
      if (vd.initExpr != nullptr) {
        Result init = semantic_analysis(vd.initExpr);
      } else {
        /* The initialization expression doesn't exist*/
        if (vd.isConstant) {
              errorsOutput.addError("Error in line number: " +
              std::to_string(vd.decl->var_name->lineNo) + " .The constant variable " +
              std::get<idNodeType>(vd.decl->var_name->un).id + " has no value assigned to it");
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
            DONE: Add scope checking when scoping is added 
        */
        int startingSize = errorsOutput.sizeError;
        auto symTable = identifier.scopeNodePtr->currentScope;
        /* Check that the identifier is declared */
        if (symTable->sym2.find(identifier.id) != symTable->sym2.end()) {
            /* Check that the identifier type is valid */
            EXISTING_TYPE(symTable->sym2[identifier.id].type, identifier.lineNo);

            return Result::Success(symTable->sym2[identifier.id].type);
        }
        else {
            auto parentTable = symTable->parentScope;
            while (parentTable != nullptr)
            {
              if (parentTable->sym2.find(identifier.id) != parentTable->sym2.end()) {
                /* Check that the identifier exists in an outer scope */
                EXISTING_TYPE(parentTable->sym2[identifier.id].type, identifier.lineNo);
                return Result::Success(parentTable->sym2[identifier.id].type);
              }
              parentTable = parentTable->parentScope;
            }
            /* If not, then it's not in any scope. */
            errorsOutput.addError("Error in line number: " + 
            std::to_string(identifier.lineNo) + " .Identifier "
            + identifier.id + " is not declared");
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
      std::string varType="<no type>";
      if(var.isSuccess()){
        varType=std::get<SuccessType>(var); 
      } else {
        errorsOutput.addError("Error in line number: " + std::to_string(sw.var->lineNo) +
                              " .The switch variable is not valid");
      }
      nodeType* head = sw.case_list_head;
      auto cases = std::get<caseNodeType>(head->un).toVec();
      for(int i = 0; i < cases.size(); i++) { 
        auto labelExpr = semantic_analysis(cases[i]->labelExpr);
        /* Label Expression must be of the same type as the switch variable & must be a constant or a literal */
        if (!labelExpr.isSuccess()) {
          errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->labelExpr->lineNo) +
                                " .The label expression of the case statement is not valid");
        } else {
          if (cases[i]->labelExpr != nullptr) {
            /* Make sure of the type, constancy or literally or the expression */
            std::string typeCaseExpr = std::get<SuccessType>(labelExpr);
            if (varType != "<no type>" && typeCaseExpr != varType) {
            errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->labelExpr->lineNo) +
                                  " .The label expression of the case statement is not of the same type as the switch variable");
            }
            
            bool constant = false;
            bool literal = false;
            if (std::holds_alternative<idNodeType>(cases[i]->labelExpr->un)) {
              auto& labelName = std::get<idNodeType>(cases[i]->labelExpr->un).id;
              auto& labelSymTableEntry = varSymTableEntry(labelName, currentNodePtr->currentScope);
              constant = labelSymTableEntry.isConstant;
            } else if (std::holds_alternative<conNodeType>(cases[i]->labelExpr->un)) {
              literal = true;
            }
            if (!constant && !literal) {
              errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->labelExpr->lineNo) +
                                    " .The label expression of the case statement must be a constant or a literal");
            }
          }
        }
        auto caseBody = semantic_analysis(cases[i]->caseBody);
        if(!caseBody.isSuccess()) {
          errorsOutput.addError("Error in line number: " + std::to_string(cases[i]->caseBody->lineNo) +
                                " .The case body of the case statement is not valid");
        }      
      }
      if(startingSize != errorsOutput.sizeError)
      {
        return Result::Error("error");
      }
      return Result::Success(varType);
    } 

    Result operator()(breakNodeType& br) { 
      // loops 
      
      return Result::Success("success"); 
    } // TODO

    Result operator()(enumNode& en) { 
      int startingSize = errorsOutput.sizeError;
      auto nameStr = std::get<idNodeType>(en.name->un).id;
      auto& enums = enumSymTable(nameStr, currentNodePtr->currentScope);
      auto& enumitr = enums[nameStr];

      //TODO change in scope
      if(enums.find(nameStr) != enums.end())
      {
        errorsOutput.addError("This enum is already declared , there is error in line "+ std::to_string(en.name->lineNo));
      }
        
      std::unordered_set<std::string> members;

      for (int i=0;i<en.enumMembers.size();i++)
      {
        members.insert(en.enumMembers[i]);

      }
      if(members.size() != en.enumMembers.size())
      {
        errorsOutput.addError("Enum has duplicate memebers in line "+std::to_string(en.name->lineNo) );
      }
      if(errorsOutput.sizeError!=startingSize)
      {
        return Result::Error("error");

      }
      else{
        enums.insert(make_pair(nameStr,en));
        for (int i=0;i<en.enumMembers.size();i++)
        {
          members.insert(en.enumMembers[i]);
          SymbolTableEntry entry = SymbolTableEntry();
          entry.type = "enum";
          entry.isConstant = false;
          entry.initExpr = nullptr;
          currentNodePtr->currentScope->sym2[nameStr+" "+en.enumMembers[i]] = entry; // TODO: Change this to enums table.
      }
         
        return Result::Success("enum");
      }
      return Result::Success("ok");
      
      } 

    Result operator()(enumUseNode& eu) { 
      //TODO add line numbers
      int startingSize = errorsOutput.sizeError;
      auto enums = enumSymTable(eu.enumName, currentNodePtr->currentScope);
      auto enumitr = enums.find(eu.enumName);

      if(enumitr==enums.end())
      {
        errorsOutput.addError("This enum is not defined, there is error in line ");
      }
      else
      {
        enumNode enumCurrent= enumitr->second;
        bool found=false;
        for(int i= 0;i< enumCurrent.enumMembers.size();i++)
        {
          if(enumCurrent.enumMembers[i]==eu.memberName)
          {
            found=true;
            break;
          }
        }
        if(!found)
        {
          errorsOutput.addError("This enum member is not defined, there is error in line ");

        }
        if(startingSize!=errorsOutput.sizeError)
        {
          // TODO: Make sure this is the proper scope.
          return Result::Success( currentNodePtr->currentScope->sym2[eu.enumName+" "+eu.memberName].type); // TODO: Probably change this to enums table.
        }
      }
      
      return Result::Error("error"); 
      } 

    /* For Enums (list of identifiers) */
    Result operator()(IdentifierListNode& il) { return Result::Success("success"); } // TODO

    Result operator()(functionNodeType& fn) { 
      /*
        TODO : add table , not existing , error
        body
        varaibles => scope , create table ,
        return type, return 

      */ 
      
      return Result::Success("success");
      
    } // TODO
    Result operator()(FunctionCall& fn) { 
      // TODO line number of this
      int startingSize =errorsOutput.sizeError;
      auto& functions = fnSymTable(fn.functionName, currentNodePtr->currentScope);
      auto function = functions.find(fn.functionName);

      std::string type=" ";
      if(function!= functions.end())
      {
        functionNodeType  currentFunction=function->second;
        if(fn.parameterExpressions.size() !=currentFunction.parameters.size())
        {
          errorsOutput.addError("The paramaters length doesn't match paramters of functions, you have"+std::to_string(fn.parameterExpressions.size())+"but it expected to be "+std::to_string(currentFunction.parameters.size()));
        }
        int min_size = std::min(currentFunction.parameters.size(),fn.parameterExpressions.size());
        for(int i=0;i< min_size;i++)
        {
          type = std::get<idNodeType>(currentFunction.parameters[i]->type->un).id;
          EXISTING_TYPE(type, currentFunction.parameters[i]->type->lineNo);
          auto functionParamter = semantic_analysis(fn.parameterExpressions[i]->exprCode);
          if(!functionParamter.isSuccess())
          {
            errorsOutput.addError("This error in "+std::to_string(i)+" experssion which provided in function call in line");
          }
          auto functionParamterType =std::get<SuccessType>(functionParamter);
          
          if(type!=functionParamterType)
          {
            errorsOutput.addError("This error in "+std::to_string(i)+" experssion which provided in function call in line , the types doesn't match");
          }
          
        }
      }
      else{
        errorsOutput.addError("This function doesn't exist , error in line" );
      }
      
      if(startingSize != errorsOutput.sizeError)
      {
       
        return Result::Error("error");
      }
      return Result::Success(type);
      
    } 
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
                              " .The condition in do while loop is not valid");
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
                              " .The body in do while loop is not valid");
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
                              " .The condition in while loop is not valid");
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
                              " .The body in while loop is not valid");
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
                              " .The initialization statement in the for loop is not valid");
      }
      
      auto condition = semantic_analysis(f.loop_condition);
      if (!condition.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(f.loop_condition->lineNo) +
                              " .The condition in the for loop is not valid");
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
                              " .The body in the for loop is not valid");
      }
      
      auto post_statement = semantic_analysis(f.post_loop_statement);
      if (!post_statement.isSuccess()){
        errorsOutput.addError("Error in line number: " +
                              std::to_string(f.post_loop_statement->lineNo) +
                              " .The post body statement in the for loop is not valid");
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
          auto symTable = opr.op[0]->currentScope;
          NOT_CONST(opr.op[0], opr.op[0]->lineNo, symTable->sym2); 
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
          auto symTable = opr.op[0]->currentScope;
          NOT_CONST(opr.op[0], opr.op[0]->lineNo,symTable->sym2); 
          /* Check that the left expression is valid (identifier is declared) */
          LEFT_VALID(opr.op[0]); // * gives left
          LEFT_TYPE(left);

          /* Manual casting without macro */
          if (leftType == "string") {
              errorsOutput.addError("Error in line number: " +
              std::to_string(opr.op[0]->lineNo) + " .Cannot cast to or from string");  
          } else if (leftType == "int" || leftType == "float") {
              return Result::Success(leftType);
          } else if (leftType == "char") {
              return Result::Success("int");
          } else {
              errorsOutput.addError("Error in line number: " +
              std::to_string(opr.op[0]->lineNo) + " .Can't Increment/Decrement booleans");
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
    if (p == nullptr) return Result::Success("success");
    return std::visit(semantic_analysis_visitor{p}, p->un);
}


