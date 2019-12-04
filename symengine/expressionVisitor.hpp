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
// we assume that the left most leaf of the expression tree is the next part to be evaluated,
// if the leaf is a literal then either the parent or the right child of the parent is the next to be evaluated,
// depending on whether the parent is a binary expression and the type of the right child
class expressionVisistor {
public:
    // uses an expression tree for current state to create a new expression tree for the next state
    std::shared_ptr<expressionNode> visitNextExpression(expressionNode* node, std::map<std::string, constraint> vars){
        expressionNode* changeHere = nullptr;
        if(node->getNodeType() == BinaryExpression){
            binaryExpressionNode* binexpr = (dynamic_cast<binaryExpressionNode*>(node));
            changeHere = getBottomExpression(binexpr, binexpr->getLeft(), binexpr->getRight(),vars);
        } else if(node->getNodeType() == BinaryExpression) {

        }
        return makeNewTree(node, changeHere, vars);
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
            case ArrayAccess: {
                arrayAccessNode* arracc = dynamic_cast<arrayAccessNode*>(child_left);
                return getBottomExpression(arracc, arracc->getAccessor(), nullptr, vars);
            }
            case Variable: {
                return child_left;
            }
            case Read: {
                return checkRightChild(parent, child_left, child_right, vars);
            }
            case ConstraintNode: {
                return checkRightChild(parent, child_left, child_right, vars);
            }
            case Literal: {
                return checkRightChild(parent, child_left, child_right, vars);
            }
            default: {
                // ERROR
                return ExpressionVisitorERROR("getBottomExpression()",
                        "unhandled node type on left side of expression, received node of type" + std::to_string(child_left->getNodeType())).get();
            }
        }
    }

    // returns an expressionNode pointer based on the right side of an expression,
    // if the expression has no rightside then child_right is nullptr
    expressionNode* checkRightChild(expressionNode* parent, expressionNode* child_left, expressionNode* child_right, std::map<std::string, constraint> vars){
        // in case of parent being unaryExpression or array access, child_right == null_ptr
        if(child_right == nullptr) {
            return parent;
        }
        switch(child_right->getNodeType()) {
            case BinaryExpression: {
                binaryExpressionNode* binexpr = (dynamic_cast<binaryExpressionNode*>(child_right));
                return getBottomExpression(binexpr, binexpr->getLeft(), binexpr->getRight(), vars);
            }
            case UnaryExpression: {
                unaryExpressionNode* unexpr = (dynamic_cast<unaryExpressionNode*>(child_right));
                return getBottomExpression(unexpr, unexpr->getExpression(), nullptr, vars);
            }
            case ArrayAccess: {
                arrayAccessNode* arracc = dynamic_cast<arrayAccessNode*>(child_right);
                return getBottomExpression(arracc, arracc->getAccessor(), nullptr, vars);
            }
            case Read:
                return child_right;
            case ConstraintNode:
                return parent;
            case Variable:
                return child_right;
            case Literal:
                return parent;
            default:
                // ERROR this should not happen
                return ExpressionVisitorERROR("checkRightChild()",
                        "unhandled node type  on rightside of expresion, received node of type: " + std::to_string(child_right->getNodeType())).get();
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
    static std::shared_ptr<expressionNode> makeNewTree(const expressionNode* tree, const node* updateLocation, const std::map<std::string, constraint>& vars) {
        if(tree != updateLocation){
            standardDeepCopy(tree, updateLocation, vars);
        } else {
            switch(tree->getNodeType()) {
                case BinaryExpression:{
                    const binaryExpressionNode* binexpr = dynamic_cast<const binaryExpressionNode *>(tree);
                    return EvaluateBinepxr(binexpr);
                }
                case UnaryExpression: {
                    const unaryExpressionNode* unexpr = dynamic_cast<const unaryExpressionNode*>(tree);
                    std::shared_ptr<expressionNode> e = makeExpressionUpdate(unexpr->getExpression(), vars);
                    std::shared_ptr<expressionNode> res =
                            std::make_shared<unaryExpressionNode>(
                                    unaryExpressionNode(unexpr->getType(), unexpr->getOperator(), e));
                    return res;
                }
                case ArrayAccess:{
                    const arrayAccessNode* arracc = dynamic_cast<const arrayAccessNode*>(tree);
                    std::shared_ptr<expressionNode> acc = makeExpressionUpdate(arracc->getAccessor(), vars);
                    std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(arracc->getType(), acc, arracc->getName()));
                    return res;
                }
                default:
                    //should never happen
                    return ExpressionVisitorERROR("makeNewTree()",
                            "unhandled node type in Expression update location, received node of type: " + std::to_string(tree->getNodeType()));
            }
        }
    }

    // creates copy of current node and copies children through getNewTree
    static std::shared_ptr<expressionNode> standardDeepCopy(const expressionNode *tree, const node* updateLocation, const std::map<std::string, constraint>& vars){
        switch(tree->getNodeType()) {
            case Read:
                if (auto a = dynamic_cast<const readNode*>(tree)) {
                    std::shared_ptr<expressionNode> v = makeNewTree(a->getVar(), updateLocation, vars);
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
                        std::shared_ptr<expressionNode> inter = makeNewTree(i.get(), updateLocation, vars);
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
                    std::shared_ptr<expressionNode> e1 = makeNewTree(a->getLeft(), updateLocation, vars);
                    std::shared_ptr<expressionNode> e2 = makeNewTree(a->getRight(), updateLocation, vars);
                    std::shared_ptr<expressionNode> res = std::make_shared<binaryExpressionNode>(binaryExpressionNode(a->getType(), a->getOperator(), e1, e2));
                    return res;
                }
                break;
            case UnaryExpression:
                if(auto a = dynamic_cast<const unaryExpressionNode*>(tree)) {
                    std::shared_ptr<expressionNode> e = makeNewTree(a->getExpression(), updateLocation, vars);
                    std::shared_ptr<expressionNode> res = std::make_shared<unaryExpressionNode>(unaryExpressionNode(a->getType(), a->getOperator(),e));
                    return res;
                }
                break;
            default:
                std::cout << "ERROR: wrong node type located in expression" << std::endl;
                //should never happen
                break;
        }
    }

    // helper function to create the updated part of the new tree
    static std::shared_ptr<expressionNode> makeExpressionUpdate(const expressionNode* node, const std::map<std::string, constraint>& vars){
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
                constraint constraints = vars.find(var->name)->second;
                if(auto con = dynamic_cast<constraintNode*>(constraints.getRules()[0].get())){
                    // symbolic: gets the constraint node
                    std::shared_ptr<constraintNode> cstrptr =
                            std::make_shared<constraintNode>(*con);
                    result = cstrptr;
                } else {
                    // concrete: creates a new literalNode with the value of the variable
                    std::shared_ptr<literalNode> litptr =
                            std::make_shared<literalNode>(literalNode(constraints.type, dynamic_cast<literalNode*>(constraints.getRules()[0].get())->value));
                    result = litptr;
                }
            }
            case ArrayAccess: {
                const arrayAccessNode* var = dynamic_cast<const arrayAccessNode*>(node);
                std::string var_name = var->getName() + "[" + dynamic_cast<literalNode*>(var->getAccessor())->value + "]";
                constraint c = vars.find(var_name)->second;
                std::shared_ptr<literalNode> litptr =
                        std::make_shared<literalNode>(literalNode(c.type, dynamic_cast<literalNode*>(c.getRules()[0].get())->value));
                result = litptr;
            }
            case Read: {
                std::shared_ptr<variableNode> var = std::make_shared<variableNode>(variableNode(intType, "_x_"));
                std::shared_ptr<literalNode> max  = std::make_shared<literalNode>(literalNode(intType, std::to_string(INT_MAX)));
                std::shared_ptr<literalNode> min  = std::make_shared<literalNode>(literalNode(intType, std::to_string(INT_MIN)));
                std::shared_ptr<binaryExpressionNode> less = std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, LE, var, max));
                std::shared_ptr<binaryExpressionNode> great = std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, GE, var, min));
                std::vector<std::shared_ptr<binaryExpressionNode>> cstrs = std::vector<std::shared_ptr<binaryExpressionNode>>{less, great};
                std::shared_ptr<constraintNode> cstr = std::make_shared<constraintNode>(constraintNode(cstrs));
                result = cstr;
            }
            default :{
                // should never happen
                std::cout << "ERROR: makeExpressionUpdate() received illegal expressionNode type" << std::endl;
            }
        }
        return result;
    }

    // evaluate the binary expression by finding the nodetypes on each side and then call the correct expression evaluation function
    static std::shared_ptr<expressionNode> EvaluateBinepxr(const binaryExpressionNode* binexpr) {
        const expressionNode* left = binexpr->getLeft();
        const expressionNode* right = binexpr->getRight();
        switch (left->getNodeType()) {
            case Read: {
                switch (right->getNodeType()) {
                    case ConstraintNode: {

                    }
                    case Literal: {

                    }
                    default:
                        return ExpressionVisitorERROR("evaluateBinexpr",
                                "Unhandled nodeType on right child, left child is of type: Read, and right child is of type: " + std::to_string(right->getNodeType()));
                }
            }
            case ConstraintNode: {
                switch (right->getNodeType()) {
                    case ConstraintNode: {

                    }
                    case Literal: {

                    }
                    default:
                        return ExpressionVisitorERROR("evaluateBinexpr",
                                                      "Unhandled nodeType on right child, left child is of type: ConstraintNode, and right child is of type: " + std::to_string(right->getNodeType()));
                }
            }
            case Literal: {
                switch (right->getNodeType()) {
                    case ConstraintNode: {

                    }
                    case Literal: {

                    }
                    default:
                        return ExpressionVisitorERROR("evaluateBinexpr",
                                                      "Unhandled nodeType on right child, left child is of type: Literal, and right child is of type: " + std::to_string(right->getNodeType()));
                }
            }
            default: {
                // ERROR
                return ExpressionVisitorERROR("evaluateBinexpr",
                        "NodeType of left expression is not of expected type, received node of type: " + std::to_string(left->getNodeType()));
            }
        }
    }

    // create functions for each expression evaluation pattern, based on the right and left side




    static std::shared_ptr<expressionNode> ExpressionVisitorERROR(std::string functionName, std::string errorString) {
        std::cout << "ERROR (" << functionName << " in expressionVisitor): " << errorString << std::endl;
        return nullptr;
    }

};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONVISITOR_HPP
