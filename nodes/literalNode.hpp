//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_LITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_LITERALNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class literalNode : public expressionNode {
public:
    literalNode(std::string a) : value{std::move(a)} {
        if (value == "true" || value == "false") {
            setType(boolType);
        } else {
            setType(intType);
        }
    };

    std::string value;
};
#endif //ANTLR_CPP_TUTORIAL_LITERALNODE_HPP