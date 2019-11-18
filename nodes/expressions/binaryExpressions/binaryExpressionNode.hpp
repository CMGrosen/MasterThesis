//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>

class binaryExpressionNode : public expressionNode {
public:
    binaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _l, std::shared_ptr<expressionNode> _r) :
        left{std::move(_l)}, right{std::move(_r)}, _operator{_op} {
        type = _type;
        setNodeType(BinaryExpression);
    }
    expressionNode* getRight() const {return right.get();};
    expressionNode* getLeft() const {return left.get();};
    op getOperator() const {return _operator;};
private:
    std::shared_ptr<expressionNode> left;
    std::shared_ptr<expressionNode> right;
    op _operator;
};

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
