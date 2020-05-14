//
// Created by hu on 31/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_VARIABLEVALUE_HPP
#define ANTLR_CPP_TUTORIAL_VARIABLEVALUE_HPP

#include <src/nodes/expressions/variableNode.hpp>
#include <string>
#include <src/nodes/node.hpp>

struct VariableValue : virtual public variableNode {
    std::string value;
    bool defined;

    VariableValue(Type type, std::string name, std::string variableName, std::string val, bool defined)
        : variableNode(type, std::move(name), std::move(variableName)), value{std::move(val)}, defined{defined} {}
};

#endif //ANTLR_CPP_TUTORIAL_VARIABLEVALUE_HPP
