#pragma once
#include "template_utils.h"
#include "value.h"
#include <algorithm>
#include <variant>
#include <iostream>
#include <vector>
#include <type_traits>

// auto success = Result::Success("int");
// auto error  = Result::Error("LHS = int, RHS = string");
// error.push_back("another error!");

template<typename S = std::string, typename E = std::vector<std::string>>
struct Result: std::variant<S, E> {
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

    Result& mergeErrors(const E& errors) {
        for(const auto& err: errors) {
            this->addError(err);
        }

        return *this;
    }

    static Result<S,E> mergeResults(std::vector<Result<S,E>>& results) {
        auto errors = Utils::filter(results, [](Result& res) { return !res.isSuccess(); });

        if(errors.empty()) { return Result::Ok; }

        Result<S,E> final = Result<S,E>::Err;
        for(const auto& err: errors) { final.mergeErrors(std::get<E>(err)); }

        return final;
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
