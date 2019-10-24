//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>

class binaryExpressionNode : public expressionNode {
public:
    binaryExpressionNode(std::string _operator, node& _l, node& _r) : expressionNode(_operator) , left{_l}, right{_r} {};

    node getRight() {return right;};
    node getLeft() {return left;};
protected:
    node left;
    node right;
};

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
