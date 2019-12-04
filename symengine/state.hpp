//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_STATE_HPP
#define ANTLR_CPP_TUTORIAL_STATE_HPP

#include <functional>
#include <iostream>

class state {
public:
    state(node *current_position, std::unordered_map<std::string, std::shared_ptr<node>> constraints) : cPos{current_position}, table{std::move(constraints)} {};
    node *get_position() const {return cPos;}
    std::vector<state> get_successors(std::vector<state> (*func)(state*)) {
        return (*func)(this);
    }
    std::stack<node> stack;
private:
    node *cPos;
    /* Still don't see why symbolic and concrete
     * cannot just be pointers to expression nodes.
     * Concrete is just a Literal
     * Symbolic is a BinaryExpression
     */
    std::unordered_map<std::string, std::shared_ptr<node>> table;
};

#endif //ANTLR_CPP_TUTORIAL_STATE_HPP
