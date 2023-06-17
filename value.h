#pragma once

#include <algorithm>
#include <cassert>
#include <variant>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

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

// TODO: Add `undefined` as a type to indicate uninitialized variables
#define PRIMITIVE_TYPES int, float, bool, char, std::string

struct Value;
using PrimitiveArray = std::vector<Value>;

struct Value : std::variant<PRIMITIVE_TYPES, PrimitiveArray> { 

    Value(): std::variant<PRIMITIVE_TYPES, PrimitiveArray>(0) {}

    template<typename SubType>
    Value(SubType s): std::variant<PRIMITIVE_TYPES, PrimitiveArray>(s) {}

    Value operator=(const struct ControlFlow& cf);

    // For use in `if`, `while`, `for`, etc...
    explicit operator bool() const {
        return std::visit(
            Visitor {
                [](std::string sValue)  { std::cerr << "No conversion from string to bool, returning false\n"; return false; },
                [](PrimitiveArray vv)   { return !vv.empty(); },
                [](auto arg)            { return (bool)arg; }
            },
            *this
        );
    }

    explicit operator std::string() const {
        using namespace std::string_literals;
        return std::visit(
            Visitor {
                    [](int iValue)         { return std::to_string(iValue); },
                    [](float fValue)       { return std::to_string(fValue); },
                    [](bool bValue)        { return bValue? "true"s: "false"s; },
                    [](char cValue)        { return std::string(1, cValue); },
                    [](std::string sValue) { return sValue; },
                    [](PrimitiveArray vv)    { 
                        std::stringstream ss;
                        ss << '[';
                        for(int i = 0; i < vv.size(); i++) {
                            ss << std::string(vv[i]);
                            if(i < vv.size() - 1) { ss << ", "; }
                        };
                        ss << ']';

                        return ss.str();
                    }
            },
            *this
        );
    }

    bool isLiteral() const {
        return std::visit(
            Visitor {
                [](int iValue)         { return true; },
                [](float fValue)       { return true; },
                [](bool bValue)        { return true; },
                [](char cValue)        { return true; },
                [](std::string sValue) { return false; },
                [](PrimitiveArray vv) {
                    return std::all_of(vv.begin(), vv.end(), [](const Value& p) { return p.isLiteral(); });
                }
            },
            *this
        );
    }

    std::string getType() const {
        using namespace std::string_literals;
        return std::visit(
            Visitor {
                [](int iValue)         { return "int"s; },
                [](float fValue)       { return "float"s; },
                [](bool bValue)        { return "bool"s; },
                [](char cValue)        { return "char"s; },
                [](std::string sValue) { return "string"s; }, 
                [](PrimitiveArray vv)   {

                    // FIXME: What to do when we have an empty array?
                    assert(!vv.empty());
                    
                    bool allElementsSameType = true;
                    int firstElementTypeIndex = vv.front().index();
                    for(int i = 1; i < vv.size(); i++) {
                        if(vv[i].index() != firstElementTypeIndex) {
                            allElementsSameType = false;
                            break;
                        }
                    }

                    assert(allElementsSameType);

                    return vv.front().getType();
                }
            },
            *this
        );
    }

    const Value& operator[](int n) const {
        const auto* vv = std::get_if<PrimitiveArray>(this);
        assert(n < vv->size());
        return (*vv)[n];
    }

    void setIndex(int n, Value val) {
        auto* vv = std::get_if<PrimitiveArray>(this);
        assert(n < vv->size());
        (*vv)[n] = val;
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
