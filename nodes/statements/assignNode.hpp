//
// Created by hu on 11/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP

#include "statementNode.hpp"
#include <nodes/expressions/expressionNodes.hpp>
#include <string>
#include <nodes/variableNode.hpp>

class assignNode : public statementNode {
public:
    assignNode (Type t, std::string name, std::shared_ptr<expressionNode> n) : name{std::move(name)}, expr{std::move(n)} {
        setType(t);
        setNodeType(Assign);
    };
    NodeType getNodeType() override { return Assign; }
    std::string getName() {return name;}
    expressionNode* getExpr() {return expr.get();}

private:
    std::string name;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
