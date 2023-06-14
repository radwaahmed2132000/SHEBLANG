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
#include "nodes.h"
#include "parser.h"
#include "result.h"
#include "semantic_utils.h"
#include "template_utils.h"
#include "value.h"

using ErrorList = std::vector<std::string>;
using SemResult = Result<Type, ErrorList>;
using namespace std::string_literals;

/*
    TODO: return statements should be valid only inside functions.
    This can be done similarly to how break nodes are checked, we set the parent
   during parsing (can be done during semantic analysis too), then in semantic
   analysis, we check if the return node has a parent.
*/

#define LEFT_VALID(oper) SemResult left = semantic_analysis(oper);

#define RIGHT_VALID(oper) SemResult right = semantic_analysis(oper);

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

SemResult cast_opr(const Type& leftType, const Type& rightType, BinOp& bop) {
    using enum BinOper;

    int startingSize = errors.size();
    if (leftType == "string" && rightType == "string") {
        if (bop.op == Assign) {
            return SemResult::ok("string"s);
        } else if (bop.op == Equal or bop.op == NotEqual) {
            return SemResult::ok("bool"s);
        } else {
            errors.push_back(
                "Error in line number: " + std::to_string(bop.lOperand->lineNo) +
                " .Cannot perform this operation on strings");
        }
    } else if (leftType == "string" || rightType == "string") {
        errors.push_back(
            "Error in line number: " + std::to_string((bop.lOperand->lineNo)) +
            " .Cannot cast to or from string");
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
            errors.push_back(
                "Error in line number: " + std::to_string(bop.lOperand->lineNo) +
                " .The LHS in a modulo operation can't be " + std::string(leftType));
        }
        if (rightType == "float" || rightType == "string") {
            errors.push_back(
                "Error in line number: " + std::to_string(bop.rOperand->lineNo) +
                " .The RHS in a modulo operation can't be " + std::string(rightType));
        }
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

    case BitAnd:
    case BitOr:
    case BitXor:
    case LShift:
    case RShift: {
        std::string castResult = "int";
        if (leftType == "float" || leftType == "string") {
            errors.push_back(
                "Error in line number: " + std::to_string(bop.lOperand->lineNo) +
                " .The LHS in a bitwise operation can't be " + std::string(leftType));
        }
        if (rightType == "float" || rightType == "string") {
            errors.push_back(
                "Error in line number: " + std::to_string(bop.rOperand->lineNo) +
                " .The RHS in bitwise operation can't be " + std::string(rightType));
        }
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
    default: {
        return SemResult::err(
            "Error in line number: " + std::to_string(bop.lOperand->lineNo) +
            " .Invalid operator");
    }
    }
}

#define LEFT_SAME_TYPE_AS_RIGHT(left, right, lineNo, opr)      \
    if (!left.isSuccess()) {                                   \
        return left;                                           \
    }                                                          \
    if (!right.isSuccess()) {                                  \
        return right;                                          \
    }                                                          \
    auto leftType = std::get<Type>(left);                      \
    auto rightType = std::get<Type>(right);                    \
    auto finalType = leftType;                                 \
    SemResult castResult = cast_opr(leftType, rightType, opr); \
    if (!castResult.isSuccess()) {                             \
    } else {                                                   \
        finalType = std::get<Type>(castResult);                \
    }                                                          \

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

#define EXISTING_TYPE(type, lineNo)                                            \
    static std::unordered_set<std::string> builtinTypes = {                    \
        "float", "int", "char", "string", "bool", "void"};                     \
    std::unordered_set<std::string> enumTypes;                                 \
    getEnumTypes(currentNodePtr->currentScope, enumTypes);                     \
    if (type != Type::Invalid &&                                               \
        builtinTypes.find(type) == builtinTypes.end() &&                       \
        enumTypes.find(type) == enumTypes.end()) {                             \
        errors.push_back(                                                      \
            "Error in line number: " + std::to_string(lineNo) +                \
            " .The data type \"" + type + "\" is not valid"                    \
        );                                                                     \
    }

SemResult ex_const_kak_TM(Node *p);

struct ex_const_visitor {
    Node* currentNodePtr;

    SemResult operator()(conNodeType& con) { 
        auto res = SemResult::ok(con.getType());
        res.value = &con;
        return res;
    }

    SemResult operator()(idNodeType& id) const {
        auto* idScope = getSymbolScope(id.id, currentNodePtr->currentScope);

        if(idScope != nullptr) {
            auto& entry = idScope->sym2[id.id];
            auto isConstant = entry.isConstant;
            auto hasInit = entry.initExpr != nullptr;
            auto* literalInit  = entry.initExpr->asPtr<conNodeType>();

            if (literalInit != nullptr && isConstant && hasInit) {
                SemResult result = SemResult::ok(entry.type);
                result.value = &entry.value;
                return result;
            } 
        }

        return SemResult::Err;
    }

    SemResult operator() (BinOp& bop) { 
        using enum BinOper;

        auto left = ex_const_kak_TM(bop.lOperand);
        auto right = ex_const_kak_TM(bop.rOperand);

        if (!left.isSuccess()) { return left; }
        if (!right.isSuccess()) { return right; }

        SemResult result;
        switch (bop.op) {
            case Add:
            case Sub:
            case Mul:
            case Div: {

                auto finalType = std::get<Type>(left) == "float" ? "float" : "int";
                finalType = std::get<Type>(right) == "float" ? "float" : finalType;

                result = SemResult::ok(Type(finalType));

                if (bop.op == Add) { result.value = new Value(*left.value + *right.value); }
                if (bop.op == Sub) { result.value = new Value(*left.value - *right.value); }
                if (bop.op == Mul) { result.value = new Value(*left.value * *right.value); }
                if (bop.op == Div) { result.value = new Value(*left.value / *right.value); }
                if (bop.op == Mod) { result.value = new Value(*left.value % *right.value); }

                return result;
            } break;

            case Mod: 
            case BitAnd:
            case BitOr:
            case BitXor:
            case LShift:
            case RShift: {
                result = SemResult::ok(std::get<Type>(left));

                if (bop.op == BitAnd) { result.value = new Value(*left.value & *right.value); }
                if (bop.op == BitOr) { result.value = new Value(*left.value | *right.value); }
                if (bop.op == BitXor) { result.value = new Value(*left.value ^ *right.value); }
                if (bop.op == LShift) { result.value = new Value(*left.value << *right.value); }
                if (bop.op == RShift) { result.value = new Value(*left.value >> *right.value); }

                return result;
            } break;

            case LessThan:
            case GreaterThan:
            case Equal:
            case NotEqual:
            case GreaterEqual:
            case LessEqual:
            case And:
            case Or: {
                result = SemResult::ok(Type("bool"));

                if (bop.op == LessThan) { result.value = new Value(*left.value < *right.value); }
                if (bop.op == GreaterThan) { result.value = new Value(*left.value > *right.value); }
                if (bop.op == Equal) { result.value = new Value(*left.value == *right.value); }
                if (bop.op == NotEqual) { result.value = new Value(*left.value != *right.value); }
                if (bop.op == GreaterEqual) { result.value = new Value(*left.value >= *right.value); }
                if (bop.op == LessEqual) { result.value = new Value(*left.value <= *right.value); }

                return result;
            }

          case Assign: { /* Do nothing */ }
        }

        if(result.isSuccess()) 
          return result;

        return SemResult::err("");
    }

    SemResult operator()(IfNode& ifNode) {
        auto conditionResult = ex_const_kak_TM(ifNode.condition);
        if(!conditionResult.isSuccess()) return conditionResult;

        auto ifResult = ex_const_kak_TM(ifNode.ifCode);
        if(!ifResult.isSuccess()) return ifResult;

        if(ifNode.elseCode != nullptr) {
            auto elseResult = ex_const_kak_TM(ifNode.elseCode);
            if(!elseResult.isSuccess()) return elseResult;
        }

        return SemResult::Ok;
    }

    // the default case:
    template<typename T> 
    SemResult operator()(T const & /*UNUSED*/ ) const { return SemResult::Ok; }  
};

SemResult ex_const_kak_TM(Node *p) {    
    if (p == nullptr) return SemResult::Ok;
    return std::visit(ex_const_visitor{p}, *p);
}

struct semantic_analysis_visitor {
    Node* currentNodePtr;

    SemResult operator()(conNodeType& con) { 
        return SemResult::ok(con.getType());
    }

    SemResult operator()(VarDecl& vd) const {
        /*
            TODO: Don't allow the declaration of a variable with the same name as a function
        */

        int startingSize = errors.size();
        /* Check that the type of the variable is valid */
        auto type = vd.getType();

        /* Check that the type & name are valid */

        EXISTING_TYPE(type.innerType, vd.type->lineNo);
        
        auto* symTable = currentNodePtr->currentScope;
        /* Check if the variable is already declared in this scope */
        auto nameStr = vd.getName();
        if (symTable->sym2.find(nameStr) != symTable->sym2.end()) {
          errors.push_back("Error in line number: " +
            std::to_string(vd.type->lineNo) + " .The variable " +
            nameStr + " is already declared in this scope.");
        } else {
          /* Add the variable name & type as a new entry in the symbol table */
          // TODO: Use a SymbolTableEntry constructor?
          SymbolTableEntry entry = SymbolTableEntry();
          entry.type = type;
          entry.declaredAtLine = vd.type->lineNo;
          entry.isUsed = false;
          symTable->sym2[nameStr] = entry;
          return SemResult::ok(entry.type);
        };
        return SemResult::Err;
    }

    SemResult operator()(VarDefn& vd) const {
      int startingSize = errors.size();

      /* Check if the variable is declared */
      vd.decl->self->currentScope = currentNodePtr->currentScope;
      SemResult decl = semantic_analysis(vd.decl->self); 

      if(!decl.isSuccess()) { error(decl); }

      /* Check that the initial expression is valid if it exits */
      SemResult initResult;

      if (vd.initExpr != nullptr) {
        initResult = semantic_analysis(vd.initExpr);
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
            auto &varEntry = getSymEntry(vd.decl->getName(), currentNodePtr->currentScope);

            varEntry.isConstant = vd.isConstant;
            varEntry.initExpr = vd.initExpr;
        }
      }

      return decl;
    }
    

    // Check if the given identifier exists in the current or preceeding scopes.
    SemResult operator()(idNodeType &identifier) const {
        int startingSize = errors.size();
        auto *symTable =
            getSymbolScope(identifier.id, currentNodePtr->currentScope);

        if (symTable == nullptr) {
            /* If not, then it's not in any scope. */
            errors.push_back(
                "Error in line number: " +
                std::to_string(currentNodePtr->lineNo) + " .Identifier " +
                identifier.id + " is not declared"
            );
            return SemResult::Err;
        }

        EXISTING_TYPE(
            symTable->sym2[identifier.id].type.innerType, currentNodePtr->lineNo
        );
        symTable->sym2[identifier.id].isUsed = true;

        return SemResult::ok(symTable->sym2[identifier.id].type);
    }

    SemResult operator()(caseNodeType& cs) {
        auto bodyResult = semantic_analysis(cs.caseBody);
        auto labelResult = semantic_analysis(cs.labelExpr);

        if(bodyResult.isSuccess() && labelResult.isSuccess()) return SemResult::Ok;

        if(!bodyResult.isSuccess()) error(bodyResult);
        if(!labelResult.isSuccess()) error(labelResult);

        return SemResult::err("");
    } 

    // Checks if there are duplicate case identifiers, and if all case labels
    // evaluate to the same type.
    SemResult operator()(switchNodeType &sw) const {
        int startingSize = errors.size();
        auto var = semantic_analysis(sw.var);

        if (!var.isSuccess()) {
            errors.push_back(
                "Error in line number: " + std::to_string(sw.var->lineNo) +
                " .The switch variable is not valid"
            );
        }

        auto cases = sw.caseListTail->as<caseNodeType>().toVec();

        for (int i = 0; i < cases.size(); i++) {
            auto labelExprResult = semantic_analysis(cases[i]->labelExpr);

            /* Label Expression must be of the same type as the switch variable
             * & must be a constant or a literal */
            if (!labelExprResult.isSuccess()) {
                errors.push_back(
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

                auto labelExprType = std::get<Type>(labelExprResult);

                if(var.isSuccess()) {
                    auto varType = std::get<Type>(var);
                    if (varType != Type::Invalid && labelExprType != varType) {
                        auto lineNo =
                            std::to_string(cases[i]->labelExpr->lineNo);
                        errors.push_back(
                            "Error in line number: " + lineNo +
                            " .The label expression of the case statement is "
                            "not "
                            "of the same type as the switch variable"
                        );
                    }
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
                    errors.push_back(
                        "Error in line number: " +
                        std::to_string(cases[i]->labelExpr->lineNo) +
                        " .The label expression of the case statement must be "
                        "a constant or a literal"
                    );
                }
            }

            auto caseBody = semantic_analysis(cases[i]->caseBody);
            if (!caseBody.isSuccess()) {
                errors.push_back(
                    "Error in line number: " +
                    std::to_string(cases[i]->labelExpr->lineNo) +
                    " .The case body of the case statement is not valid"
                );
            }
        }

        if (startingSize != errors.size()) {
            return SemResult::Err;
        }

        return var;
    }

    SemResult operator()(breakNodeType& br) const { 
        if(br.parent_switch == nullptr) {
            auto lineNo = std::to_string(currentNodePtr->lineNo);
            errors.push_back("Error at line: " + lineNo + ". Break statements are valid inside for loops, while loops, or do while loops only." );

            return SemResult::Err;
        }
        return SemResult::Ok; 
    } 

    SemResult operator()(enumNode& en) const { 

      /* Check that an enum with the same name is not declared already */
      std::string enumName = en.name->as<idNodeType>().id;
			auto* enumScope = getSymbolScope(enumName, currentNodePtr->currentScope);

			// If the enum can be found in the current scope or the ones before it, register an error.
			if(enumScope != nullptr) {
        errors.push_back("Error in line number: " + std::to_string(en.name->lineNo) +
                              " .The enum " + enumName + " is already declared");
			}

      /* Check that all members are unique */
      std::unordered_set<std::string> members(en.enumMembers.begin(), en.enumMembers.end());

      if(members.size() != en.enumMembers.size()) {
        errors.push_back("Enum has duplicate memebers in line "+std::to_string(en.name->lineNo) );
      }
    
      /* Add the enum to the enums table */
      currentNodePtr->currentScope->enums[enumName] = en;
      
      return SemResult::Ok;
    } 

    SemResult operator()(enumUseNode& eu) const { 
      
      int startingSize = errors.size();
      
      /* Check that the enum is declared */
      auto* scope = getSymbolScope(eu.enumName, currentNodePtr->currentScope);

      if(scope == nullptr) {
					errors.push_back("Error in line number: " + std::to_string(eu.lineNo) +
									" .The enum " + eu.enumName + " is not declared");
      } else {
        auto enumMembers = scope->enums[eu.enumName].enumMembers;
        /* Check that the member name is part of the enum members */
        if (std::find(enumMembers.begin(), enumMembers.end(), eu.memberName) == enumMembers.end()) {
						errors.push_back("Error in line number: " + std::to_string(eu.lineNo) +
										" .The enum member " + eu.memberName + " is not part of the enum " + eu.enumName);
        }
      }

      if (startingSize != errors.size()) {
        return SemResult::Err;
      }

      return SemResult::ok(eu.enumName);
    }

    SemResult operator()(FunctionDefn &fn) const {

        int startingSize = errors.size();

        /* Function Declaration code */
        auto functionName = fn.name->as<idNodeType>().id;
        auto *functionScope = getSymbolScope(functionName, currentNodePtr->currentScope);

        // Already declared.
        if (functionScope != nullptr) {
            errors.push_back(lineError(
                "Function '%s' is already declared in this scope",
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

            /* Check if the parameter list is emtpy */
            if (param->type == nullptr || param->varName == nullptr) {
                break;
            }

            auto* paramPtr = parameterNodes[i];
            paramPtr->currentScope = fn.statements->currentScope;
            auto paramResult = semantic_analysis(paramPtr);

            if (!paramResult.isSuccess()) {
                errors.push_back(lineError(
                    "The function parameter '%s' is not valid",
                    param->type->lineNo,
                    param->varName->as<idNodeType>().id 
                ));
            }
        }

        /* Check if the function body is valid */
        auto functionBody = semantic_analysis(fn.statements);
        if (!functionBody.isSuccess()) {
            errors.push_back(lineError(
                "The function body is not valid", fn.statements->lineNo
            ));
        }

        /* Check if the return type is valid */
        auto returnType = fn.return_type->as<idNodeType>().id;
        EXISTING_TYPE(returnType, fn.return_type->lineNo);
        if (errors.size() != startingSize) {
            errors.push_back(lineError(
                "The function return type '%s' is not valid",
                fn.return_type->lineNo,
                returnType
            ));
        }

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
                errors.push_back(
                    fmt("Error at line(s): %s. Void functions cannot return "
                        "types (%s).",
                        returnLines,
                        returnTypes)
                );
            }
        } else {
            if (innerReturnTypes.empty()) {
                // No return types
                errors.push_back(
                    fmt("Error at lines(s): %s. Function with return type '%s' "
                        "has no returns.",
                        returnLines,
                        returnType)
                );
            } else if (innerReturnTypes.size() != 1) {
                // Multiple return types

                auto returnTypes = collectStrings<std::set<std::string>>(innerReturnTypes.begin(), innerReturnTypes.end());
                errors.push_back(
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
                    errors.push_back(lineError(
                        "Cannot return type '%s' from a "
                        "function with return type '%s'. ",
                        currentNodePtr->lineNo,
                        innerType,
                        returnType
                    ));
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
        int startingSize = errors.size();

        /* Check if the function exists in the functions table or not */
        auto* fnScope = getSymbolScope(fc.functionName, currentNodePtr->currentScope); 
        auto callArgExprs = fc.parameterExpressions;
        std::vector<VarDecl*> fnParams;

        // Function can't be found in current or preceeding scopes => not declared.
        if(fnScope == nullptr) {
            errors.push_back("Error in line number: " + 
                    std::to_string(fc.lineNo) + " .The function "
                    + fc.functionName + " is not declared");
        } else {
            auto &fnRef = fnScope->functions[fc.functionName];
            fnParams = fnRef.getParameters();

            if (fnParams.size() != callArgExprs.size()) {
                errors.push_back(
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
                errors.push_back("Error in line number: " +
                        std::to_string(arg->exprCode->lineNo) + 
                        " .The function parameter number: " + std::to_string(i) + " is not valid");
            } else {
                if (fnScope != nullptr) {
                    auto *param = fnParams[i];

                    // Check if the passed  expression matches the parameter's
                    // type
                    auto paramType = param->getType();
                    auto exprType = std::get<Type>(expression);

                    auto paramExprTypeMatchesParamType =
                        (paramType == exprType);
                    SemResult exprIsCastable = castToTarget(exprType, paramType, arg->exprCode, fc.lineNo);

                    if (!exprIsCastable.isSuccess()) {
                        errors.push_back(
                            "Error in line number: " +
                            std::to_string(arg->exprCode->lineNo) +
                            " . The function parameter number: " +
                            std::to_string(i) + " type does not match");
                    }
                }
            }
        }

        if (startingSize != errors.size()) {
            return SemResult::Err;
        }

        auto fnReturnType = fnScope->functions[fc.functionName].return_type->as<idNodeType>().id;
        return SemResult::ok(fnReturnType);
    } 
    /* 
      Entry point, we get a list of statements & iterate over each of them & make sure they are
      Semantically correct
    */
    SemResult operator()(StatementList& sl) {
        SemResult ret = SemResult::Ok;
        for(const auto& statement: sl.statements) {
            auto statementError = semantic_analysis(statement);

            /* 
              If the child statement returned an error, convert the result
              we'll return to an error if it's not already, then add all child
              statement errors to the returned result.
            */

            if(auto* e = std::get_if<ErrorList>(&statementError); e) {
                if(ret.isSuccess()) {
                    ret = statementError;
                }
                ret.mergeErrors(*e);
            }
        }

        for(const auto& [name, entry]: currentNodePtr->currentScope->sym2) {
            if(!entry.isUsed) {
                warnings.push_back(
                    "Warning in line number: " +
                    std::to_string(entry.declaredAtLine) + " .Variable " +
                    name + " is declared but never used"
                );
            }
        }

        return ret;
    }
    
    SemResult operator()(doWhileNodeType& dw) {
        int startingSize = errors.size();

        auto condition = semantic_analysis(dw.condition);
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

        auto loop = semantic_analysis(dw.loop_body); 
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
        auto condition = semantic_analysis(w.condition);
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
        auto loop = semantic_analysis( w.loop_body);
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

        auto init_statement = semantic_analysis( f.init_statement);
        if (!init_statement.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(f.init_statement->lineNo) +
                    " .The initialization statement in the for loop is not valid");
        }

        auto condition = semantic_analysis(f.loop_condition);
        if (!condition.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(f.loop_condition->lineNo) +
                    " .The condition in the for loop is not valid");
        } else {
            auto conditionType = std::get<Type>(condition);
            if (conditionType != "bool") {
                SemResult conditionCastResult = castToTarget(conditionType, Type("bool"), f.loop_condition, f.loop_condition->lineNo);

                if (!conditionCastResult.isSuccess()) {
                    error(conditionCastResult);
                }
            }
        }

        auto loop = semantic_analysis(f.loop_body);
        if (!loop.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(f.loop_body->lineNo) +
                    " .The body in the for loop is not valid");
        }

        auto post_statement = semantic_analysis(f.post_loop_statement);
        if (!post_statement.isSuccess()){
            errors.push_back("Error in line number: " +
                    std::to_string(f.post_loop_statement->lineNo) +
                    " .The post body statement in the for loop is not valid");
        }

        if(startingSize != errors.size()) { return SemResult::Err; }             
        return condition;
    }
           

    SemResult operator()(BinOp& bop) const {
      /* 
       * TODO: Implement the followinging check when the function identifier is added
       to the symbol table
       * Check that the right expression is not a function identifier but a function call *
       * Check that the left identifier is not a function *

       ! Do some operator specific checks !
       */

      int startingSize = errors.size();

      /* Check that the left expression is valid (identifier is declared) */
      LEFT_VALID(bop.lOperand); // * gives left

      /* Check that the right expression is semantically valid */
      RIGHT_VALID(bop.rOperand); // * gives right

      /*  Check that the two expressions on the left & on the right are of the
       * same type */
      LEFT_SAME_TYPE_AS_RIGHT(
          left, right, bop.lOperand->lineNo,
          bop); // * gives leftType & rightType & finalType

      auto setIdNodeAsUsed =
        [currentScope = currentNodePtr->currentScope](Node *ptr) {
          ptr->currentScope = currentScope;

          if (auto *idNode = ptr->asPtr<idNodeType>(); idNode) {
            auto nodeSymEntry = getSymEntry(idNode->id, ptr->currentScope);
            nodeSymEntry.isUsed = true;
          }
        };

      /* Check that the left and right are either both integers or both float
      */
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
          if (uop.operand != nullptr) return semantic_analysis(uop.operand);
      } break;

      case Minus:
      case Plus: {
          return semantic_analysis(uop.operand);
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

        SemResult condition = semantic_analysis(ifNode.condition);

        if (!condition.isSuccess()) {
            errors.push_back(
                "Error in line number: " + std::to_string(ifNode.condition->lineNo) +
                " .Condition in an if statement is invalid"
            );
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
            SemResult alwaysFalseResult = ex_const_kak_TM(ifNode.condition);

            if (alwaysFalseResult.isSuccess() &&
                std::get<Type>(alwaysFalseResult) == "bool") {
                Value val = *alwaysFalseResult.value;

                if (bool(val) == false) {
                    warnings.push_back(
                        "Warning in line number: " +
                        std::to_string(ifNode.condition->lineNo) +
                        " .Condition in an if statement is always false"
                    );
                }
            }
        }

        SemResult ifBody = semantic_analysis(ifNode.ifCode);
        if (!ifBody.isSuccess()) {
            errors.push_back(
                "Error in line number: " + std::to_string(ifNode.ifCode->lineNo) +
                " .Body of an if statement is invalid"
            );
        }

        if (ifNode.elseCode != nullptr) {
            SemResult elseBody = semantic_analysis(ifNode.elseCode);
            if (!ifBody.isSuccess()) {
                errors.push_back(
                    "Error in line number: " +
                    std::to_string(ifNode.elseCode->lineNo) +
                    " .Body of an if statement is invalid"
                );
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
            SemResult exprResult = semantic_analysis(exp);

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
        auto arrayExprResult = semantic_analysis(ai.arrayExpr);

        if(!arrayExprResult.isSuccess()) {
            errors.push_back(lineError("Invalid array expression.", currentLineNo));
        } else {
            auto arrayType = std::get<Type>(arrayExprResult);
            if(arrayType.isArray != Type::IsArray::Yes) {
                errors.push_back(lineError("Cannot index a non-array.", currentLineNo));
            }
        }


        auto indexExprResult = semantic_analysis(ai.indexExpr);
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

SemResult semantic_analysis(Node *p) {    
    if (p == nullptr) return SemResult::Ok;
    return std::visit(semantic_analysis_visitor{p}, *p);
}
