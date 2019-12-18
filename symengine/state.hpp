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
    state(std::shared_ptr<node> current_position, std::unordered_map<std::string, std::shared_ptr<node>> constraints) : cPos{std::move(current_position)}, table{std::move(constraints)} {
        for(auto &it : table) {
            ssa_map.insert({it.first, 0});
        }
    };
    state(std::shared_ptr<node> current_position, std::unordered_map<std::string, std::shared_ptr<node>> constraints, stateStack<std::shared_ptr<node>> nodes, std::vector<std::shared_ptr<constraint>> cond, std::unordered_map<std::string, int32_t> map) :
        cPos{std::move(current_position)}, table{std::move(constraints)}, stack{std::move(nodes)}, path_condition{std::move(cond)}, ssa_map{std::move(map)} {};
    std::shared_ptr<node> get_position() const {return cPos;}
    std::vector<state> get_successors(std::vector<state> (*func)(state*)) {
        return (*func)(this);
    }
    bool operator==(const state& s) const {
        return this->get_position() == s.get_position();
    }
    std::unordered_map<std::string, std::shared_ptr<node>> getTable() const {return table;}
    stateStack<std::shared_ptr<node>> stack;
    std::vector<std::shared_ptr<constraint>> path_condition;
    std::unordered_map<std::string, int32_t> ssa_map;
private:
    std::shared_ptr<node> cPos;
    /* Still don't see why symbolic and concrete
     * cannot just be pointers to expression nodes.
     * Concrete is just a Literal
     * Symbolic is a BinaryExpression
     */
    std::unordered_map<std::string, std::shared_ptr<node>> table;
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class T>
inline void hash_combine(std::size_t& seed, T *v)
{
    seed ^= std::hash<T *>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {

    template<> struct hash<state> {
        size_t operator()(const state& s) const {
            size_t seed = 0;
            node *orig = s.get_position().get();
            if (s.get_position()->getNodeType() == Concurrent) {
                auto l = dynamic_cast<concurrentNode *>(orig);
                if (l->orig)
                    orig = l->orig;
                hash_combine(seed, orig);
                for (auto it = begin(l->threads); it < end(l->threads); it++)
                    hash_combine(seed, *it); //boost::hash_combine
            } else {
                hash_combine(seed, orig);
            }
            auto table = s.getTable();
            for (auto &it : table) {
                if (it.second->getNodeType() == ConstraintNode) {
                    hash_combine(seed, *dynamic_cast<constraintNode *>(it.second.get()));
                } else {
                    hash_combine(seed, it.second->getValue());
                }
            }

            return seed;
        }
    };

    template <> struct hash<constraintNode> {
        size_t operator()(const constraintNode& c) const {
            size_t seed = 0;
            for (auto it = begin(c.constraints); it < end(c.constraints); it++) {
                hash_combine(seed, **it);
            }
            return seed;
        }
    };

    template <> struct hash<constraint> {
        size_t operator()(const constraint& c) const {
            size_t seed = 0;
            std::shared_ptr<node> n = c._constraint;
            while (n) {
                if (n->getNodeType() == BinaryExpression || n->getNodeType() == UnaryExpression) {
                    hash_combine(seed, n->getOperator());
                } else {
                    hash_combine(seed,n->getValue());
                }
                n = n->getNexts()[0];
            }
            return seed;
        }
    };
}



#endif //ANTLR_CPP_TUTORIAL_STATE_HPP
