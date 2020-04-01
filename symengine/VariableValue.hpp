//
// Created by hu on 31/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_VARIABLEVALUE_HPP
#define ANTLR_CPP_TUTORIAL_VARIABLEVALUE_HPP

#include <nodes/expressions/variableNode.hpp>
#include <string>
#include <nodes/node.hpp>

struct VariableValue : virtual public variableNode {
    std::string value;

    VariableValue(Type type, std::string name, std::string variableName, std::string val)
        : variableNode(type, std::move(name), std::move(variableName)), value{std::move(val)} {}
};

#endif //ANTLR_CPP_TUTORIAL_VARIABLEVALUE_HPP