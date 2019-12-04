//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_STATE_HPP
#define ANTLR_CPP_TUTORIAL_STATE_HPP

#include <functional>
#include <iostream>
#include <symengine/stateStack.hpp>

class state {
public:
    state(std::shared_ptr<node> current_position, std::unordered_map<std::string, std::shared_ptr<node>> constraints) : cPos{current_position}, table{std::move(constraints)} {};
    state(std::shared_ptr<node> current_position, std::unordered_map<std::string, std::shared_ptr<node>> constraints, stateStack<std::shared_ptr<node>> nodes) : cPos{current_position}, table{std::move(constraints)}, stack{std::move(nodes)} {};
    std::shared_ptr<node> get_position() const {return cPos;}
    std::vector<state> get_successors(std::vector<state> (*func)(state*)) {
        return (*func)(this);
    }
    bool operator==(const state& s) const {
        return this->get_position() == s.get_position();
    }
    std::unordered_map<std::string, std::shared_ptr<node>> getTable() const {return table;}
    stateStack<std::shared_ptr<node>> stack;
private:
    std::shared_ptr<node> cPos;
    /* Still don't see why symbolic and concrete
     * cannot just be pointers to expression nodes.
     * Concrete is just a Literal
     * Symbolic is a BinaryExpression
     */
    std::unordered_map<std::string, std::shared_ptr<node>> table;
};

namespace std {

    template<> struct hash<state> {
        size_t operator()(const state& s) const {
            return std::hash<std::shared_ptr<node>>{}(s.get_position());
        }
    };
}

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

#endif //ANTLR_CPP_TUTORIAL_STATE_HPP
