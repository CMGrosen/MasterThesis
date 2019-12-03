//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_LITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_LITERALNODE_HPP

#include <nodes/node.hpp>

class literalNode : public node {
public:
    literalNode(Type t, std::string a) : node(t, Literal), value{std::move(a)} {}

    std::string getValue() const override {return value;};
    std::string value;
};
#endif //ANTLR_CPP_TUTORIAL_LITERALNODE_HPP