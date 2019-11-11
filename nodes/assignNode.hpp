//
// Created by hu on 11/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP

#include "nodes.hpp"
#include <string>

class assignNode : public statementNode {
public:
    assignNode(std::string name, std::shared_ptr<expressionNode> n) : name{std::move(name)}, expr{std::move(n)} {};
private:
    std::string name;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ASSIGNNODE_HPP
