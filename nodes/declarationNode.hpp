//
// Created by CMG on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_DECLARATIONNODE_H
#define ANTLR_CPP_TUTORIAL_DECLARATIONNODE_H

#include "node.hpp"
#include <string>

class declarationNode : public node {
public:
    std::string name;
    expressionNode value;
    declarationNode(std::string n, expressionNode val) : name{n}, value{val} {node();};
};
#endif //ANTLR_CPP_TUTORIAL_DECLARATIONNODE_H
