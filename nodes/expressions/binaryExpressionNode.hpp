//
// Created by CMG on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H
#define ANTLR_CPP_TUTORIAL_BINARYEXPRESSIONNODE_H

#include <nodes/node.hpp>
#include <nodes/expressions/literalNode.hpp>
//#include <nodes/statements/readNode.hpp>



class binaryExpressionNode : public node {
public:
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
