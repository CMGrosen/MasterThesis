//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_LITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_LITERALNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class literalNode : public expressionNode {
public:
    literalNode(int a) {
        value = a;
        type = Type::intType;
    };

    int value;
};
#endif //ANTLR_CPP_TUTORIAL_LITERALNODE_HPP