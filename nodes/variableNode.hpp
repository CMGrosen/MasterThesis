//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_NAMENODE_HPP
#define ANTLR_CPP_TUTORIAL_NAMENODE_HPP

#include "node.hpp"
#include <string>

class variableNode : public node {
public:
    variableNode(Type _type, std::string n) : node(_type, Variable), name{std::move(n)} {};
    std::string name;
    std::string getValue() const override {return name;};
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP