//
// Created by hu on 23/10/2019.
//

#include "node.hpp"
#include <string>

#ifndef ANTLR_CPP_TUTORIAL_NAMENODE_HPP
#define ANTLR_CPP_TUTORIAL_NAMENODE_HPP


class variableNode : public node {
public:
    variableNode(Type _type, std::string n) {
        type = _type;
        name = std::move(n);
    };
    std::string name;
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP