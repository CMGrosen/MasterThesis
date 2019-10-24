//
// Created by hu on 24/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_DECORATEDTREE_HPP
#define ANTLR_CPP_TUTORIAL_DECORATEDTREE_HPP

#include <nodes/node.hpp>
#include <nodes/literalNode.hpp>

class decoratedTree {
private:
    int something = 0;
    node _node;
public:
    decoratedTree() : _node{literalNode(1)}{something=1; _node.setType(intType);};
    decoratedTree(int a) : something{a} {_node = literalNode(a); _node.setType(intType);};
    decoratedTree(node n) : _node{node}, something{1000} {};
    int get_something() {return something;};
};
#endif //ANTLR_CPP_TUTORIAL_DECORATEDTREE_HPP
