//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_WRITENODE_HPP
#define ANTLR_CPP_TUTORIAL_WRITENODE_HPP

#include <nodes/expressions/expressionNode.hpp>
#include "statementNode.hpp"

class writeNode : public statementNode {
public:
    writeNode(int16_t pin, std::shared_ptr<expressionNode> e) : _e{std::move(e)}, _pin{pin} {
        type = (_e->getType() == intType) ? okType : errorType;
        setNodeType(Write);
    }
    writeNode(Type t, int16_t pin, std::shared_ptr<expressionNode> e) : _e{std::move(e)}, _pin{pin} {
        setType(t);
        setNodeType(Write);
    }

    int16_t getPin() const {return _pin;};
    expressionNode *getExpr() const {return _e.get();};

private:
    const int16_t _pin;
    std::shared_ptr <expressionNode> _e;
};

#endif //ANTLR_CPP_TUTORIAL_WRITENODE_HPP
