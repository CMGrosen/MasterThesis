//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_MULTIPLICATIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_MULTIPLICATIONNODE_HPP


#include "binaryExpressionNode.hpp"

class multiplicationNode : binaryExpressionNode {

    multiplicationNode(node l, node r) : binaryExpressionNode("*", l, r){};

};
#endif //ANTLR_CPP_TUTORIAL_MULTIPLICATIONNODE_HPP