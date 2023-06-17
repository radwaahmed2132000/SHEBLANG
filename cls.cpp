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

#include "cl.h"
#include "node.h"
#include "nodes.h"
#include "parser.h"
#include "result.h"
#include "semantic_utils.h"
#include "template_utils.h"
#include "value.h"
#include "const_expr_checker.h"

using ErrorList = std::vector<std::string>;
using SemResult = Result<Type, ErrorList>;
using namespace std::string_literals;

/*
    TODO: return statements should be valid only inside functions.
    This can be done similarly to how break nodes are checked, we set the parent
   during parsing (can be done during semantic analysis too), then in semantic
   analysis, we check if the return node has a parent.
*/

#define LEFT_VALID(oper) SemResult left = semanticAnalysis(oper);

#define RIGHT_VALID(oper) SemResult right = semanticAnalysis(oper);

#define NOT_CONST(oper, lineNo, sym2)                                          \
    auto lhsName = oper->as<idNodeType>().id;                                  \
    if (sym2.find(lhsName) != sym2.end()) {                                    \
        auto &lEntry = sym2[lhsName];                                          \
        if (lEntry.isConstant) {                                               \
            errors.push_back(                                                  \
                "Error in line number: " + std::to_string(lineNo) +            \
                " The LHS of an assignment operation is constant."             \
            );                                                                 \
        }                                                                      \
    }

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
SemResult castToTarget(Type currentType, Type targetType, Node* currentNode, int lineNo) {
  if (currentType != targetType && (currentType == "string" || targetType == "string")) {
    return SemResult::err("Error in line number: " + std::to_string(lineNo) + " .Cannot cast to or from string");
  }

  if (currentType.isArray == Type::IsArray::Yes || targetType.isArray == Type::IsArray::Yes) {
      if (currentType == targetType) return SemResult::ok(targetType);
      else
          return SemResult::err(lineError(
              "Mismatched types for arrays (%s, %s)",
              currentNode->lineNo,
              std::string(targetType),
              std::string(currentType)
          ));
  }

  if (currentType == targetType) {
    return SemResult::ok(targetType);
  } else {
    currentNode->conversionType = targetType.innerType;
    return SemResult::ok(targetType);
  }
}

int getTypePriority(const Type& type, Node* nodePtr) {
    static std::unordered_map<std::string, int> typesPriority = {
        {"string", 0}, {"bool", 1}, {"char", 2}, {"int", 3}, {"float", 5}
    };

    // If the given type is builtin
    if(typesPriority.find(type.innerType) != typesPriority.end()) { return typesPriority[type.innerType]; }

    // If it's not built in, might be an enum type
    return typesPriority["int"];
}

SemResult castBinOp(const Type& leftType, const Type& rightType, BinOp& bop) {
    using enum BinOper;

    int startingSize = errors.size();

    if (leftType == "string" && rightType == "string") {
        if (bop.op == Assign) {
            return SemResult::ok("string"s);
        } else if (bop.op == Equal or bop.op == NotEqual) {
            return SemResult::ok("bool"s);
        } else {
            errors.push_back(lineError(
                "Cannot perform operation '%s' on strings.",
                bop.lOperand->lineNo,
                bop
            ));
        }
    } else if (leftType == "string" || rightType == "string") {
        errors.push_back(
            lineError("Cannot cast to or from 'string'.", bop.lOperand->lineNo)
        );
    }

    switch (bop.op) {
      case Assign: {
        const auto& castResult = leftType;
        if (startingSize == errors.size()) {
            if (rightType != castResult) {
                bop.rOperand->conversionType = castResult.innerType;
            }
        } else {
            return SemResult::Err;
        }

        return SemResult::ok(castResult);
    } break;

    case Add:
    case Sub:
    case Mul:
    case Div: {
        std::string castResult =
            leftType == "float" || rightType == "float" ? "float" : "int";
        if (startingSize == errors.size()) {
            if (leftType != castResult) {
                bop.lOperand->conversionType = castResult;
            }
            if (rightType != castResult) {
                bop.rOperand->conversionType = castResult;
            }
        } else {
            return SemResult::Err;
        }

        return SemResult::ok(castResult);
    } break;

    case Mod: {
        std::string castResult = "int";

        if (leftType == "float" || leftType == "string") {
            errors.push_back(lineError(
                "LHS in a modulo operation can't be '%s'.",
                bop.lOperand->lineNo,
                std::string(leftType)
            ));
        }

        if (rightType == "float" || rightType == "string") {
            errors.push_back(lineError(
                "RHS in a modulo operation can't be '%s'.",
                bop.rOperand->lineNo,
                std::string(rightType)
            ));
        }

        if (startingSize == errors.size()) {
            if (leftType != castResult) { bop.lOperand->conversionType = castResult; }
            if (rightType != castResult) { bop.rOperand->conversionType = castResult; }

        } else {
            return SemResult::Err;
        }

        return SemResult::ok(castResult);
    } break;

    case BitAnd:
    case BitOr:
    case BitXor:
    case LShift:
    case RShift: {
        std::string castResult = "int";

        if (leftType == "float" || leftType == "string") {
            errors.push_back(lineError(
                "LHS in a bitwise operation can't be '%s'.",
                bop.lOperand->lineNo,
                std::string(leftType)
            ));
        }

        if (rightType == "float" || rightType == "string") {
            errors.push_back(lineError(
                "RHS in a bitwise operation can't be '%s'.",
                bop.rOperand->lineNo,
                std::string(rightType)
            ));
        }

        if (startingSize == errors.size()) {
            if (leftType != castResult) { bop.lOperand->conversionType = castResult; }
            if (rightType != castResult) { bop.rOperand->conversionType = castResult; }
        } else {
            return SemResult::Err;
        }

        return SemResult::ok(castResult);
    } break;
    case And:
    case Or: {
        std::string castResult = "bool";
        if (startingSize == errors.size()) {
            if (leftType != castResult) {
                bop.lOperand->conversionType = castResult;
            }
            if (rightType != castResult) {
                bop.rOperand->conversionType = castResult;
            }
        } else {
            return SemResult::Err;
        }
        return SemResult::ok(castResult);
    } break;
    case GreaterEqual:
    case LessEqual:
    case GreaterThan:
    case LessThan:
    case Equal:
    case NotEqual: {
        std::string castResult = "bool";

        if (startingSize == errors.size()) {
            // ASSUMES  that left is op[0], right is op[1]
            auto& lOpr = bop.lOperand;
            auto& rOpr = bop.rOperand;

            int leftPriority = getTypePriority(leftType, lOpr);
            int rightPriority = getTypePriority(rightType, rOpr);

            // Note that when a node's conversionType is different from it's
            // type (for idNodes for example), we emit a convTypeToVarType
            // instruction.
            // for example, if a node has a type of `int` and a convType of
            // `float`, it will emit `floatToInt`
            if(leftPriority > rightPriority) {
                rOpr->conversionType = leftType.innerType;
            } else {
                lOpr->conversionType = rightType.innerType;
            }

        } else {
            return SemResult::Err;
        }
        return SemResult::ok(castResult);
    } break;
    }
}

#define LEFT_TYPE(left) \
    auto leftType = std::get<Type>(left); \

#define RIGHT_TYPE(right) \
    auto rightType = std::get<Type>(right); \

void getEnumTypes(ScopeSymbolTables* p, std::unordered_set<std::string>& enumTypes) {
    if (p == nullptr) return;

    for(const auto& [enumName, _]: p->enums) {
        enumTypes.insert(enumName);
    }

    getEnumTypes(p->parentScope, enumTypes);
}

bool isValidSymbol(Type semanticType,  Node* currentNode) {
    static std::unordered_set<std::string> builtinTypes = {
        "float",
        "int",
        "char",
        "string",
        "bool",
        "void",
    };

    std::unordered_set<std::string> enumTypes;                                 
    getEnumTypes(currentNode->currentScope, enumTypes);

    bool notBuiltinType = builtinTypes.find(semanticType.innerType) == builtinTypes.end();
    bool notEnumType = enumTypes.find(semanticType.innerType) == enumTypes.end();
    bool isInvalid = semanticType == Type::Invalid;

    return isInvalid || (notBuiltinType && notEnumType);
}

SemResult checkType(Type semanticType, Node* currentNode, std::string dataType) {
    if (isValidSymbol(semanticType, currentNode)) {
        errors.push_back(lineError(
            "The datatype '%s' is not valid.",
            currentNode->lineNo,
            std::string(dataType)
        ));

        return SemResult::Err;
    }

    return SemResult::Ok;
}

struct SemanticAnalysisVisitor {
    Node* currentNodePtr;

    SemResult operator()(conNodeType& con) { 
        return SemResult::ok(con.getType());
    }

    SemResult operator()(VarDecl& vd) const {
        /*
            TODO: Don't allow the declaration of a variable with the same name as a function
        */

        int startingSize = errors.size();

        auto type = vd.getType();
        auto nameStr = vd.getName();

        auto* symTable = currentNodePtr->currentScope;

        /* Check if the variable is already declared in this scope */
        if (symTable->sym2.contains(nameStr)) {
            errors.push_back(lineError(
                "The variable '%s' is already declared in this scope.",
                vd.type->lineNo,
                nameStr
            ));
        } else {
            /* Add the variable name & type as a new entry in the symbol table */
            auto declTypeResult = checkType(type, currentNodePtr, type.innerType);

            if(declTypeResult.isSuccess()) {
                symTable->sym2[nameStr] = SymbolTableEntry(type, vd.type->lineNo, false);
                return SemResult::ok(type);
            }         
        };

        return SemResult::Err;
    }

    SemResult operator()(VarDefn& vd) const {
      int startingSize = errors.size();

      /* Check if the variable is declared */
      vd.decl->self->currentScope = currentNodePtr->currentScope;
      SemResult decl = semanticAnalysis(vd.decl->self); 

      if(!decl.isSuccess()) { error(decl); }

      /* Check that the initial expression is valid if it exits */
      SemResult initResult;

      if (vd.initExpr != nullptr) {
        initResult = semanticAnalysis(vd.initExpr);
      } else {
        /* The initialization expression doesn't exist*/
        if (vd.isConstant) {
              errors.push_back("Error in line number: " +
              std::to_string(vd.decl->varName->lineNo) + " .The constant variable " +
              vd.decl->getName() + " has no value assigned to it");
        }

        if(startingSize != errors.size()) { return SemResult::Err; }
      }

      if(initResult.isSuccess() && decl.isSuccess()) {
        auto initType = std::get<Type>(initResult);
        auto declType = std::get<Type>(decl);
        auto castResult = castToTarget(initType, declType, currentNodePtr, currentNodePtr->lineNo);

        if(!castResult.isSuccess()) {
            error(castResult);
        } else {
            auto &varEntry = getVarEntry(vd.decl->getName(), currentNodePtr->currentScope);

            varEntry.isConstant = vd.isConstant;
        }
      }

      return decl;
    }
    
    SemResult operator()(idNodeType &identifier) const {
        int startingSize = errors.size();

        auto [symbolScope, symbolType] = getSymbolScope(identifier.id, currentNodePtr->currentScope);

        // Check if the given identifier exists in the current or preceeding scopes.
        if (symbolScope == nullptr || symbolType != SymbolType::Variable) {
            /* If not, then it's not in any scope. */
            errors.push_back(lineError(
                "Identifier '%s' is not declared.",
                currentNodePtr->lineNo,
                identifier.id
            ));

            return SemResult::Err;
        }

        auto type = symbolScope->sym2[identifier.id].type.innerType;
        auto typeResult = checkType(type, currentNodePtr, type);

        if (typeResult.isSuccess()) {
            symbolScope->sym2[identifier.id].isUsed = true;
            return SemResult::ok(symbolScope->sym2[identifier.id].type);
        } 

        return SemResult::Err;
    }

    SemResult operator()(caseNodeType& cs) {
        auto bodyResult = semanticAnalysis(cs.caseBody);
        auto labelResult = semanticAnalysis(cs.labelExpr);

        if(bodyResult.isSuccess() && labelResult.isSuccess()) return SemResult::Ok;

        if(!bodyResult.isSuccess()) error(bodyResult);
        if(!labelResult.isSuccess()) error(labelResult);

        return SemResult::err("");
    } 

    // Checks if there are duplicate case identifiers, and if all case labels
    // evaluate to the same type.
    SemResult operator()(switchNodeType &sw) const {
        int startingSize = errors.size();
        auto var = semanticAnalysis(sw.var);

        if (!var.isSuccess()) {
            errors.push_back(lineError(
                "Invalid switch variable '%s'.",
                sw.var->lineNo,
                sw.var->as<idNodeType>().id
            ));
        }

        auto cases = sw.caseListTail->as<caseNodeType>().toVec();

        for (int i = 0; i < cases.size(); i++) {
            auto labelExprResult = semanticAnalysis(cases[i]->labelExpr);

            /* Label Expression must be of the same type as the switch variable
             * & must be a constant or a literal */
            if (!labelExprResult.isSuccess()) {
                errors.push_back(lineError(
                    "Invalid case label expression.", cases[i]->labelExpr->lineNo
                ));
                continue;
            }

            // Default cases have no labelExpr. So we don't need to check them.
            // For a labelExpression to be true, it has to use constant or a
            // literal.
            if (!cases[i]->isDefault()) {

                auto labelExprType = std::get<Type>(labelExprResult);

                if(var.isSuccess()) {
                    auto varType = std::get<Type>(var);
                    if (varType != Type::Invalid && labelExprType != varType) {
                        errors.push_back(lineError(
                            "Mismatched switch variable and case label types (%s, %s).",
                            cases[i]->labelExpr->lineNo,
                            std::string(varType),
                            std::string(labelExprType)
                        ));
                    }
                }

                bool isConstant = false;
                bool isLiteral = false;
                auto *labelExpr = cases[i]->labelExpr;

                if (labelExpr->is<idNodeType>()) {

                    auto &labelName = labelExpr->as<idNodeType>().id;
                    auto &labelSymTableEntry =
                        getVarEntry(labelName, currentNodePtr->currentScope);
                    isConstant = labelSymTableEntry.isConstant;

                } else if (labelExpr->is<conNodeType>() || labelExpr->is<enumUseNode>()) {
                    isLiteral = true;
                }

                if (!isConstant && !isLiteral) {
                    errors.push_back(lineError(
                        "Case label expressions must be "
                        "constants or literals.",
                        cases[i]->labelExpr->lineNo
                    ));
                }
            }

            auto caseBody = semanticAnalysis(cases[i]->caseBody);
            if (!caseBody.isSuccess()) {
                errors.push_back(lineError(
                    "Invalid case body.", cases[i]->labelExpr->lineNo
                ));
            }
        }

        if (startingSize != errors.size()) {
            return SemResult::Err;
        }

        return var;
    }

    SemResult operator()(breakNodeType& br) const { 
        if(br.parent_switch == nullptr) {
            errors.push_back(lineError(
                "Break statements are valid only inside loops",
                currentNodePtr->lineNo
            ));

            return SemResult::Err;
        }
        return SemResult::Ok; 
    } 

    SemResult operator()(enumNode& en) const {

        /* Check that an enum with the same name is not declared already */
        std::string enumName = en.name->as<idNodeType>().id;
        auto [enumScope, _] =
            getSymbolScope(enumName, currentNodePtr->currentScope);

        // If the enum can be found in the current scope or the ones before it,
        // register an error.
        if (enumScope != nullptr) {
            errors.push_back(lineError(
                "The enum '%s' is already declared", en.name->lineNo, enumName
            ));
        }

        /* Check that all members are unique */
        std::unordered_set<std::string> members(en.enumMembers.begin(), en.enumMembers.end());

        if (members.size() != en.enumMembers.size()) {
            errors.push_back(lineError(
                "Enum '%s' has duplicate members.", en.name->lineNo, enumName
            ));
        }

        /* Add the enum to the enums table */
        currentNodePtr->currentScope->enums[enumName] = en;

        return SemResult::Ok;
    } 

    SemResult operator()(enumUseNode& eu) const { 
      
      int startingSize = errors.size();
      
      /* Check that the enum is declared */
      auto [symbolScope, symbolType] = getSymbolScope(eu.enumName, currentNodePtr->currentScope);

      if(symbolScope == nullptr || symbolType != SymbolType::Enum) {
          errors.push_back(
              lineError("The enum '%s' is not declared", eu.lineNo, eu.enumName)
          );
      } else {

        auto enumMembers = symbolScope->enums[eu.enumName].enumMembers;
        /* Check that the member name is part of the enum members */
        auto memberIter = std::find(enumMembers.begin(), enumMembers.end(), eu.memberName);
        bool notFound = memberIter == enumMembers.end();

        if (notFound) {
            errors.push_back(lineError(
                "The symbol '%s' is not a member of enum '%s'",
                eu.lineNo,
                eu.memberName,
                eu.enumName
            ));
        }
      }

      if (startingSize != errors.size()) { return SemResult::Err; }
      return SemResult::ok(eu.enumName);
    }

    SemResult operator()(FunctionDefn &fn) const {

        int startingSize = errors.size();

        /* Function Declaration code */
        auto functionName = fn.name->as<idNodeType>().id;
        auto [functionScope, _] = getSymbolScope(functionName, currentNodePtr->currentScope);

        // Already declared.
        if (functionScope != nullptr) {
            errors.push_back(lineError(
                "Function '%s' is already declared in this scope.",
                fn.name->lineNo,
                functionName
            ));
        }

        /* Add the new function to the functions table */
        // Since functions create their own scope, need to add
        // the function reference to the parent scope.
        currentNodePtr->currentScope->functions[fn.name->as<idNodeType>().id] = fn;

        /* Check if the function parameters are valid */
        auto parameters = fn.getParameters();
        auto parameterNodes = fn.getParametersAsNodes();

        for (int i = 0; i < parameters.size(); i++) {
            auto *param = parameters[i];
            auto* paramPtr = parameterNodes[i];

            paramPtr->currentScope = fn.statements->currentScope;
            auto paramResult = semanticAnalysis(paramPtr);

            if (!paramResult.isSuccess()) {
                errors.push_back(lineError(
                    "Function '%s': parameter '%s' is not valid",
                    param->type->lineNo,
                    functionName,
                    param->varName->as<idNodeType>().id 
                ));
            }

            auto paramName = param->getName();
            if(paramName == functionName) {
                errors.push_back(lineError(
                    "Function '%s': cannot have a parameter with the same name as the function",
                    param->varName->lineNo,
                    functionName
                ));
            }
        }

        /* Check if the return type is valid */
        auto returnType = fn.return_type->as<Type>();
        auto returnTypeResult = checkType(returnType, currentNodePtr, getFnType(fn));
        
        if(!returnTypeResult.isSuccess()) {
            errors.push_back(lineError(
                "Function '%s': return type is not valid",
                fn.return_type->lineNo,
                functionName
            ));
        }

        /* Check if the function body is valid */
        auto functionBody = semanticAnalysis(fn.statements);
        if (!functionBody.isSuccess()) {
            errors.push_back(lineError(
                "Function '%s': body is not valid", 
                fn.statements->lineNo,
                functionName
            ));
        } else {

            // If the function body is valid, check the return types.

            auto innerReturnStatements = getFnReturnStatements(fn.statements);

            // Need to perserve the order, hence no unordered_set
            auto innerReturnTypes = std::set<std::string>();
            for (auto *ret : innerReturnStatements) {
                innerReturnTypes.insert(getReturnType(ret).innerType);
            }

            auto returnLines = collectStrings<std::vector<Node *>>(
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
                    auto returnTypes = collectStrings<std::set<std::string>>(innerReturnTypes.begin(), innerReturnTypes.end());
                    errors.push_back(fmt(
                        "Error at line(s): %s. Void function '%s' cannot return "
                        "types (%s).",
                        returnLines,
                        functionName,
                        returnTypes
                    ));
                }
            } else {
                if (innerReturnTypes.empty()) {
                    // No return types
                    errors.push_back(
                        fmt("Error at lines(s): %s. Function '%s' with return type "
                            "'%s' has no returns.",
                            returnLines,
                            functionName,
                            returnType.innerType)
                    );
                } else if (innerReturnTypes.size() != 1) {
                    // Multiple return types
                    auto returnTypes = collectStrings<std::set<std::string>>(innerReturnTypes.begin(), innerReturnTypes.end());
                    errors.push_back(
                        fmt("Error at line(s): %s. Mismatched return statements "
                            "(%s) in function '%s' with return type '%s'.",
                            returnLines,
                            returnTypes,
                            functionName,
                            returnType.innerType)
                    );
                } else if (innerReturnTypes.size() == 1) {
                    // Just one return type
                    // Need to make sure that the type of the return is castable to
                    // the function's return type.
                    auto innerType = *innerReturnTypes.begin();

                    auto castInnerReturnTypes = castToTarget(
                        innerType,
                        returnType,
                        currentNodePtr,
                        currentNodePtr->lineNo
                    );

                    if (!castInnerReturnTypes.isSuccess()) {
                        errors.push_back(lineError(
                            "Cannot return type '%s' from "
                            "function '%s' with return type '%s'. ",
                            currentNodePtr->lineNo,
                            innerType,
                            functionName,
                            returnType.innerType
                        ));
                    }
                }
            }
        }

