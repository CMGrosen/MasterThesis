//
// Created by hu on 11/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP

#include <nodes/node.hpp>

class statementNode : public node {
public:
    virtual uint getNodeType() = 0;
};

#endif //ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
