//
// Created by hu on 11/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP

#include <nodes/node.hpp>

class statementNode : public node {
public:
    virtual std::vector<std::shared_ptr<statementNode>> debug_getAllNodes() {};
    virtual std::string to_string() {};
};

#endif //ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
