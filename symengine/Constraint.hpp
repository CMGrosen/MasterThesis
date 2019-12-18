//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP

#include <nodes/nodes.hpp>

class constraint{
public:
    constraint(Type _type, std::shared_ptr<node> _expression) : type{_type}, _constraint{std::move(_expression)} {}

    Type type;

    std::shared_ptr<node> _constraint;
private:
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
