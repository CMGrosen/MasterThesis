//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP

#include "binaryExpressionNode.hpp"

class subtractionNode : public  binaryExpressionNode{
public:
    subtractionNode(node l, node r) : binaryExpressionNode("-", l, r){};
};
#endif //ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP