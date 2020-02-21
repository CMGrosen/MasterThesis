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

    void setName(std::string newName) {name = std::move(newName);}
    std::string getName() const {return name;}
    expressionNode* getExpr() const {return expr.get();}
    std::string to_string() override {
        return nameToTikzName(name) + " = " + expr->to_string();
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _expr = expr->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<assignNode>(assignNode(type, name, _expr));
        return _this;
    }
private:
    std::string name;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
