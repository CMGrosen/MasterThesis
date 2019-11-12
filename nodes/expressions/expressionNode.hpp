//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP

#include <nodes/node.hpp>

class expressionNode : public node {
public:
    virtual std::string getOperator() { return _operator;}

protected:
    std::string _operator;
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP