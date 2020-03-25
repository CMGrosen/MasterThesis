//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/expressions/expressionNode.hpp>

class binaryExpressionNode : virtual public expressionNode {
public:
    binaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _left, std::shared_ptr<expressionNode> _right)
    : _operator{_op}, left{std::move(_left)}, right{std::move(_right)} {
        type = _type;
        setNodeType(BinaryExpression);
    }

    std::string to_string() const override {
        return left->to_string() + " " + operatorToString[_operator] + " " + right->to_string();
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<binaryExpressionNode>(binaryExpressionNode(type, _operator, left->copy_expression(), right->copy_expression()));
        _this->setSSA(onSSA);
        return _this;
    }
    op getOperator() const {return _operator;};

    void setSSA(bool t) override {
        onSSA = t;
        left->setSSA(t);
        right->setSSA(t);
    }

    bool operator==(const expressionNode *expr) const override {
        if (expr->getNodeType() == BinaryExpression) {
            if (auto binNode = dynamic_cast<const binaryExpressionNode*>(expr)) {
                return binNode->getOperator() == _operator && binNode->getLeft() == left.get() && binNode->getRight() == right.get();
            }
        }
        return false;
    }

    expressionNode *getLeft() const {return left.get();}
    expressionNode *getRight() const {return right.get();}

private:
    op _operator;
    std::shared_ptr<expressionNode> left;
    std::shared_ptr<expressionNode> right;
};
/*
static std::map< NodeType, std::type_info > binExprs = {
        {Read, typeid(readNode) },
        {Literal, typeid(literalNode)},
        {BinaryExpression, typeid(binaryExpressionNode)},
};*/

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
