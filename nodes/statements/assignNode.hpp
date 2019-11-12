//
// Created by hu on 11/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP

#define  ASSINGNMENT std::hash<std::string>{}("assingment")

#include "statementNode.hpp"
#include <nodes/expressions/expressionNodes.hpp>
#include <string>
class assignNode : public statementNode {
public:
    assignNode (std::string name, std::shared_ptr<expressionNode> n) : name{std::move(name)}, expr{std::move(n)} {};
    uint getNodeType() override { return ASSINGNMENT; }
private:
    std::string name;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
