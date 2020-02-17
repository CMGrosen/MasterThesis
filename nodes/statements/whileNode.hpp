//
// Created by CMG on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_WHILENODE_H
#define ANTLR_CPP_TUTORIAL_WHILENODE_H

#include <nodes/expressions/expressionNode.hpp>

class whileNode : public statementNode {
public:
    whileNode(Type t, std::shared_ptr<expressionNode> c, std::shared_ptr<statementNode> b) : condition{std::move(c)}, body{std::move(b)} {
        setType(t);
        setNodeType(While);
    }
    const expressionNode *getCondition() const {return condition.get();}
    const std::shared_ptr<statementNode> getBody() const {return body;}

    std::string to_string() override {
        return "while(" + condition->to_string() + ")";
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _c = condition->copy_expression();
        std::shared_ptr<statementNode> _b = body->copy_statement();
        std::shared_ptr<statementNode> _this = std::make_shared<whileNode>(whileNode(type, _c, _b));
        return _this;
    }

private:
    std::shared_ptr<expressionNode> condition;
    std::shared_ptr<statementNode> body;
};

#endif //ANTLR_CPP_TUTORIAL_WHILENODE_H
