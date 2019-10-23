//
// Created by hu on 23/10/2019.
//

#include "node.hpp"
#include <string>

#ifndef ANTLR_CPP_TUTORIAL_NAMENODE_HPP
#define ANTLR_CPP_TUTORIAL_NAMENODE_HPP


class nameNode : public node {
public:
    nameNode(std::string n) : name{n} {};
    std::string name;
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP