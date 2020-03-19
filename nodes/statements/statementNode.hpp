//
// Created by hu on 11/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP

#include <nodes/node.hpp>

class statementNode : virtual public node {
public:
    //virtual std::vector<std::shared_ptr<statementNode>> debug_getAllNodes() {};
    virtual std::string to_string() const = 0;
    virtual std::shared_ptr<statementNode> copy_statement() const = 0;
};

#endif //ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
