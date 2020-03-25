//
// Created by hu on 11/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP

#include <nodes/statements/statementNode.hpp>
#include <string>

class assignNode : virtual public statementNode {
public:
    assignNode (Type t, std::string _name, std::shared_ptr<expressionNode> n) : name{std::move(_name)}, expr{std::move(n)} {
        origName = name;
        setType(t);
        setNodeType(Assign);
    };

    void setName(std::string newName) {name = std::move(newName);}
    std::string getName() const {return name;}
    std::string getOriginalName() const {return origName;}
    expressionNode* getExpr() const {return expr.get();}
    std::string to_string() const override {
        return nameToTikzName(name, onSSA) + " = " + expr->to_string();
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _expr = expr->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<assignNode>(assignNode(type, origName, _expr));
        _this->setSSA(onSSA);
        dynamic_cast<assignNode*>(_this.get())->setName(name);
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
        expr->setSSA(t);
    }
private:
    std::string origName;
    std::string name;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
