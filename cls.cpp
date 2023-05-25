#include <cstddef>
#include <functional>
#include <iterator>
#include <string>
#include <unordered_map>
#include <variant>
#include <algorithm>
#include <iostream>
#include <optional>
#include <unordered_set>
#include <set>
#include <vector>
#include <sstream>
#include <format>

#include "cl.h"
#include "y.tab.h"
#include "result.h"
#include "semantic_utils.h"
#include "template_utils.h"

/*
    TODO: return statements should be valid only inside functions.
    This can be done similarly to how break nodes are checked, we set the parent
   during parsing (can be done during semantic analysis too), then in semantic
   analysis, we check if the return node has a parent.
*/

#define LEFT_VALID(oper) \
    Result left = semantic_analysis(oper); \

#define RIGHT_VALID(oper) \
    Result right = semantic_analysis(oper); \

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

int getTypePriority(const std::string& type, nodeType* nodePtr) {
    static std::unordered_map<std::string, int> typesPriority = {
        {"string", 0}, {"bool", 1}, {"char", 2}, {"int", 3}, {"float", 5}
    };

    // If the given type is builtin
    if(typesPriority.find(type) != typesPriority.end()) { return typesPriority[type]; }

    // If it's not built in, might be an enum type
    return typesPriority["int"];
}

Result cast_opr(const std::string &leftType, const std::string &rightType, oprNodeType &opr) {
		int startingSize = errorsOutput.sizeError;
    if (leftType == "string" && rightType == "string") {
        if (opr.oper == '=') {
            return Result::Success("string");
        } else if (opr.oper == EQ or opr.oper == NE) {
            return Result::Success("bool");
        } else {
            errorsOutput.addError(
                "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
                " .Cannot perform this operation on strings");
        }
    } else if (leftType == "string" || rightType == "string") {
        errorsOutput.addError(
            "Error in line number: " + std::to_string((opr.op[0]->lineNo)) +
            " .Cannot cast to or from string");
    }
    switch (opr.oper) {
    case '=': {
        const std::string& castResult = leftType;
        if (startingSize == errorsOutput.sizeError) {
            if (rightType != castResult) {
                opr.op[1]->conversionType = castResult;
            }
        } else {
            return Result::Error("Error");
        }

        return Result::Success(castResult);
    } break;
    case '+':
    case '-':
    case '*':
    case '/': {
        std::string castResult =
            leftType == "float" || rightType == "float" ? "float" : "int";
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
    } break;
    case '%': {
        std::string castResult = "int";
        if (leftType == "float" || leftType == "string") {
            errorsOutput.addError(
                "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
                " .The LHS in a modulo operation can't be " + leftType);
        }
        if (rightType == "float" || rightType == "string") {
            errorsOutput.addError(
                "Error in line number: " + std::to_string(opr.op[1]->lineNo) +
                " .The RHS in a modulo operation can't be " + rightType);
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
    } break;
    case '&':
    case '|':
    case '^':
    case LS:
    case RS: {
        std::string castResult = "int";
        if (leftType == "float" || leftType == "string") {
            errorsOutput.addError(
                "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
                " .The LHS in a bitwise operation can't be " + leftType);
        }
        if (rightType == "float" || rightType == "string") {
            errorsOutput.addError(
                "Error in line number: " + std::to_string(opr.op[1]->lineNo) +
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
    } break;
    case AND:
    case OR: {
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
    } break;
    case GE:
    case LE:
    case '>':
    case '<':
    case EQ:
    case NE: {
        std::string castResult = "bool";

        if (startingSize == errorsOutput.sizeError) {
            // ASSUMES  that left is op[0], right is op[1]
            auto& lOpr = opr.op[0];
            auto& rOpr = opr.op[1];

            int leftPriority = getTypePriority(leftType, lOpr);
            int rightPriority = getTypePriority(rightType, rOpr);

            // Note that when a node's conversionType is different from it's
            // type (for idNodes for example), we emit a convTypeToVarType
            // instruction.
            // for example, if a node has a type of `int` and a convType of
            // `float`, it will emit `floatToInt`
            if(leftPriority > rightPriority) {
                rOpr->conversionType = leftType;
            } else {
                lOpr->conversionType = rightType;
            }

        } else {
            return Result::Error("Error");
        }
        return Result::Success(castResult);
    } break;
    default: {
        return Result::Error(
            "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
            " .Invalid operator");
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

void getEnumTypes(ScopeSymbolTables* p, std::unordered_set<std::string>& enumTypes) {
    if (p == nullptr) return;

    for(const auto& [enumName, _]: p->enums) {
        enumTypes.insert(enumName);
    }

    getEnumTypes(p->parentScope, enumTypes);
}

#define EXISTING_TYPE(type, lineNo)                                            \
    static std::unordered_set<std::string> builtinTypes = {                    \
        "float", "int", "char", "string", "bool", "void"};                     \
    std::unordered_set<std::string> enumTypes;                                 \
    getEnumTypes(currentNodePtr->currentScope, enumTypes);                     \
    if (type != "<no type>" &&builtinTypes.find(type) == builtinTypes.end() && \
               enumTypes.find(type) == enumTypes.end()) {                      \
        errorsOutput.addError(                                                 \
            "Error in line number: " + std::to_string(lineNo) +                \
            " .The data type \"" + type + "\" is not valid");                  \
    }


Result ex_const_kak_TM(nodeType *p);

struct ex_const_visitor {
    nodeType* currentNodePtr;

    Result operator()(conNodeType& con) { 
        Value v = std::visit(
                  Visitor {
                      [](int iValue)         { return Value(iValue); },
                      [](bool bValue)        { return Value(bValue); },
                      [](char cValue)        { return Value(cValue); },
                      [](float fValue)       { return Value(fValue); },
                      [](std::string sValue) { return Value(sValue); },
                  },
                  con
          );
        if (std::holds_alternative<int>(v)) {
             Result result = Result::Success("int");
             result.value = new Value(std::get<int>(v));
             return result;
        } else if (std::holds_alternative<float>(v)) {
             Result result = Result::Success("float");
             result.value = new Value(std::get<float>(v));
             return result;
        } else if (std::holds_alternative<char>(v)) {
             Result result = Result::Success("char");
             result.value = new Value(std::get<char>(v));
             return result;
        } else if (std::holds_alternative<std::string>(v)) {
             Result result = Result::Success("string");
             result.value = new Value(std::get<std::string>(v));
             return result;
        } else if (std::holds_alternative<bool>(v)) {
             Result result = Result::Success("bool");
             result.value = new Value(std::get<bool>(v));
             return result;
        } else {
            return Result::Error("Error");
        }
    }


    Result operator()(idNodeType& id) const {
        auto* idScope = getSymbolScope(id.id, currentNodePtr->currentScope);

        if(idScope != nullptr) {
            auto& entry = idScope->sym2[id.id];
            auto isConstant = entry.isConstant;
            auto hasInit = entry.initExpr != nullptr;
            auto* literalInit  = entry.initExpr->asPtr<conNodeType>();

            if (literalInit != nullptr && isConstant && hasInit) {
                Result result = Result::Success(entry.type);
                result.value = &entry.value;
                return result;
            } 
        }

        return Result::Error("Error");
    }

    Result operator() (oprNodeType& opr) {
        switch (opr.oper) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%': {
            auto left = ex_const_kak_TM(opr.op[0]);
            auto right = ex_const_kak_TM(opr.op[1]);

            if (!left.isSuccess()) {
                return left;
            }
            if (!right.isSuccess()) {
                return right;
            }

            std::string finalType = std::get<SuccessType>(left) == "float" ? "float"
                                                                           : "int";
            finalType = std::get<SuccessType>(right) == "float" ? "float" : finalType;

            if (opr.oper == '+') {
                Result result = Result::Success(finalType);
                result.value = new Value(*left.value + *right.value);
            }

            if (opr.oper == '-') {
                Result result = Result::Success(finalType);
                result.value = new Value(*left.value - *right.value);
            }

            if (opr.oper == '*') {
                Result result = Result::Success(finalType);
                result.value = new Value(*left.value * *right.value);
            }

            if (opr.oper == '/') {
                Result result = Result::Success(finalType);
                result.value = new Value(*left.value / *right.value);
            }

            if (opr.oper == '%') {
                Result result = Result::Success("int");
                result.value = new Value(*left.value % *right.value);
            }

            return Result::Error("");
        } break;

        case '&':
        case '|':
        case '^':
        case '~':
        case LS:
        case RS: {
            auto left = ex_const_kak_TM(opr.op[0]);
            auto right = ex_const_kak_TM(opr.op[1]);

            if (!left.isSuccess()) { return left; }
            if (!right.isSuccess()) { return right; }

            if (opr.oper == '&') {
                Result result = Result::Success(std::get<SuccessType>(left));
                result.value = new Value(*left.value & *right.value);
            }

            if (opr.oper == '|') {
                Result result = Result::Success(std::get<SuccessType>(left));
                result.value = new Value(*left.value | *right.value);
            }

            if (opr.oper == '^') {
                Result result = Result::Success(std::get<SuccessType>(left));
                result.value = new Value(*left.value ^ *right.value);
            }

            if (opr.oper == '~') {
                Result result = Result::Success(std::get<SuccessType>(left));
                result.value = new Value(~(*left.value));
            }

            if (opr.oper == LS) {
                Result result = Result::Success(std::get<SuccessType>(left));
                result.value = new Value(*left.value << *right.value);
            }

            if (opr.oper == RS) {
                Result result = Result::Success(std::get<SuccessType>(left));
                result.value = new Value(*left.value >> *right.value);
            }


            return Result::Error("");
        } break;

        case '<':
        case '>':
        case EQ:
        case NE:
        case GE:
        case LE: {
            auto left = ex_const_kak_TM(opr.op[0]);
            auto right = ex_const_kak_TM(opr.op[1]);

            if (!left.isSuccess()) { return left; }
            if (!right.isSuccess()) { return right; }

            if (opr.oper == '<') {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value < *right.value);
                return result;
            }

            if (opr.oper == '>') {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value > *right.value);
                return result;
            }

            if (opr.oper == EQ) {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value == *right.value);
                return result;
            }

            if (opr.oper == NE) {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value != *right.value);
                return result;
            }

            if (opr.oper == GE) {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value >= *right.value);
                return result;
            }

            if (opr.oper == LE) {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value <= *right.value);
                return result;
            }

            return Result::Error("");
        }

        case AND:
        case OR: {
            auto left = ex_const_kak_TM(opr.op[0]);
            auto right = ex_const_kak_TM(opr.op[1]);

            if (!left.isSuccess()) { return left; }
            if (!right.isSuccess()) { return right; }
            
            if (opr.oper == AND) {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value && *right.value);
                return result;
            }

            if (opr.oper == OR) {
                Result result = Result::Success("bool");
                result.value = new Value(*left.value || *right.value);
                return result;
            }

            return Result::Error("");
        }
        default: { return Result::Error("Unknown operator"); }
        }
    }

    // the default case:
    template<typename T> 
    Result operator()(T const & /*UNUSED*/ ) const { return Result::Success(""); }  
};

