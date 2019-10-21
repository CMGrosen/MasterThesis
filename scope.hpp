//
// Created by hu on 21/10/2019.
//

#include <vector>
#include <set>
#include "symbol.hpp"

#ifndef ANTLR_CPP_TUTORIAL_SCOPE_H
#define ANTLR_CPP_TUTORIAL_SCOPE_H

#endif //ANTLR_CPP_TUTORIAL_SCOPE_H

class scope {
public:
    std::unordered_map<std::string, symbol> symboltable;
    Type type = ignoreType;
};