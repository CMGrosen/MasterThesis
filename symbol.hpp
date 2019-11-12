//
// Created by hu on 21/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOL_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOL_HPP

#include <string>
#include "nodes/nodes.hpp"

class symbol {
public:
    Type type;
    std::string name;
    symbol(std::string _name, const Type _type) : name{_name}, type{_type} {};

    bool operator<(const symbol& other) const {return name < other.name;}
    bool operator==(const symbol& other) const {return name == other.name;}
};

#endif //ANTLR_CPP_TUTORIAL_SYMBOL_HPP
