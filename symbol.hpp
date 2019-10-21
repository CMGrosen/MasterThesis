//
// Created by hu on 21/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOL_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOL_HPP

#endif //ANTLR_CPP_TUTORIAL_SYMBOL_HPP

#include <string>
#include "nodetype.hpp"

class symbol {
public:
    Type type;
    std::string name;
    symbol(const std::string &name) : name(name) {type = ignoreType;};

    bool operator<(const symbol& other) const {return name < other.name;}
    bool operator==(const symbol& other) const {return name == other.name;}
};