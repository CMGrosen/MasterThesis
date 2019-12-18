//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/node.hpp>
#include <nodes/expressions/literalNode.hpp>
#include <limits>
//#include <nodes/statements/readNode.hpp>



class binaryExpressionNode : public node {
public:
    binaryExpressionNode(op _op, std::string name) :
        node(intType, BinaryExpression, _op),
        left{std::make_shared<node>(node(intType, Variable, std::move(name)))} {
        if (_op == GEQ)
            right = std::make_shared<node>(node(intType, Literal, std::to_string(INT16_MIN)));
        else
            right = std::make_shared<node>(node(intType, Literal, std::to_string(INT16_MAX)));
    }
    binaryExpressionNode(Type _type, op _op, std::string name, std::string otherName) :
            node(_type, BinaryExpression, _op),
            left{std::make_shared<node>(node(_type, Variable, std::move(name)))},
            right{std::make_shared<node>(node(_type, Variable, std::move(otherName)))} {}
    binaryExpressionNode(Type _type, op _op, std::shared_ptr<node> _l, std::shared_ptr<node> _r) :
        node(_type, BinaryExpression, _op),
        left{std::move(_l)}, right{std::move(_r)} {}

    binaryExpressionNode(std::shared_ptr<binaryExpressionNode> n) : node(n->getType(), BinaryExpression, n->_operator),
    left{(*n).left}, right{(*n).right} {}

    node* getRight() const {return right.get();};
    node* getLeft() const {return left.get();};

private:
    std::shared_ptr<node> left;
    std::shared_ptr<node> right;
};
/*
static std::map< NodeType, std::type_info > binExprs = {
        {Read, typeid(readNode) },
        {Literal, typeid(literalNode)},
        {BinaryExpression, typeid(binaryExpressionNode)},
};*/

#endif //ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
