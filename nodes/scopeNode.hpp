//
// Created by CMG on 25/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SCOPENODE_H
#define ANTLR_CPP_TUTORIAL_SCOPENODE_H

#include <nodes/node.hpp>
#include <nodes/declarationNode.hpp>

class scopeNode : public node{
public:
    scopeNode(Type _type) {
        type = _type;
    };

    std::shared_ptr<node> next_statement;
    std::vector<std::shared_ptr<declarationNode>> dcls;

};

#endif //ANTLR_CPP_TUTORIAL_SCOPENODE_H
