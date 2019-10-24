//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_DIVISIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_DIVISIONNODE_HPP


#include "binaryExpressionNode.hpp"

class divisionNode : binaryExpressionNode {

    divisionNode(node l, node r) : binaryExpressionNode("/", l, r){};

};
#endif //ANTLR_CPP_TUTORIAL_DIVISIONNODE_HPP