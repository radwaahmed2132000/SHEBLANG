#pragma once
#include "value.h"
#include <variant>
#include <iostream>
#include <vector>

// auto success = Result::Success("int");
// auto error  = Result::Error("LHS = int, RHS = string");
// error.push_back("another error!");

template<typename S = std::string, typename E = std::vector<std::string>>
struct Result: std::variant<S, E> {
    Value * value = nullptr;

    inline static Result<S,E> Err = Result{std::variant<S, E>(E{})};
    inline static Result<S,E> Ok = Result{std::variant<S, E>(S{})};

    bool isSuccess() const {
        return std::holds_alternative<S>(*this);
    }

    static Result ok(S type) {
        return Result{std::variant<S, E>(S{type})};
    }

    static Result err(E error) {
        return Result{std::variant<S, E>(error)};
    }
    
    template<typename I>
    static Result err(I error) {
        return Result{std::variant<S, E>(E{error})};
    }

    Result& addError(std::string newError) {
        if(auto *s = std::get_if<S>(this); s != nullptr) {
            std::cerr << "Cannot add an error to a success variant\n";
        } else if (auto *e = std::get_if<E>(this); e != nullptr) {
            e->push_back(newError);
        }

        return *this;
    }

    Result& mergeErrors(E& errors) {
        for(const auto& err: errors) {
            this->addError(err);
        }

        return *this;
    }

    void print() {
        if(auto *s = std::get_if<S>(this); s != nullptr) {
            std::cout << *s;
        } else if (auto *e = std::get_if<E>(this); e != nullptr) {
            for(auto& innerErr: *e) {
                std::cerr << innerErr << "\n";
            }
        }
    }
};
