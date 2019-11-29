//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>
#include <nodes/literalNode.hpp>
#include <nodes/statements/readNode.hpp>



class binaryExpressionNode : public expressionNode {
public:
    binaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _l, std::shared_ptr<expressionNode> _r) :
        left{std::move(_l)}, right{std::move(_r)}, _operator{_op} {
        type = _type;
        setNodeType(BinaryExpression);
    }

    binaryExpressionNode(std::shared_ptr<binaryExpressionNode> n) : left{(*n).left}, right{(*n).right}, _operator{(*n)._operator} {
        type = n->getType();
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
/*
static std::map< NodeType, std::type_info > binExprs = {
        {Read, typeid(readNode) },
        {Literal, typeid(literalNode)},
        {BinaryExpression, typeid(binaryExpressionNode)},
};*/

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
