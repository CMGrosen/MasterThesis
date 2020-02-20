//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>

class binaryExpressionNode : public expressionNode {
public:
    binaryExpressionNode(Type _type, op _op) : _operator{_op} {
        type = _type;
        setNodeType(BinaryExpression);
    }

    std::string to_string() override {
        std::string res = operatorToString[_operator] + " ";
        if (getNext()) {
            res += getNext()->to_string();
        }
        return res;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<binaryExpressionNode>(binaryExpressionNode(type, _operator));
        _this->setNext(this->copy_next());
        return _this;
    }
    op getOperator() const {return _operator;};

    bool operator==(const expressionNode *expr) const override {
        return (nodetype == expr->getNodeType() && _operator == dynamic_cast<const binaryExpressionNode*>(expr)->_operator);
    }
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
