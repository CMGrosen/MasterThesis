//
// Created by hu on 04/12/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_STATESTACK_HPP
#define ANTLR_CPP_TUTORIAL_STATESTACK_HPP

#include <stack>

template <typename T>
struct stateStack : std::stack<T> {
    stateStack() : std::stack<T>() {}

    T myPop() {
        T val = this->top();
        this->pop();
        return std::move(val);
    }
};

#endif //ANTLR_CPP_TUTORIAL_STATESTACK_HPP
