//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_WRITENODE_HPP
#define ANTLR_CPP_TUTORIAL_WRITENODE_HPP

#include <nodes/expressions/expressionNode.hpp>
#include "statementNode.hpp"

class writeNode : public statementNode {
public:
    writeNode(std::shared_ptr <expressionNode> node) : n{std::move(node)} {}

    NodeType getNodeType() { return Write; }

private:
    std::shared_ptr <expressionNode> n;
};

#endif //ANTLR_CPP_TUTORIAL_WRITENODE_HPP