        if (startingSize != errors.size()) {
            return SemResult::Err;
        }
        return SemResult::ok(returnType);
    }

    // Checks if the function has already been declared
    // Checks if the function call arguments have the same count and type as the parameters.
    SemResult operator()(FunctionCall& fc) const { 
        auto intToOrdinal = [](int i) {
            using namespace std::string_literals;
            if (i == 0) {
                return "1st"s;
            } else if (i == 1) {
                return "2nd"s;
            } else if (i == 2) {
                return "3rd"s;
            } else {
                return fmt("%dth", i + 1);
            }
        };

        int startingSize = errors.size();

        auto exprResults = Utils::transform<SemResult>(
            fc.parameterExpressions,
            [](ExprListNode *expr) { return semanticAnalysis(expr->exprCode); }
        );

        auto exprErrors = Utils::filter(exprResults, [](const SemResult &r) {
            return !r.isSuccess();
        });

        for (int i = 0; i < exprErrors.size(); i++) {
            errors.push_back(lineError(
                "Function '%s': The %s function parameter expression is invalid.",
                fc.parameterExpressions[i]->exprCode->lineNo,
                fc.functionName,
                intToOrdinal(i)
            ));
        }

        /* Check if the function exists in the functions table or not */
        auto [fnScope, symbolType] =
            getSymbolScope(fc.functionName, currentNodePtr->currentScope);

        // Function can't be found in current or preceeding scopes => not declared.
        if(fnScope == nullptr || symbolType != SymbolType::Function) {
            errors.push_back(lineError(
                "Function '%s' is not declared in this scope.", fc.lineNo, fc.functionName
            ));
        } else {
            auto &fnRef = fnScope->functions[fc.functionName];
            auto fnParams = fnRef.getParameters();

            if (fnParams.size() != fc.parameterExpressions.size()) {
                errors.push_back(lineError(
                    "Number of arguments to function '%s' doesn't match the "
                    "number of parameters in its definition.",
                    fc.lineNo,
                    fc.functionName
                ));
            }

            /* Check if the Expression List is valid */
            for (int i = 0; i < std::min(fc.parameterExpressions.size(), fnParams.size()); i++) {
                /* Check that the parameter types match with the function types */
                auto exprResult = exprResults[i];
                auto exprCodeLine = fc.parameterExpressions[i]->exprCode->lineNo;

                if (exprResult.isSuccess()) {
                    // Check if the passed  expression matches the parameter's type
                    auto paramType = fnParams[i]->getType();
                    auto exprType = std::get<Type>(exprResult);

                    SemResult exprIsCastable = castToTarget(exprType, paramType, fc.parameterExpressions[i]->self, fc.lineNo);

                    if (!exprIsCastable.isSuccess()) {
                        errors.push_back(lineError(
                            "The %s function paramter to '%s' "
                            "has a mismatched type.",
                            exprCodeLine,
                            intToOrdinal(i)
                        ));
                    }
                }
            }
        }

        if (startingSize != errors.size()) { return SemResult::Err; }

        auto fnReturnType = fnScope->functions[fc.functionName].return_type->as<Type>().innerType;
        return SemResult::ok(fnReturnType);
    } 

    /* 
      Entry point, we get a list of statements & iterate over each of them & make sure they are
      Semantically correct
    */
    SemResult operator()(StatementList& sl) const {
        
        auto statementResults = Utils::transform<SemResult>(sl.statements, semanticAnalysis);
        SemResult ret = SemResult::mergeResults(statementResults);

        auto variables = currentNodePtr->currentScope->sym2;
        for(const auto& [name, entry]: variables) {
            if(!entry.isUsed) {
                warnings.push_back(lineWarning(
                    "Variable '%s' is declared but never used.",
                    entry.declLine,
                    name
                ));
            }
        }

        return ret;
    }
    
    SemResult operator()(doWhileNodeType& dw) {
        int startingSize = errors.size();

        auto condition = semanticAnalysis(dw.condition);
        if (!condition.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(dw.condition->lineNo) +
                    " .The condition in do while loop is not valid");
        } else {
            auto conditionType = std::get<Type>(condition);
            if (conditionType != "bool") {
                SemResult conditionCastResult = castToTarget(conditionType, Type("bool"), dw.condition, dw.condition->lineNo);

                if (!conditionCastResult.isSuccess()) {
                    error(conditionCastResult);
                }
            }
        }

        auto loop = semanticAnalysis(dw.loop_body); 
        if (!loop.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(dw.loop_body->lineNo) +
                    " .The body in do while loop is not valid");
        } 

        if(startingSize != errors.size()) { return SemResult::Err; }
        return condition;
    }

    SemResult operator()(whileNodeType& w) { 
        int startingSize = errors.size();
        auto condition = semanticAnalysis(w.condition);
        if (!condition.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(w.condition->lineNo) +
                    " .The condition in while loop is not valid");
        } else {
            auto conditionType = std::get<Type>(condition);
            if (conditionType != "bool") {
                SemResult conditionCastResult = castToTarget(conditionType, Type( "bool"), w.condition, w.condition->lineNo);
                if (!conditionCastResult.isSuccess()) {
                    error(conditionCastResult);
                }
            }
        }  
        auto loop = semanticAnalysis( w.loop_body);
        if (!loop.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(w.loop_body->lineNo) +
                    " .The body in while loop is not valid");
        }

        if(startingSize != errors.size()) { return SemResult::Err; }
        return condition;
    }

    SemResult operator()(forNodeType& f) { 
        int startingSize = errors.size();

        auto init_statement = semanticAnalysis( f.init_statement);
        if (!init_statement.isSuccess()){
            errors.push_back(lineError(
                "Invalid for loop initialization statament.",
                f.init_statement->lineNo
            ));
        }

        auto condition = semanticAnalysis(f.loop_condition);
        if (!condition.isSuccess()){
            errors.push_back(lineError(
                "Invalid for loop condition.", f.loop_condition->lineNo
            ));
        } else {
            auto conditionType = std::get<Type>(condition);
            if (conditionType != "bool") {
                SemResult conditionCastResult = castToTarget(conditionType, Type("bool"), f.loop_condition, f.loop_condition->lineNo);

                if (!conditionCastResult.isSuccess()) {
                    error(conditionCastResult);
                }
            }
        }

        auto loop = semanticAnalysis(f.loop_body);
        if (!loop.isSuccess()){
            errors.push_back(
                lineError("Invalid for loop body.", f.loop_body->lineNo)
            );
        }

        auto post_statement = semanticAnalysis(f.post_loop_statement);
        if (!post_statement.isSuccess()){
            errors.push_back(lineError(
                "Invalid for loop post body statement.",
                f.post_loop_statement->lineNo
            ));
        }

        if(startingSize != errors.size()) { return SemResult::Err; }             
        return condition;
    }
           

    SemResult operator()(BinOp& bop) const {
      /* 
       * TODO: Implement the following check when the function identifier is
       * added to the symbol table
       * Check that the right expression is not a function identifier but a function call *
       * Check that the left identifier is not a function *

       ! Do some operator specific checks !
       */

      int startingSize = errors.size();

      SemResult left = semanticAnalysis(bop.lOperand);
      SemResult right = semanticAnalysis(bop.rOperand);

      if (!left.isSuccess()) { return left; }
      if (!right.isSuccess()) { return right; }

      /* Check that the two expressions on the left & on the right are of the
       * same type, or the RHS is castable to the LHS's type. */

      auto leftType = std::get<Type>(left);
      auto rightType = std::get<Type>(right);
      auto finalType = leftType;

      SemResult castResult = castBinOp(leftType, rightType, bop);
      if (castResult.isSuccess()) {
          finalType = std ::get<Type>(castResult);
      }

      auto setIdNodeAsUsed =
        [currentScope = currentNodePtr->currentScope](Node *ptr) {
          ptr->currentScope = currentScope;

          if (auto *idNode = ptr->asPtr<idNodeType>(); idNode) {
            auto nodeSymEntry = getVarEntry(idNode->id, ptr->currentScope);
            nodeSymEntry.isUsed = true;
          }
        };

      setIdNodeAsUsed(bop.lOperand);
      setIdNodeAsUsed(bop.rOperand);

      if (startingSize != errors.size()) { return SemResult::Err; }
      return SemResult::ok(finalType);
    }

    SemResult operator()(UnOp& uop) {
      using enum UnOper;

      int startingSize = errors.size();

      switch (uop.op) {
      case Print: {
          /* Check that the right expression is valid */
          RIGHT_VALID(uop.operand); // * gives right
          return right;
      } break;

      case Return: {
          if (uop.operand != nullptr) return semanticAnalysis(uop.operand);
      } break;

      case Minus:
      case Plus: {
          return semanticAnalysis(uop.operand);
      } break;

      case Increment:
      case Decrement: {
          /* Check that the left identifier is not a constant */
          auto *symTable = uop.operand->currentScope;
          NOT_CONST(uop.operand, uop.operand->lineNo, symTable->sym2);

          /* Check that the left expression is valid (identifier is declared)
           */
          LEFT_VALID(uop.operand); // * gives left
          LEFT_TYPE(left);

          /* Manual casting without macro */
          if (leftType == "string") {
              errors.push_back(
                  "Error in line number: " +
                  std::to_string(uop.operand->lineNo) +
                  " .Cannot cast to or from string"
              );
          } else if (leftType == "int" || leftType == "float") {
              return SemResult::ok(leftType);
          } else if (leftType == "char") {
              return SemResult::ok("int"s);
          } else {
              errors.push_back(
                  "Error in line number: " +
                  std::to_string(uop.operand->lineNo) +
                  " .Can't Increment/Decrement booleans"
              );
          }

          if (startingSize != errors.size()) {
              return SemResult::Err;
          }
          return SemResult::ok(leftType);
      } break;

      default:
          return SemResult::Ok;
      }

      if (startingSize != errors.size()) { return SemResult::Err; }
      return SemResult::Ok;
    }

    SemResult operator()(IfNode &ifNode) {
        int startingSize = errors.size();

        SemResult condition = semanticAnalysis(ifNode.condition);

        if (!condition.isSuccess()) {
            errors.push_back(lineError(
                "Invalid if statement condition.", ifNode.condition->lineNo
            ));
        } else {
            auto conditionType = std::get<Type>(condition);

            if (conditionType != "bool") {
                SemResult conditionCastResult = castToTarget(
                    conditionType,
                    Type("bool"),
                    ifNode.condition,
                    ifNode.condition->lineNo
                );

                if (!conditionCastResult.isSuccess()) {
                    error(conditionCastResult);
                }
            }
        }

        /* Check if the case condition is always false */
        if (condition.isSuccess()) {
            ConstOpt alwaysFalseResult = checkConstantExpressions(ifNode.condition);

            if (alwaysFalseResult.has_value() && std::get<Type>(condition) == "bool") {
                Value val = alwaysFalseResult.value();

                if (bool(val) == false) {
                    warnings.push_back(lineWarning(
                        "If statement condition is always false.",
                        ifNode.condition->lineNo
                    ));
                }
            }
        }

        SemResult ifBody = semanticAnalysis(ifNode.ifCode);
        if (!ifBody.isSuccess()) {
            errors.push_back(
                lineError("Invalid if statement block.", ifNode.ifCode->lineNo)
            );
        }

        if (ifNode.elseCode != nullptr) {
            SemResult elseBody = semanticAnalysis(ifNode.elseCode);
            if (!ifBody.isSuccess()) {
                errors.push_back(lineError(
                    "Invalid else statement block.", ifNode.elseCode->lineNo
                ));
            }
        }

        if (startingSize != errors.size()) {
            return SemResult::Err;
        }
        return SemResult::ok(std::get<Type>(condition));

        if (startingSize != errors.size()) { return SemResult::Err; }

        return SemResult::Ok;
    }

    SemResult operator()(ArrayLiteral &al) const {
        std::set<std::string> elementTypes;

        bool anyElementErrored = false;
        for (auto &exp : al.expressions) {
            SemResult exprResult = semanticAnalysis(exp);

            if (!exprResult.isSuccess()) {
                errors.push_back(
                    lineError("Array element is invalid!", exp->lineNo)
                );
                anyElementErrored = true;
            } else {
                auto exprType = std::get<Type>(exprResult);
                elementTypes.insert(exprType.innerType);
            }
        }

        if (elementTypes.size() > 1) {
            // Multiple types in array
            auto arrayElements = collectStrings<std::set<std::string>>(
                elementTypes.begin(), elementTypes.end()
            );

            errors.push_back(lineError(
                "An array cannot elements with multiple types (%s)",
                currentNodePtr->lineNo,
                arrayElements
            ));

            return SemResult::Err;
        }

        auto arrayType = elementTypes.empty()? Type::Any : *elementTypes.begin();
        arrayType.depth = getArrayDepth(currentNodePtr);;
        arrayType.isArray = Type::IsArray::Yes;

        return SemResult::ok(arrayType);
    }

    SemResult operator()(ArrayIndex& ai) {
        auto arrayExprResult = semanticAnalysis(ai.arrayExpr);

        if(!arrayExprResult.isSuccess()) {
            errors.push_back(lineError("Invalid array expression.", currentLineNo));
        } else {
            auto arrayType = std::get<Type>(arrayExprResult);
            if(arrayType.isArray != Type::IsArray::Yes) {
                errors.push_back(lineError("Cannot index a non-array.", currentLineNo));
            }
        }


        auto indexExprResult = semanticAnalysis(ai.indexExpr);
        if(!arrayExprResult.isSuccess()) {
            errors.push_back(lineError("Invalid array indexing expression.", currentLineNo));
        } else {
            auto indexType = std::get<Type>(arrayExprResult);
            if(indexType != "int" && indexType.isArray == Type::IsArray::No) {
                errors.push_back(lineError("Cannot index an array with a non-integer expression", currentLineNo));
            }
        }

        return SemResult::Ok;
    }

    // the default case:
    template<typename T> 
    SemResult operator()(T const & /*UNUSED*/ ) const { return SemResult::Ok; } 
};

SemResult semanticAnalysis(Node *p) {    
    if (p == nullptr) return SemResult::Ok;
    return std::visit(SemanticAnalysisVisitor{p}, *p);
}
