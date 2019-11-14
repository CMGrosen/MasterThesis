//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP

#include <nodes/node.hpp>

enum op { PLUS, MINUS, MULT, DIV, MOD, NOT, AND, OR, LE, LEQ, GE, GEQ, EQ, NEQ, NEG, NOTUSED};

class expressionNode : public node {
public:
    virtual op getOperator() { return _operator;}

protected:
    op _operator;
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP