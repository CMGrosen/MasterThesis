//
// Created by hu on 15/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_EVENTNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class eventNode : public statementNode {
public:
    eventNode(Type t, std::shared_ptr<expressionNode> condition) : _condition{std::move(condition)} {
        setType(t);
        setNodeType(Event);
    }
    const expressionNode *getCondition() const {return _condition.get();}
    std::string to_string() override {
        return "event(" + _condition->to_string() + ")";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _expr = _condition->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<eventNode>(eventNode(type, _expr));
        return _this;
    }
private:
    std::shared_ptr<expressionNode> _condition;
};

#endif //ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
