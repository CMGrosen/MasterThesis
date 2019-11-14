//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_READNODE_HPP
#define ANTLR_CPP_TUTORIAL_READNODE_HPP

#include <nodes/variableNode.hpp>

class readNode : public expressionNode {
public:
    readNode(std::shared_ptr<variableNode> node) : n{std::move(node)} {}
    NodeType getNodeType() { return Read; }

private:
    std::shared_ptr<variableNode> n;
};

#endif //ANTLR_CPP_TUTORIAL_READNODE_HPP
