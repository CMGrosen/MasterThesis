//
// Created by hu on 19/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP

#include "statementNode.hpp"
#include <nodes/expressions/expressionNodes.hpp>
#include <string>

class arrayFieldAssignNode : public statementNode {
public:
    arrayFieldAssignNode (Type t, std::shared_ptr<arrayAccessNode> arrField, std::shared_ptr<expressionNode> n) : field{std::move(arrField)}, expr{std::move(n)} {
        setType(t);
        setNodeType(AssignArrField);
        name = field->getName();
    };

    arrayAccessNode* getField() const {return field.get();}
    expressionNode* getExpr() const {return expr.get();}
    std::string getName() const {return name;}

    std::string to_string() override {
        return nameToTikzName(field->to_string() + " = " + expr->to_string(), onSSA);
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _fieldExpr = field->copy_expression();
        std::shared_ptr<arrayAccessNode> _field = std::make_shared<arrayAccessNode>(arrayAccessNode(field->getType(), _fieldExpr, name));
        std::shared_ptr<expressionNode> _expr = expr->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(type, _field, _expr));
        _this->setSSA(onSSA);
        return _this;
    }
private:
    std::string name;
    std::shared_ptr<arrayAccessNode> field;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP
