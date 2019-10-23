//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP

#include "nodes/expressions/expressionNode.hpp"

class subtractionNode : public expressionNode {
public:
    subtractionNode(node l, node r) : expressionNode(l, r) {};
};
#endif //ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP