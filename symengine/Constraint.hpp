//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP

#include <nodes/nodes.hpp>

class constraint{
public:
    constraint(Type _type) : type{_type} {}

    constraint(Type _type, std::shared_ptr<expressionNode> _expression) : type{_type}, rules{std::move(_expression)} {}

    constraint(Type _type, std::vector<std::shared_ptr<expressionNode>> _expression) : type{_type}, rules{std::move(_expression)} {}

    Type type;

    void updateRule(std::shared_ptr<expressionNode> _rule){
        rules.emplace_back(_rule);
    }

    void setRule(std::shared_ptr<expressionNode> _rule){
        rules = std::vector<std::shared_ptr<expressionNode>>{std::move(_rule)};
    }

    std::vector<std::shared_ptr<expressionNode>> getRules(){
        return rules;
    }

private:
    std::vector<std::shared_ptr<expressionNode>> rules;
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
