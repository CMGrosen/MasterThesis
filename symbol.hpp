//
// Created by hu on 21/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOL_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOL_HPP

#include <string>
#include "nodes/nodes.hpp"
#include <symengine/Constraint.hpp>

class symbol {
public:
    Type type;
    constraint cons = constraint();

    symbol(const Type _type) : type{_type} {};

};

#endif //ANTLR_CPP_TUTORIAL_SYMBOL_HPP
