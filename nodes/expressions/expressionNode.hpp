//
// Created by hu on 23/10/2019.
//

#include <nodes/node.hpp>

#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP


class expressionNode : public node {
public:
    virtual std::string getOperator() { return _operator;}

protected:
    std::string _operator;
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP