//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>

class binaryExpressionNode : public expressionNode {
public:

    virtual expressionNode* getRight() {return right.get();};
    virtual expressionNode* getLeft() {return left.get();};
protected:
    std::shared_ptr<expressionNode> left;
    std::shared_ptr<expressionNode> right;
};

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
