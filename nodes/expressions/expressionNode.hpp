//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP

#include <nodes/node.hpp>

enum op { PLUS, MINUS, MULT, DIV, MOD, NOT, AND, OR, LE, LEQ, GE, GEQ, EQ, NEQ, NEG, NOTUSED};

class expressionNode : public node {
    std::shared_ptr<expressionNode> _next = nullptr;

public:
    void setNext(std::shared_ptr<expressionNode> n) {_next = std::move(n);}
    std::shared_ptr<expressionNode> getNext() {return _next;}
    std::shared_ptr<expressionNode> getLast() {
        if(!_next) return nullptr;
        auto next = _next;
        while(next->getNext()) next = next->getNext();
        return next;
    }
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP