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
public:
    virtual std::string to_string() {return "";};
    virtual bool operator==(const expressionNode *expr) const {return false;};
    virtual std::shared_ptr<expressionNode> copy_expression() const {return nullptr;};
};
#endif //ANTLR_CPP_TUTORIAL_EXPRESSIONNODE_HPP