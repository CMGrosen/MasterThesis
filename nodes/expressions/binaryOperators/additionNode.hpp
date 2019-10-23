//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ADDITIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_ADDITIONNODE_HPP

#include "nodes/expressions/expressionNode.hpp"

class additionNode : expressionNode {
    expressionNode right;
    expressionNode left;
    Type t;

    additionNode(expressionNode l, expressionNode r) : left{l}, right{r} {};

};

#endif //ANTLR_CPP_TUTORIAL_ADDITIONNODE_HPP