//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP

#include <nodes/node.hpp>

enum op { PLUS, MINUS, MULT, DIV, MOD, NOT, AND, OR, LE, LEQ, GE, GEQ, EQ, NEQ, NEG, NOTUSED};

static std::map<op, std::string> operatorToString {
        {PLUS, "+"},
        {MINUS, "-"},
        {MULT, "*"},
        {DIV, "/"},
        {MOD, "\\%"},
        {AND, "\\&\\&"},
        {OR, "||"},
        {LE, "<"},
        {LEQ, "<="},
        {GE, ">"},
        {GEQ, ">="},
        {EQ, "=="},
        {NEQ, "!="},
        {NEG, "-"}
};

class expressionNode : public node {
    std::shared_ptr<expressionNode> _next = nullptr;

public:
    void setNext(std::shared_ptr<expressionNode> n) {_next = std::move(n);}
    std::shared_ptr<expressionNode> getNext() const {return _next;}
    std::shared_ptr<expressionNode> getLast() {
        if(!_next) return nullptr;
        auto next = _next;
        while(next->getNext()) next = next->getNext();
        return next;
    }
    virtual std::string to_string() {};
    virtual std::shared_ptr<expressionNode> copy_expression() const {};
    virtual bool operator==(const expressionNode *expr) const {};
    std::shared_ptr<expressionNode> copy_next() const {
        if(getNext()) return getNext()->copy_expression();
        else return nullptr;
    }
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP