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
    binaryExpressionNode(Type _type, op _op) : _operator{_op} {
        type = _type;
        setNodeType(BinaryExpression);
    }

    op getOperator() const {return _operator;};

private:
    op _operator;
};
/*
static std::map< NodeType, std::type_info > binExprs = {
        {Read, typeid(readNode) },
        {Literal, typeid(literalNode)},
        {BinaryExpression, typeid(binaryExpressionNode)},
};*/

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
