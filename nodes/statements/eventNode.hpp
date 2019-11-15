//
// Created by hu on 15/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_EVENTNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class eventNode : public statementNode {
public:
    eventNode(Type t, std::shared_ptr<expressionNode> condition) : _condition{condition} {
        setType(t);
    }
    const expressionNode *getCondition() {return _condition.get();}
private:
    std::shared_ptr<expressionNode> _condition;
};

#endif //ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
