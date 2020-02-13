//
// Created by hu on 11/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP

#include "statementNode.hpp"
#include <nodes/expressions/expressionNodes.hpp>
#include <string>

class assignNode : public statementNode {
public:
    assignNode (Type t, std::string name, std::shared_ptr<expressionNode> n) : name{std::move(name)}, expr{std::move(n)} {
        setType(t);
        setNodeType(Assign);
    };

    std::string getName() const {return name;}
    expressionNode* getExpr() const {return expr.get();}
    std::string to_string() override {

        return nameToTikzName(name) + " = " + expr->to_string();
    }
private:
    std::string name;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