Result ex_const_kak_TM(nodeType *p) {    
    if (p == nullptr) return Result::Success("success");
    return std::visit(ex_const_visitor{p}, p->un);
}


struct semantic_analysis_visitor {
    nodeType* currentNodePtr;

    Result operator()(conNodeType& con) { 
        return Result::Success(con.getType());
    }

    Result operator()(VarDecl& vd) const {
        /*
            TODO: Don't allow the declaration of a variable with the same name as a function
        */

        int startingSize = errorsOutput.sizeError;
        /* Check that the type of the variable is valid */
        auto type = std::get<idNodeType>(vd.type->un).id;

        /* Check that the type & name are valid */

        EXISTING_TYPE(type, vd.type->lineNo);
        
        auto* symTable = currentNodePtr->currentScope;
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
          entry.isConstant = vd.isConstant;
          entry.initExpr = vd.initExpr;
          entry.declaredAtLine = vd.type->lineNo;
          entry.isUsed = false;
          symTable->sym2[nameStr] = entry;
          return Result::Success(entry.type);
        }
        return Result::Error("error");
    }

    Result operator()(VarDefn& vd) const {
      int startingSize = errorsOutput.sizeError;
      /* Check if the variable is declared */

      nodeType *nt = new nodeType(VarDecl(vd.decl->type, vd.decl->var_name, vd.initExpr, vd.isConstant), vd.decl->type->lineNo);
      nt->currentScope = currentNodePtr->currentScope;
      Result decl = semantic_analysis(nt); 

      std::string declType = decl.isSuccess() ? (std::string)std::get<SuccessType>(decl) : "<no type>";

      /* Check that the initial expression is valid if it exits */
      Result initResult;
      if (vd.initExpr != nullptr) {
        initResult = semantic_analysis(vd.initExpr);
      } else {
        /* The initialization expression doesn't exist*/
        if (vd.isConstant) {
              errorsOutput.addError("Error in line number: " +
              std::to_string(vd.decl->var_name->lineNo) + " .The constant variable " +
              std::get<idNodeType>(vd.decl->var_name->un).id + " has no value assigned to it");
        }

        if(startingSize != errorsOutput.sizeError) { return Result::Error("error"); }
      }

      if(initResult.isSuccess()) {
        vd.initExpr->conversionType = declType;
      }

      return Result::Success(declType);
    }
    

		// Check if the given identifier exists in the current or preceeding scopes.
    Result operator()(idNodeType& identifier) const { 
				int startingSize = errorsOutput.sizeError;
				auto* symTable = getSymbolScope(identifier.id, currentNodePtr->currentScope);

				if(symTable == nullptr) {
						/* If not, then it's not in any scope. */
						errorsOutput.addError("Error in line number: " + std::to_string(currentNodePtr->lineNo) + " .Identifier " + identifier.id + " is not declared");
						return Result::Error("error");
				}

				EXISTING_TYPE(symTable->sym2[identifier.id].type, currentNodePtr->lineNo);
				symTable->sym2[identifier.id].isUsed = true;

				return Result::Success(symTable->sym2[identifier.id].type);
		}

    Result operator()(caseNodeType& cs) {
        auto bodyResult = semantic_analysis(cs.caseBody);
        auto labelResult = semantic_analysis(cs.labelExpr);

        if(bodyResult.isSuccess() && labelResult.isSuccess()) return Result::Success("");

        if(!bodyResult.isSuccess()) errorsOutput.mergeErrors(std::get<ErrorType>(bodyResult));
        if(!labelResult.isSuccess()) errorsOutput.mergeErrors(std::get<ErrorType>(labelResult));

        return Result::Error("");
    } 

    // Checks if there are duplicate case identifiers, and if all case labels
    // evaluate to the same type.
    Result operator()(switchNodeType &sw) const {
        int startingSize = errorsOutput.sizeError;
        auto var = semantic_analysis(sw.var);

        if (!var.isSuccess()) {
            errorsOutput.addError(
                "Error in line number: " + std::to_string(sw.var->lineNo) +
                " .The switch variable is not valid"
            );
        }

        auto varType = std::get<SuccessType>(var);
        auto cases = sw.caseListTail->as<caseNodeType>().toVec();

        for (int i = 0; i < cases.size(); i++) {
            auto labelExprResult = semantic_analysis(cases[i]->labelExpr);

            /* Label Expression must be of the same type as the switch variable
             * & must be a constant or a literal */
            if (!labelExprResult.isSuccess()) {
                errorsOutput.addError(
                    "Error in line number: " +
                    std::to_string(cases[i]->labelExpr->lineNo) +
                    " .The label expression of the case statement is not valid"
                );
                continue;
            }

            // Default cases have no labelExpr. So we don't need to check them.
            // For a labelExpression to be true, it has to use constant or a
            // literal.
            if (!cases[i]->isDefault()) {

                std::string labelExprType =
                    std::get<SuccessType>(labelExprResult);

                if (varType != "<no type>" && labelExprType != varType) {
                    auto lineNo = std::to_string(cases[i]->labelExpr->lineNo);
                    errorsOutput.addError(
                        "Error in line number: " + lineNo +
                        " .The label expression of the case statement is not "
                        "of the same type as the switch variable"
                    );
                }

                bool isConstant = false;
                bool isLiteral = false;
                auto *labelExpr = cases[i]->labelExpr;

                if (labelExpr->is<idNodeType>()) {

                    auto &labelName = labelExpr->as<idNodeType>().id;
                    auto &labelSymTableEntry =
                        getSymEntry(labelName, currentNodePtr->currentScope);
                    isConstant = labelSymTableEntry.isConstant;

                } else if (labelExpr->is<conNodeType>() || labelExpr->is<enumUseNode>()) {
                    isLiteral = true;
                }

                if (!isConstant && !isLiteral) {
                    errorsOutput.addError(
                        "Error in line number: " +
                        std::to_string(cases[i]->labelExpr->lineNo) +
                        " .The label expression of the case statement must be "
                        "a constant or a literal"
                    );
                }
            }

            auto caseBody = semantic_analysis(cases[i]->caseBody);
            if (!caseBody.isSuccess()) {
                errorsOutput.addError(
                    "Error in line number: " +
                    std::to_string(cases[i]->caseBody->lineNo) +
                    " .The case body of the case statement is not valid"
                );
            }
        }

        if (startingSize != errorsOutput.sizeError) {
            return Result::Error("error");
        }

        return Result::Success(varType);
    }

    Result operator()(breakNodeType& br) const { 
        if(br.parent_switch == nullptr) {
            auto lineNo = std::to_string(currentNodePtr->lineNo);
            errorsOutput.addError("Error at line: " + lineNo + ". Break statements are valid inside for loops, while loops, or do while loops only." );

            return Result::Error("");
        }
        return Result::Success("success"); 
    } 

    Result operator()(enumNode& en) const { 

      /* Check that an enum with the same name is not declared already */
      std::string enumName = en.name->as<idNodeType>().id;
			auto* enumScope = getSymbolScope(enumName, currentNodePtr->currentScope);

			// If the enum can be found in the current scope or the ones before it, register an error.
			if(enumScope != nullptr) {
        errorsOutput.addError("Error in line number: " + std::to_string(en.name->lineNo) +
                              " .The enum " + enumName + " is already declared");
			}

      /* Check that all members are unique */
      std::unordered_set<std::string> members(en.enumMembers.begin(), en.enumMembers.end());

      if(members.size() != en.enumMembers.size()) {
        errorsOutput.addError("Enum has duplicate memebers in line "+std::to_string(en.name->lineNo) );
      }
    
      /* Add the enum to the enums table */
      currentNodePtr->currentScope->enums[enumName] = en;
      
      return Result::Success("ok");
    } 

    Result operator()(enumUseNode& eu) const { 
      
      int startingSize = errorsOutput.sizeError;
      
      /* Check that the enum is declared */
      auto* scope = getSymbolScope(eu.enumName, currentNodePtr->currentScope);

      if(scope == nullptr) {
					errorsOutput.addError("Error in line number: " + std::to_string(eu.lineNo) +
									" .The enum " + eu.enumName + " is not declared");
      } else {
        auto enumMembers = scope->enums[eu.enumName].enumMembers;
        /* Check that the member name is part of the enum members */
        if (std::find(enumMembers.begin(), enumMembers.end(), eu.memberName) == enumMembers.end()) {
						errorsOutput.addError("Error in line number: " + std::to_string(eu.lineNo) +
										" .The enum member " + eu.memberName + " is not part of the enum " + eu.enumName);
        }
      }

      if (startingSize != errorsOutput.sizeError) {
        return Result::Error("error");
      }

      return Result::Success(eu.enumName);
    }

    Result operator()(functionNodeType &fn) const {

        int startingSize = errorsOutput.sizeError;

        /* Function Declaration code */
        auto functionName = fn.name->as<idNodeType>().id;
        auto *functionScope =
            getSymbolScope(functionName, currentNodePtr->currentScope);

        // Already declared.
        if (functionScope != nullptr) {
            errorsOutput.addError(lineError(
                "Function '%s' is already declared in this scope",
                fn.name->lineNo,
                functionName
            ));
        }

        /* Add the new function to the functions table */
        // Since functions create their own scope, need to add
        // the function reference to the parent scope.
        currentNodePtr->currentScope->parentScope
            ->functions[std::get<idNodeType>(fn.name->un).id] = fn;

        /* Check if the function parameters are valid */
        auto parameters = fn.parametersTail->toVec();
        for (int i = 0; i < parameters.size(); i++) {
            auto *param = parameters[i];

            /* Check if the parameter list is emtpy */
            if (param->type == nullptr || param->var_name == nullptr) {
                break;
            }

            nodeType *nt = new nodeType(
                VarDecl(param->type, param->var_name), param->type->lineNo
            );
            nt->currentScope = currentNodePtr->currentScope;
            auto parameter = semantic_analysis(nt);

            if (!parameter.isSuccess()) {
                errorsOutput.addError(lineError(
                    "The function parameter '%s' is not valid",
                    param->type->lineNo,
                    std::get<idNodeType>(param->var_name->un).id 
                ));
            }

            // Add parameter to the function's symbol table.
            auto paramName =
                parameters[i]->var_name->as<idNodeType>().id;
            auto paramType = parameters[i]->type->as<idNodeType>().id;

            currentNodePtr->currentScope->sym2[paramName] =
                SymbolTableEntry(parameters[i]->isConstant, paramType);
        }

        /* Check if the function body is valid */
        auto functionBody = semantic_analysis(fn.statements);
        if (!functionBody.isSuccess()) {
            errorsOutput.addError(lineError(
                "The function body is not valid", fn.statements->lineNo
            ));
        }
        /* Check if the return type is valid */
        auto returnType = fn.return_type->as<idNodeType>().id;
        EXISTING_TYPE(returnType, fn.return_type->lineNo);
        if (errorsOutput.sizeError != startingSize) {
            errorsOutput.addError(lineError(
                "The function return type '%s' is not valid",
                fn.return_type->lineNo,
                returnType
            ));
        }

        auto innerReturnStatements = getFnReturnStatements(fn.statements);

        // Need to perserve the order, hence no unordered_set
        auto innerReturnTypes = std::set<std::string>();
        for (auto *ret : innerReturnStatements) {
            innerReturnTypes.insert(getReturnType(ret));
        }

        auto returnLines = collectString<std::vector<nodeType *>>(
            innerReturnStatements.begin(),
            innerReturnStatements.end(),
            [](auto iter) { return std::to_string(iter->lineNo); }
        );

        // Cases to check
        //  - Function with no returns but the return type is NOT void
        //  - Function with returns but the return type is void
        //  - Function with multiple return types

        if (returnType == "void") {
            if(!innerReturnTypes.empty()) {
                auto returnTypes = collectString<std::set<std::string>>(innerReturnTypes.begin(), innerReturnTypes.end());
                errorsOutput.addError(
                    fmt("Error at line(s): %s. Void functions cannot return "
                        "types (%s).",
                        returnLines,
                        returnTypes)
                );
            }
        } else {
            if (innerReturnTypes.empty()) {
                // No return types
                errorsOutput.addError(
                    fmt("Error at lines(s): %s. Function with return type '%s' "
                        "has no returns.",
                        returnLines,
                        returnType)
                );
            } else if (innerReturnTypes.size() != 1) {
                // Multiple return types

                auto returnTypes = collectString<std::set<std::string>>(innerReturnTypes.begin(), innerReturnTypes.end());
                errorsOutput.addError(
                    fmt("Error at line(s): %s. Mismatched return statements "
                        "(%s) in function with return type '%s'.",
                        returnLines,
                        returnTypes,
                        returnType)
                );

            } else if (innerReturnTypes.size() == 1) {
                // Just one return type
                // Need to make sure that the type of the return is castable to
                // the function's return type.
                auto innerType = *innerReturnTypes.begin();

                auto castInnerReturnTypes =
                    castToTarget(innerType, returnType, currentNodePtr,
                                 currentNodePtr->lineNo);

                if (!castInnerReturnTypes.isSuccess()) {
                    errorsOutput.addError(lineError(
                        "Cannot return type '%s' from a "
                        "function with return type '%s'. ",
                        currentNodePtr->lineNo,
                        innerType,
                        returnType
                    ));
                }
            }
        }

        if (startingSize != errorsOutput.sizeError) {
            return Result::Error("error");
        }
        return Result::Success(returnType);
    }

        // Checks if the function has already been declared
		// Checks if the function call arguments have the same count and type as the parameters.
    Result operator()(FunctionCall& fc) const { 
        int startingSize = errorsOutput.sizeError;

        /* Check if the function exists in the functions table or not */
        auto* fnScope = getSymbolScope(fc.functionName, currentNodePtr->currentScope); 
        auto callArgExprs = fc.parameterExpressions;
        std::vector<VarDecl*> fnParams;

        // Function can't be found in current or preceeding scopes => not declared.
        if(fnScope == nullptr) {
            errorsOutput.addError("Error in line number: " + 
                    std::to_string(fc.lineNo) + " .The function "
                    + fc.functionName + " is not declared");
        } else {
            auto &fnRef = fnScope->functions[fc.functionName];
            fnParams = fnRef.parametersTail->toVec();

            if (fnParams.size() != callArgExprs.size()) {
                errorsOutput.addError(
                    "Error in line number: " + std::to_string(fc.lineNo) +
                    ". The number of arguments to the function call "
                    "doesn't match the number of parameters in the "
                    "function definition");
            }
        }

        /* Check if the Expression List is valid */
        for(int i = 0; i < fc.parameterExpressions.size(); i++) {
            auto* arg = callArgExprs[i];
            /* Check if the Expression list is emtpy */
            if (arg->exprCode == nullptr) { break; } 

            /* Check that the parameter types match with the function types */
            auto expression = semantic_analysis(arg->exprCode);
            if (!expression.isSuccess()) {
                errorsOutput.addError("Error in line number: " +
                        std::to_string(arg->exprCode->lineNo) + 
                        " .The function parameter number: " + std::to_string(i) + " is not valid");
            } else {
                if (fnScope != nullptr) {
                    auto *param = fnParams[i];

                    // Check if the passed  expression matches the parameter's
                    // type
                    auto paramType = param->type->as<idNodeType>().id;
                    auto exprType = std::get<SuccessType>(expression);

                    auto paramExprTypeMatchesParamType =
                        (paramType == exprType);
                    Result exprIsCastable = castToTarget(
                        exprType, paramType, arg->exprCode, fc.lineNo);

                    if (!exprIsCastable.isSuccess()) {
                        errorsOutput.addError(
                            "Error in line number: " +
                            std::to_string(arg->exprCode->lineNo) +
                            " . The function parameter number: " +
                            std::to_string(i) + " type does not match");
                    }
                }
            }
        }

        if (startingSize != errorsOutput.sizeError) {
            return Result::Error("error");
        }

        auto fnReturnType = fnScope->functions[fc.functionName].return_type->as<idNodeType>().id;
        return Result::Success(fnReturnType);
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
        
        for(const auto& [name, entry]: sl.statementCode->currentScope->sym2) {
            if(!entry.isUsed) {
                warningsOutput.addError("Warning in line number: " + std::to_string(entry.declaredAtLine) +
                        " .Variable " + name + " is declared but never used");
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
            auto conditionType = std::get<SuccessType>(condition);
            if (conditionType != "bool") {
                Result conditionCastResult = castToTarget(conditionType, "bool", dw.condition, dw.condition->lineNo);

                if (!conditionCastResult.isSuccess()) {
                    errorsOutput.mergeErrors(std::get<ErrorType>(conditionCastResult));
                }
            }
        }

        auto loop = semantic_analysis(dw.loop_body); 
        if (!loop.isSuccess()){
            errorsOutput.addError("Error in line number: " +
                    std::to_string(dw.loop_body->lineNo) +
                    " .The body in do while loop is not valid");
        } 

        if(startingSize != errorsOutput.sizeError) { return Result::Error("error"); }
        return Result::Success(std::get<SuccessType>(condition));
    }

    Result operator()(whileNodeType& w) { 
        int startingSize = errorsOutput.sizeError;
        auto condition = semantic_analysis(w.condition);
        if (!condition.isSuccess()){
            errorsOutput.addError("Error in line number: " +
                    std::to_string(w.condition->lineNo) +
                    " .The condition in while loop is not valid");
        } else {
            std::string conditionType = std::get<SuccessType>(condition);
            if (conditionType != "bool") {
                Result conditionCastResult = castToTarget(conditionType, "bool", w.condition, w.condition->lineNo);
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

        if(startingSize != errorsOutput.sizeError) { return Result::Error("error"); }
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
                Result conditionCastResult = castToTarget(conditionType, "bool", f.loop_condition, f.loop_condition->lineNo);

                if (!conditionCastResult.isSuccess()) {
                    errorsOutput.mergeErrors(std::get<ErrorType>(conditionCastResult));
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

        if(startingSize != errorsOutput.sizeError) { return Result::Error("error"); }             
        return Result::Success(std::get<SuccessType>(condition));
    }
           

    Result operator()(oprNodeType &opr) const {
        /* Things that need to be checked */
        /*
            TODO: Implement the followinging check when the function identifier is added
           to the symbol table
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
            auto *symTable = opr.op[0]->currentScope;
            NOT_CONST(opr.op[0], opr.op[0]->lineNo, symTable->sym2);

            /* Check that the right expression is semantically valid */
            RIGHT_VALID(opr.op[1]); // * gives right
                                    //
            /*  Check that the two expressions on the left & on the right are of the
             * same type */
            LEFT_SAME_TYPE_AS_RIGHT(
                left, right, opr.op[0]->lineNo,
                opr); // * gives leftType & rightType & finalType

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(finalType);
        } break;

            /* Mathematical Operators */

        case PP:
        case MM: {
            /* Check that the left identifier is not a constant */
            auto *symTable = opr.op[0]->currentScope;
            NOT_CONST(opr.op[0], opr.op[0]->lineNo, symTable->sym2);

            /* Check that the left expression is valid (identifier is declared) */
            LEFT_VALID(opr.op[0]); // * gives left
            LEFT_TYPE(left);

            /* Manual casting without macro */
            if (leftType == "string") {
                errorsOutput.addError(
                    "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
                    " .Cannot cast to or from string");
            } else if (leftType == "int" || leftType == "float") {
                return Result::Success(leftType);
            } else if (leftType == "char") {
                return Result::Success("int");
            } else {
                errorsOutput.addError(
                    "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
                    " .Can't Increment/Decrement booleans");
            }

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(leftType);
        } break;

        case AND:
        case OR: {
            /* Check that the left expression is valid (identifier is declared) */
            LEFT_VALID(opr.op[0]); // * gives left
                                   //
            /* Check that the right expression is semantically valid */
            RIGHT_VALID(opr.op[1]); // * gives right
                                    //
            /*  Check that the two expressions on the left & on the right are of the
             * same type */
            LEFT_SAME_TYPE_AS_RIGHT(
                left, right, opr.op[0]->lineNo,
                opr); // * gives leftType & rightType & finalType
            /* Check that the left and right are bool */

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(finalType);

        } break;

        case '+':
        case '-':
        case '*':
        case '/':
        case '%': {
            /* Check that the left expression is valid (identifier is declared) */
            LEFT_VALID(opr.op[0]); // * gives left

            /* Check that the right expression is semantically valid */
            RIGHT_VALID(opr.op[1]); // * gives right

            /*  Check that the two expressions on the left & on the right are of the
             * same type */
            LEFT_SAME_TYPE_AS_RIGHT(
                left, right, opr.op[0]->lineNo,
                opr); // * gives leftType & rightType & finalType

            auto setIdNodeAsUsed =
                [currentScope = currentNodePtr->currentScope](nodeType *ptr) {
                    ptr->currentScope = currentScope;

                    if (auto *lhsVar = ptr->asPtr<idNodeType>(); lhsVar) {
                        auto lhsVarSymEntry =
                            getSymEntry(lhsVar->id, ptr->currentScope);
                        lhsVarSymEntry.isUsed = true;
                    }
                };

            /* Check that the left and right are either both integers or both float
             */
            setIdNodeAsUsed(opr.op[0]);
            setIdNodeAsUsed(opr.op[1]);

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(finalType);
        } break;

        case GE:
        case LE:
        case '>':
        case '<': {
            /* Check that the left expression is valid (identifier is declared) */
            LEFT_VALID(opr.op[0]); // * gives left

            /* Check that the right expression is semantically valid */
            RIGHT_VALID(opr.op[1]); // * gives right

            /*  Check that the two expressions on the left & on the right are of the
             * same type */
            LEFT_SAME_TYPE_AS_RIGHT(
                left, right, opr.op[0]->lineNo,
                opr); // * gives leftType & rightType & finalType

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(finalType);
        } break;

        case EQ:
        case NE: {
            /* Check that the left expression is valid (identifier is declared) */
            LEFT_VALID(opr.op[0]); // * gives left

            /* Check that the right expression is semantically valid */
            RIGHT_VALID(opr.op[1]); // * gives right

            /*  Check that the two expressions on the left & on the right are of the
             * same type */
            LEFT_SAME_TYPE_AS_RIGHT(
                left, right, opr.op[0]->lineNo,
                opr); // * gives leftType & rightType & finalType

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(finalType);
        } break;

        case '&':
        case '^':
        case '|':
        case LS:
        case RS: {
            /* Check that the left expression is valid (identifier is declared) */

            LEFT_VALID(opr.op[0]); // * gives left

            /* Check that the right expression is semantically valid */
            RIGHT_VALID(opr.op[1]); // * gives right

            LEFT_SAME_TYPE_AS_RIGHT(
                left, right, opr.op[0]->lineNo,
                opr); // * gives leftType & rightType & finalType

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(finalType);
        } break;

        case PRINT: {
            /* Check that the right expression is valid */
            RIGHT_VALID(opr.op[0]); // * gives right
            return right;
        } break;

        case IF: {
            Result condition = semantic_analysis(opr.op[0]);

            if (!condition.isSuccess()) {
                errorsOutput.addError(
                    "Error in line number: " + std::to_string(opr.op[0]->lineNo) +
                    " .Condition in an if statement is invalid");
            } else {
                auto conditionType = std::get<SuccessType>(condition);

                if (conditionType != "bool") {
                    Result conditionCastResult = castToTarget(
                        conditionType, "bool", opr.op[0], opr.op[0]->lineNo);
                    if (!conditionCastResult.isSuccess()) {
                        errorsOutput.mergeErrors(
                            std::get<ErrorType>(conditionCastResult));
                    }
                }
            }

            /* Check if the case condition is always false */
            if (condition.isSuccess()) {
                Result alwaysFalseResult = ex_const_kak_TM(opr.op[0]);

                if (alwaysFalseResult.isSuccess() &&
                    std::get<SuccessType>(alwaysFalseResult) == "bool") {
                    Value val = *alwaysFalseResult.value;

                    if (std::get<bool>(val) == false) {
                        warningsOutput.addError(
                            "Warning in line number: " +
                            std::to_string(opr.op[0]->lineNo) +
                            " .Condition in an if statement is always false");
                    }
                }
            }

            Result ifBody = semantic_analysis(opr.op[1]);
            if (!ifBody.isSuccess()) {
                errorsOutput.addError(
                    "Error in line number: " + std::to_string(opr.op[1]->lineNo) +
                    " .Body of an if statement is invalid");
            }

            if (opr.op.size() > 2) {
                Result elseBody = semantic_analysis(opr.op[2]);
                if (!ifBody.isSuccess()) {
                    errorsOutput.addError("Error in line number: " +
                                          std::to_string(opr.op[1]->lineNo) +
                                          " .Body of an if statement is invalid");
                }
            }

            if (startingSize != errorsOutput.sizeError) {
                return Result::Error("error");
            }
            return Result::Success(std::get<SuccessType>(condition));
        } break;

        case RETURN: {
            if (!opr.op.empty())
                return semantic_analysis(opr.op[0]);
        } break;

        case UMINUS:
        case UPLUS: { return semantic_analysis(opr.op[0]); } break;

        default: return Result::Success("success");
        }

        if (startingSize != errorsOutput.sizeError) {
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
