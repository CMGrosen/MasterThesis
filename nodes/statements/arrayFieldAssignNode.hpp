//
// Created by hu on 19/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP

#include "statementNode.hpp"
#include <nodes/expressions/expressionNodes.hpp>
#include <string>
#include <nodes/variableNode.hpp>

class arrayFieldAssignNode : public statementNode {
public:
    arrayFieldAssignNode (Type t, std::shared_ptr<expressionNode> arrField, std::shared_ptr<expressionNode> n) : field{std::move(arrField)}, expr{std::move(n)} {
        setType(t);
        setNodeType(AssignArrField);
    };

    expressionNode* getField() const {return field.get();}
    expressionNode* getExpr() const {return expr.get();}
private:
    std::shared_ptr<expressionNode> field;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP
