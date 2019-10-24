//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_MODULONODE_HPP
#define ANTLR_CPP_TUTORIAL_MODULONODE_HPP


#include "binaryExpressionNode.hpp"

class moduloNode : binaryExpressionNode {
moduloNode(node l, node r) : binaryExpressionNode("%", l, r){};

};
#endif //ANTLR_CPP_TUTORIAL_MODULONODE_HPP