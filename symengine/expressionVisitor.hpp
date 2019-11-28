//
// Created by CMG on 19/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONVISITOR_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONVISITOR_HPP

#include <iostream>
#include <antlr4-runtime/antlr4-runtime.h>
#include <antlr4-runtime/SmallVisitor.h>
#include <nodes/nodes.hpp>
#include "Constraint.hpp"
#include <DST.h>

class expressionVisistor {
public:
    // uses an expression tree for current state to create a new expression tree for the next state
    std::shared_ptr<expressionNode> visitNextExpression(expressionNode* node, std::map<std::string, constraint> vars){
        expressionNode* changeHere = nullptr;
        if(node->getNodeType() == BinaryExpression){
            binaryExpressionNode* binexpr = (dynamic_cast<binaryExpressionNode*>(node));
            changeHere = getBottomExpression(binexpr, binexpr->getLeft(), binexpr->getLeft(),vars);
        } else if(node->getNodeType() == BinaryExpression) {

        }
        return getNewTree(node, changeHere, vars);
    }

    //finds the next part of the expression to be updated
    expressionNode* getBottomExpression(expressionNode* parent, expressionNode* child_left, expressionNode* child_right, std::map<std::string, constraint> vars){
        switch(child_left->getNodeType()) {
            case BinaryExpression: {
                binaryExpressionNode* binexpr = (dynamic_cast<binaryExpressionNode*>(child_left));
                return getBottomExpression(binexpr, binexpr->getLeft(), binexpr->getRight(), vars);
            }
            case UnaryExpression: {
                unaryExpressionNode* unexpr = (dynamic_cast<unaryExpressionNode*>(child_left));
                return getBottomExpression(unexpr, unexpr->getExpression(), nullptr, vars);
            }
            case Variable: {
                variableNode* var = (dynamic_cast<variableNode*>(child_left));
                if(vars.find(var->name)->second.getRules()[0]->getNodeType() == Literal){
                    return parent;
                } else {
                    // symbolsk
                }
            }
            case Literal: {
                switch(child_right->getNodeType()) {
                    case BinaryExpression: {

                    }
                    case UnaryExpression: {

                    }
                    case Read: {

                    }
                    case ArrayAccess:{

                    }
                }
            }
        }
    }

    // finds all variable names in the expression
    // might not be needed
    std::vector<std::string> getVarNames(expressionNode* node) {
        std::vector<std::string> result;
        if(node->getNodeType() == BinaryExpression){
            binaryExpressionNode* binexpr = (dynamic_cast<binaryExpressionNode*>(node));
            for(auto str : getVarNames(binexpr->getLeft())){
                result.emplace_back(str);
            }
            for(auto str : getVarNames(binexpr->getRight())){
                result.emplace_back(str);
            }
        } else if (node->getNodeType() == UnaryExpression) {
            unaryExpressionNode* unexpr = (dynamic_cast<unaryExpressionNode*>(node));
            for(auto str : getVarNames(unexpr->getExpression())){
                result.emplace_back(str);
            }
        } else if (node->getNodeType() == Variable){
            variableNode* var = (dynamic_cast<variableNode*>(node));
            result.emplace_back(var->name);
        }
        return result;
    }

private:
    //Creates a new expression tree for the new state
    static antlrcpp::Any getNewTree(const expressionNode *tree, const node* updateLocation, const std::map<std::string, constraint>& vars) {
        if(tree != updateLocation){
            switch(tree->getNodeType()) {
                case Read:
                    if (auto a = dynamic_cast<const readNode*>(tree)) {
                        std::shared_ptr<expressionNode> v = getNewTree(a->getVar(), updateLocation, vars);
                        std::shared_ptr<expressionNode> res = std::make_shared<readNode>(readNode(a->getType(),v));
                        return res;
                    }
                    break;
                case Literal:
                    if(auto a = dynamic_cast<const literalNode*>(tree)) {
                        std::shared_ptr<expressionNode> res = std::make_shared<literalNode>(literalNode(a->getType(), a->value));
                        return res;
                    }
                    break;
                case ArrayLiteral:
                    if(auto a = dynamic_cast<const arrayLiteralNode*>(tree)) {
                        std::vector<std::shared_ptr<expressionNode>> nodes;
                        for (auto &i : a->getArrLit()) {
                            std::shared_ptr<expressionNode> inter = getNewTree(i.get(), updateLocation, vars);
                            nodes.push_back(inter);
                        }
                        std::shared_ptr<expressionNode> res = std::make_shared<arrayLiteralNode>(arrayLiteralNode(a->getType(),nodes));
                        return res;
                    }
                    break;
                case Variable:
                    if(auto a = dynamic_cast<const variableNode*>(tree)) {
                        std::shared_ptr<expressionNode> res = std::make_shared<variableNode>(variableNode(a->getType(),a->name));
                        return res;
                    }
                    break;
                case BinaryExpression:
                    if(auto a = dynamic_cast<const binaryExpressionNode*>(tree)) {
                        std::shared_ptr<expressionNode> e1 = getNewTree(a->getLeft(), updateLocation, vars);
                        std::shared_ptr<expressionNode> e2 = getNewTree(a->getRight(), updateLocation, vars);
                        std::shared_ptr<expressionNode> res = std::make_shared<binaryExpressionNode>(binaryExpressionNode(a->getType(), a->getOperator(), e1, e2));
                        return res;
                    }
                    break;
                case UnaryExpression:
                    if(auto a = dynamic_cast<const unaryExpressionNode*>(tree)) {
                        std::shared_ptr<expressionNode> e = getNewTree(a->getExpression(), updateLocation, vars);
                        std::shared_ptr<expressionNode> res = std::make_shared<unaryExpressionNode>(unaryExpressionNode(a->getType(), a->getOperator(),e));
                        return res;
                    }
                    break;
                default:
                    std::cout << "ERROR: wrong node type located in expression" << std::endl;
                    //should never happen
                    break;
            }
        } else {
            switch(tree->getNodeType()) {
                case BinaryExpression:{
                    const binaryExpressionNode* binexpr = dynamic_cast<const binaryExpressionNode *>(tree);
                    std::shared_ptr<expressionNode> left;
                    std::shared_ptr<expressionNode> right;
                    if(binexpr->getLeft()->getNodeType() == Literal) {
                        left = getNewTree(binexpr->getLeft(), updateLocation, vars);
                        right = getExpressionUpdate(binexpr->getRight(), vars);
                    } else {
                        left = getExpressionUpdate(binexpr->getLeft(), vars);
                        right = getNewTree(binexpr->getRight(), updateLocation, vars);
                    }
                    std::shared_ptr<expressionNode> res =
                            std::make_shared<binaryExpressionNode>(
                                    binaryExpressionNode(binexpr->getType(), binexpr->getOperator(), left, right));
                    return res;
                }
                case UnaryExpression: {
                    const unaryExpressionNode* unexpr = dynamic_cast<const unaryExpressionNode*>(tree);
                    std::shared_ptr<expressionNode> e = getExpressionUpdate(unexpr->getExpression(), vars);
                    std::shared_ptr<expressionNode> res =
                            std::make_shared<unaryExpressionNode>(
                                    unaryExpressionNode(unexpr->getType(), unexpr->getOperator(), e));
                    return res;
                }
                case ArrayAccess:{
                    const arrayAccessNode* arracc = dynamic_cast<const arrayAccessNode*>(tree);
                    std::shared_ptr<expressionNode> acc = getExpressionUpdate(arracc->getAccessor(), vars);
                    std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(arracc->getType(), acc, arracc->getName()));
                    return res;
                }
                default:
                    std::cout << "ERROR: in finding Expression update location" << std::endl;
                    //should never happen
            }
        }
    }

    // helper function to create the updated part of the new tree
    static std::shared_ptr<expressionNode> getExpressionUpdate(const expressionNode* node, const std::map<std::string, constraint>& vars){
        std::shared_ptr<expressionNode> result;
        switch (node->getNodeType()) {
            case BinaryExpression: {
                const binaryExpressionNode* binexpr = dynamic_cast<const binaryExpressionNode*>(node);
                literalNode left = *(dynamic_cast<literalNode*>(binexpr->getLeft()));
                literalNode right = *(dynamic_cast<literalNode*>(binexpr->getRight()));
                result = DST::compute_new_literal(left, right, binexpr->getOperator(), binexpr->getType());
            }
            case UnaryExpression: {
                const unaryExpressionNode* unexpr = dynamic_cast<const unaryExpressionNode*>(node);
                literalNode lit = *(dynamic_cast<literalNode*>(unexpr->getExpression()));
                result = DST::compute_new_literal(lit, lit, unexpr->getOperator(), unexpr->getType());
            }
            case Variable: {
                const variableNode* var = dynamic_cast<const variableNode*>(node);
                constraint c = vars.find(var->name)->second;
                std::shared_ptr<literalNode> litptr =
                        std::make_shared<literalNode>(literalNode(c.type, dynamic_cast<literalNode*>(c.getRules()[0].get())->value));
                result = litptr;
            }
            case ArrayAccess: {
                const arrayAccessNode* var = dynamic_cast<const arrayAccessNode*>(node);
                std::string var_name = var->getName() + "[" + dynamic_cast<literalNode*>(var->getAccessor())->value + "]";
                constraint c = vars.find(var_name)->second;
                std::shared_ptr<literalNode> litptr =
                        std::make_shared<literalNode>(literalNode(c.type, dynamic_cast<literalNode*>(c.getRules()[0].get())->value));
                result = litptr;
            }
            default :{
                // should never happen
                std::cout << "ERROR: getExpressionUpdate() received illegal expressionNode type" << std::endl;
            }
        }
        return result;
    }

};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONVISITOR_HPP
