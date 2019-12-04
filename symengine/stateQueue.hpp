//
// Created by hu on 04/12/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEQUEUE_HPP
#define ANTLR_CPP_TUTORIAL_STATEQUEUE_HPP

#include <queue>

template <typename T>
struct stateQueue : std::queue<T>{
    stateQueue() : std::queue<T>() {};
    T myPop() {
        T val = std::move(this->front());
        this->pop();
        return val;
    }
};


#endif //ANTLR_CPP_TUTORIAL_STATEQUEUE_HPP
