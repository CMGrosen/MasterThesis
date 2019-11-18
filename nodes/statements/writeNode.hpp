//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_WRITENODE_HPP
#define ANTLR_CPP_TUTORIAL_WRITENODE_HPP

#include <nodes/expressions/expressionNode.hpp>
#include "statementNode.hpp"

class writeNode : public statementNode {
public:
    writeNode(std::shared_ptr<variableNode> v, std::shared_ptr<expressionNode> e) : _e{std::move(e)}, _v{std::move(v)} {
        type = (_v->getType() == intType && _e->getType() == intType) ? okType : errorType;
        setNodeType(Write);
    }
    writeNode(Type t, std::shared_ptr<variableNode> v, std::shared_ptr<expressionNode> e) : _e{std::move(e)}, _v{std::move(v)} {
        setType(t);
        setNodeType(Write);
    }

    variableNode *getVar() const {return _v.get();};
    expressionNode *getExpr() const {return _e.get();};

private:
    std::shared_ptr <variableNode> _v;
    std::shared_ptr <expressionNode> _e;
};

#endif //ANTLR_CPP_TUTORIAL_WRITENODE_HPP
