//
// Created by hu on 23/10/2019.
//

#include "node.hpp"

#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP


class expressionNode : public node {
public:
    expressionNode() {};
    expressionNode(std::string &v) : self{v} {};
    expressionNode(node& l, node& r) : left{l}, right{r} {};
    virtual ~expressionNode() {};
    node getRight() {return right;};
    node getLeft() {return left;};



protected:
    node left;
    node right;
    std::string self;
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP