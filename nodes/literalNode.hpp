//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_LITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_LITERALNODE_HPP


#include "expressionNode.hpp"
class literalNode : public node {
public:
    literalNode(int a) : value{a} {};
    int value;
};
#endif //ANTLR_CPP_TUTORIAL_LITERALNODE_HPP