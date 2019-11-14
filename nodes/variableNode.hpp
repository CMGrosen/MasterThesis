//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_NAMENODE_HPP
#define ANTLR_CPP_TUTORIAL_NAMENODE_HPP

#include "node.hpp"
#include <string>

class variableNode : public expressionNode {
public:
    variableNode(Type _type, std::string n) {
        type = _type;
        name = std::move(n);
    };
    std::string name;
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP