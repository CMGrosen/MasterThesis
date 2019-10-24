//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ADDITIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_ADDITIONNODE_HPP

#include "binaryExpressionNode.hpp"

class additionNode : public binaryExpressionNode {

    additionNode(node l, node r) : binaryExpressionNode("+", l, r){};

};

#endif //ANTLR_CPP_TUTORIAL_ADDITIONNODE_HPP