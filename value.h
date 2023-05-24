#include <variant>
#include <string>
#include <iostream>


// https://en.cppreference.com/w/cpp/utility/variant/visit
// Needed so you can use std::visitor ergonomically like so:
//      std::visit(
//          conVisitor {
//              [](variant_subtype_1 a) { // do something with a; return 0; }
//              [](variant_subtype_2 a) { // do something with a; return 0; }
//              [](auto a) { // default case }
//          }
//          ,variant
//      );
// Note that this style of subtype resolution doesn't play well with
// recursive functions (see set_break_parent or ex_visitor for a workaround).

template<class... Ts>
struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

using ValueVariant = std::variant<int, float, bool, char, std::string>;
struct Value : ValueVariant { 
    // Explicit constructor to avoid implicit conversions (caused issues when implementing operators)
    // Implicit conversion `return 0;` being valid in a function that returns `Value`. (the 0 is converted to Value{0})
    explicit Value(): ValueVariant(0) {}
    explicit Value(int iVal): ValueVariant(iVal) {}
    explicit Value(float fVal): ValueVariant(fVal) {}
    explicit Value(bool bVal): ValueVariant(bVal) {}
    explicit Value(char cVal): ValueVariant(cVal) {}
    explicit Value(std::string sVal): ValueVariant(sVal) {}

    // For use in `if`, `while`, `for`, etc...
    explicit operator bool() const {
        return std::visit(
            Visitor {
                    [](bool bValue)        { return bValue; },
                    [](std::string sValue) { std::cerr << "No conversion from string to bool, returning false\n"; return false; },
                    [](auto arg)           { return (bool)arg; }
            },
            *this
        );
    }

    std::string toString() const {
        using namespace std::string_literals;
        return std::visit(
            Visitor {
                    [](int iValue)         { return std::to_string(iValue); },
                    [](float fValue)       { return std::to_string(fValue); },
                    [](bool bValue)        { return bValue? "true"s: "false"s; },
                    [](char cValue)        { return std::string(1, cValue); },
                    [](std::string sValue) { return sValue; },
            },
            *this
        );
    }

    bool isLiteral() {
        return std::visit(
            Visitor {
                    [](int iValue)         { return true; },
                    [](float fValue)       { return true; },
                    [](bool bValue)        { return true; },
                    [](char cValue)        { return true; },
                    [](std::string sValue) { return false; },
            },
            *this
        );
    }

    std::string getType() const {
        return std::visit(
            Visitor {
                    [](int iValue)         { return "int"; },
                    [](float fValue)       { return "float"; },
                    [](bool bValue)        { return "bool"; },
                    [](char cValue)        { return "char"; },
                    [](std::string sValue) { return "string"; },
            },
            *this
        );
    }
};

// Defined in `value_operators.cpp`
Value operator+(Value, Value);
Value operator-(Value, Value);
Value operator*(Value, Value);
Value operator/(Value, Value);

Value operator&(Value, Value);
Value operator|(Value, Value);
Value operator^(Value, Value);
Value operator~(Value);

Value operator<<(Value, Value);
Value operator>>(Value, Value);
Value operator%(Value, Value);

bool operator<(Value, Value);
bool operator>(Value, Value);
bool operator&&(Value, Value);
bool operator||(Value, Value);
bool operator<=(Value, Value);
bool operator>=(Value, Value);
bool operator==(Value, Value);
bool operator!=(Value, Value);
bool operator!(Value);

Value operator+(Value);
Value operator-(Value);

std::ostream& operator<<(std::ostream& os, const Value& v);
