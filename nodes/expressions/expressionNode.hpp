//
// Created by hu on 23/10/2019.
//

#include "../node.hpp"

#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP


class expressionNode : public node {
public:
    expressionNode() {};
    expressionNode(std::string &v) : _operator{v} {};



protected:
    std::string _operator;
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP