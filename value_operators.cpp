#include "cl.h"
#include <variant>
#include <string>

// Here we overload all the needed operators so that `Value` can be used in the interpreter freely.
// Things needed to overload are:
//
//      Define the operator as:
//          Value operator+(Value a, Value b) {
//              return std::visit(BinaryPlusVisitor(), a, b);
//          }
//
//      Now, we need to implement `BinaryPlusVisitor`, it should look something similar to this:
//          struct BinaryPlusVisitor {
//              // Error case: int + string:
//              // Notice that you _must_ return a value, all overloads must return the same type.
//              Value operator()(int i, std::string s) {  /* print error message */  return Value(0); }
//
//              // Special case: char + char
//              // Let's say you want to handle this by concatenating them
//              Value operator()(char c1, char c2) { return Value(""s + c1 + c2); }
//          
//              // Default case, any unhandled cases will fall here. 
//              // (float + int, int + float, int + char, etc...)
//              // Might cause compilation errors if `arg1 + arg2` is invalid.
//              template <typename T, typename U>                                                
//              Value operator()(T arg1, U arg2) { return Value(arg1 + arg2); } 
//          };
//
//  This is the simplest way to go about this. All the other vile concoctions in this file
//  were written by me "in the zone" so I don't expect even myself to understand this.
//  They were mostly slapped together by me to reduce duplication. The expanded version
//  is better for ease of reading and debugability, so feel free to expand the macros as you wish!

#define ERR_MSG_BIN(type1, type2) \
    std::cerr << "Invalid binary operator operands: " << type1 << " and " << type2 << '\n'

#define ERR_MSG_UN(type1) \
    std::cerr << "Invalid unary operator operand: " << type1 << '\n'

#define BINARY_OPERATOR_DEFN_BEGIN(name) \
    struct ValueVisitor##name { 

#define BINARY_OPERATOR_DEFN_END(oper, return_type)                                      \
        template <typename T, typename U>                                                \
        return_type operator()(T arg1, U arg2) { return return_type(arg1 oper arg2); }   \
    };                                                                      

#define UNARY_OPERATOR_DEFN_BEGIN(name) \
    struct ValueVisitor##name { 

#define UNARY_OPERATOR_DEFN_END(oper, return_type)                          \
        template <typename T>                                               \
        return_type operator()(T arg1) { return return_type(oper (arg1)); } \
    };                                                                      

#define ERROR_CASE_BINARY_DIFF_TYPES(type1, type2, return_type) \
    return_type operator()(type1 arg1, type2 arg2) {   ERR_MSG_BIN(#type1, #type2); return return_type(0); } \
    return_type operator()(type2 arg1, type1 arg2) {   ERR_MSG_BIN(#type2, #type1); return return_type(0); } 

#define ERROR_CASE_BINARY_SAME_TYPE(type, return_type) \
    return_type operator()(type arg1, type arg2) {   ERR_MSG_BIN(#type, #type); return return_type(0); }

#define ERROR_CASE_UNARY(type, return_type) \
    return_type operator()(type arg1) {   ERR_MSG_UN(#type); return return_type(0); }

#define ARITHMETIC_ERROR_CASES                              \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, int, Value)   \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, float, Value) \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, bool, Value)  \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, char, Value)  \
    ERROR_CASE_BINARY_SAME_TYPE(std::string, Value)         

// Bitwise operators is defined only for integers, booleans, and characters.
#define BITWISE_ERROR_CASES                                 \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, int, Value)   \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, bool, Value)  \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, char, Value)  \
    ERROR_CASE_BINARY_DIFF_TYPES(float, int, Value)         \
    ERROR_CASE_BINARY_DIFF_TYPES(float, bool, Value)        \
    ERROR_CASE_BINARY_DIFF_TYPES(float, char, Value)        \
    ERROR_CASE_BINARY_DIFF_TYPES(float, std::string, Value) \
    ERROR_CASE_BINARY_SAME_TYPE(std::string, Value)         \
    ERROR_CASE_BINARY_SAME_TYPE(float, Value)

#define EQUALITY_ERROR_CASES \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, int, bool)     \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, float, bool)   \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, char, bool)    \
    ERROR_CASE_BINARY_DIFF_TYPES(std::string, bool, bool)    

#define LOGICAL_ERROR_CASES \
    EQUALITY_ERROR_CASES    \
    ERROR_CASE_BINARY_SAME_TYPE(std::string, bool)

#define INT_ERROR_CASES                              \
    ARITHMETIC_ERROR_CASES                           \
    ERROR_CASE_BINARY_DIFF_TYPES(float, int, Value)  \
    ERROR_CASE_BINARY_DIFF_TYPES(float, char, Value) \
    ERROR_CASE_BINARY_DIFF_TYPES(float, bool, Value) \
    ERROR_CASE_BINARY_SAME_TYPE(float, Value)


BINARY_OPERATOR_DEFN_BEGIN(Plus)
    ERROR_CASE_BINARY_DIFF_TYPES(int, std::string, Value)
    ERROR_CASE_BINARY_DIFF_TYPES(float, std::string, Value)
    ERROR_CASE_BINARY_DIFF_TYPES(bool, std::string, Value)
    ERROR_CASE_BINARY_DIFF_TYPES(char, std::string, Value)
    ERROR_CASE_BINARY_SAME_TYPE(std::string, Value)
BINARY_OPERATOR_DEFN_END(+, Value)

BINARY_OPERATOR_DEFN_BEGIN(Minus)    ARITHMETIC_ERROR_CASES BINARY_OPERATOR_DEFN_END(-, Value)
BINARY_OPERATOR_DEFN_BEGIN(Multiply) ARITHMETIC_ERROR_CASES BINARY_OPERATOR_DEFN_END(*, Value)
BINARY_OPERATOR_DEFN_BEGIN(Divide)   ARITHMETIC_ERROR_CASES  BINARY_OPERATOR_DEFN_END(/, Value)

BINARY_OPERATOR_DEFN_BEGIN(BitwiseAnd) BITWISE_ERROR_CASES BINARY_OPERATOR_DEFN_END(&, Value)
BINARY_OPERATOR_DEFN_BEGIN(BitwiseOr)  BITWISE_ERROR_CASES BINARY_OPERATOR_DEFN_END(|, Value)
BINARY_OPERATOR_DEFN_BEGIN(BitwiseXor) BITWISE_ERROR_CASES BINARY_OPERATOR_DEFN_END(^, Value)
UNARY_OPERATOR_DEFN_BEGIN(BitwiseNot)
    ERROR_CASE_UNARY(float, Value)
    ERROR_CASE_UNARY(std::string, Value)
UNARY_OPERATOR_DEFN_END(~, Value);

BINARY_OPERATOR_DEFN_BEGIN(GT)  LOGICAL_ERROR_CASES  BINARY_OPERATOR_DEFN_END(> , bool)
BINARY_OPERATOR_DEFN_BEGIN(LT)  LOGICAL_ERROR_CASES  BINARY_OPERATOR_DEFN_END(< , bool)
BINARY_OPERATOR_DEFN_BEGIN(And) LOGICAL_ERROR_CASES  BINARY_OPERATOR_DEFN_END(&&, bool)
BINARY_OPERATOR_DEFN_BEGIN(Or)  LOGICAL_ERROR_CASES  BINARY_OPERATOR_DEFN_END(||, bool)
BINARY_OPERATOR_DEFN_BEGIN(LE)  LOGICAL_ERROR_CASES  BINARY_OPERATOR_DEFN_END(<=, bool)
BINARY_OPERATOR_DEFN_BEGIN(GE)  LOGICAL_ERROR_CASES  BINARY_OPERATOR_DEFN_END(>=, bool)
BINARY_OPERATOR_DEFN_BEGIN(EQ)  EQUALITY_ERROR_CASES  BINARY_OPERATOR_DEFN_END(==, bool)
BINARY_OPERATOR_DEFN_BEGIN(NE)  EQUALITY_ERROR_CASES  BINARY_OPERATOR_DEFN_END(!=, bool)
UNARY_OPERATOR_DEFN_BEGIN(LogicalNot) ERROR_CASE_UNARY(std::string, bool) UNARY_OPERATOR_DEFN_END(!, bool);

// Shifting and modulo are defined only for integers.
BINARY_OPERATOR_DEFN_BEGIN(LS)     INT_ERROR_CASES  BINARY_OPERATOR_DEFN_END(<<, Value)
BINARY_OPERATOR_DEFN_BEGIN(RS)     INT_ERROR_CASES  BINARY_OPERATOR_DEFN_END(>>, Value)
BINARY_OPERATOR_DEFN_BEGIN(Modulo) INT_ERROR_CASES  BINARY_OPERATOR_DEFN_END(%, Value)

UNARY_OPERATOR_DEFN_BEGIN(UnaryMinus) ERROR_CASE_UNARY(std::string, Value); UNARY_OPERATOR_DEFN_END(-, Value);
UNARY_OPERATOR_DEFN_BEGIN(UnaryPlus) ERROR_CASE_UNARY(std::string, Value); UNARY_OPERATOR_DEFN_END(+, Value);

#define BINARY_OPERATOR(oper, name)                    \
    Value operator oper (Value a, Value b) {           \
        return std::visit(ValueVisitor##name(), a, b); \
    }

#define UNARY_OPERATOR(oper, name)                     \
    Value operator oper (Value a) {                    \
        return std::visit(ValueVisitor##name(), a);    \
    }

#define LOGICAL_BINARY_OPERATOR(oper, name)            \
    bool operator oper (Value a, Value b) {            \
        return std::visit(ValueVisitor##name(), a, b); \
    }

#define LOGICAL_UNARY_OPERATOR(oper, name)             \
    bool operator oper (Value a) {                     \
        return std::visit(ValueVisitor##name(), a);    \
    }

BINARY_OPERATOR(+, Plus)
BINARY_OPERATOR(-, Minus)
BINARY_OPERATOR(*, Multiply)
BINARY_OPERATOR(/, Divide)

BINARY_OPERATOR(&, BitwiseAnd)
BINARY_OPERATOR(|, BitwiseOr)
BINARY_OPERATOR(^, BitwiseXor)
UNARY_OPERATOR(~, BitwiseNot)

LOGICAL_BINARY_OPERATOR(>, GT)
LOGICAL_BINARY_OPERATOR(<, LT)
LOGICAL_BINARY_OPERATOR(&&, And)
LOGICAL_BINARY_OPERATOR(||, Or)
LOGICAL_BINARY_OPERATOR(<=, LE)
LOGICAL_BINARY_OPERATOR(>=, GE)
LOGICAL_BINARY_OPERATOR(==, EQ)
LOGICAL_BINARY_OPERATOR(!=, NE)
LOGICAL_UNARY_OPERATOR(!, LogicalNot)

BINARY_OPERATOR(<<, LS)
BINARY_OPERATOR(>>, RS)
BINARY_OPERATOR(%, Modulo)

UNARY_OPERATOR(+, UnaryPlus)
UNARY_OPERATOR(-, UnaryMinus)

std::ostream& operator<<(std::ostream& os, const Value& v) {
    return os << v.toString();
}

Value::Value(const struct ControlFlow& cf) : Value(cf.val){ } 

Value Value::operator=(const struct ControlFlow& cf) {
    return Value(cf);
}