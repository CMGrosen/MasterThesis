//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_STATE_HPP
#define ANTLR_CPP_TUTORIAL_STATE_HPP

#include <functional>
#include <iostream>
#include "symbolTable.hpp"
class state {
public:
    state(node *current_position, std::unordered_map<std::string, std::shared_ptr<expressionNode>> constraints, std::shared_ptr<node> operating_on) : cPos{current_position}, table{std::move(constraints)}, n{std::move(operating_on)} {};
    node *get_position() const {return cPos;}
    node *get_operation() const {return n.get();}
    std::vector<state> get_successors(std::vector<state> (*func)(state*)) {
        return (*func)(this);
    }
private:
    node *cPos;
    /* Still don't see why symbolic and concrete
     * cannot just be pointers to expression nodes.
     * Concrete is just a Literal
     * Symbolic is a BinaryExpression
     */
    std::unordered_map<std::string, std::shared_ptr<expressionNode>> table;
    std::shared_ptr<node> n;
};

#endif //ANTLR_CPP_TUTORIAL_STATE_HPP
