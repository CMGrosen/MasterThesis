//
// Created by hu on 16/04/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_VALUE_HPP
#define ANTLR_CPP_TUTORIAL_VALUE_HPP

#include <string>
#include <memory>
#include <src/nodes/statements/statementNode.hpp>

struct Value {
    std::string val;
    std::shared_ptr<statementNode> statement;
    Type type;
    Value (std::string val, std::shared_ptr<statementNode> statement, Type type) : val{std::move(val)}, statement{std::move(statement)}, type{type} {}

};

#endif //ANTLR_CPP_TUTORIAL_VALUE_HPP
