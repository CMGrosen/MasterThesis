//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP

#include <nodes/nodes.hpp>

class constraint{
public:
    constraint(Type _type) : type{_type} {}

    constraint(Type _type, std::shared_ptr<expressionNode> _expression) : type{_type}, rule{std::move(_expression)} {}

    Type type;

    void updateRule(std::shared_ptr<expressionNode> _rule){

    }

    void setRule(std::shared_ptr<expressionNode> _rule){
        rule = std::move(_rule);
    }
    std::shared_ptr<node> getRule(){
        return rule;
    }

private:
    std::shared_ptr<expressionNode> rule;
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
