//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>

class binaryExpressionNode : public expressionNode {
public:
    binaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _l, std::shared_ptr<expressionNode> _r) :
        left{std::move(_l)}, right{std::move(_r)} {
        type = _type;
        _operator = _op;
    }
    expressionNode* getRight() {return right.get();};
    expressionNode* getLeft() {return left.get();};
private:
    std::shared_ptr<expressionNode> left;
    std::shared_ptr<expressionNode> right;
};

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
