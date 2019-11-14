//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class arrayAccessNode : public expressionNode {
public:
    arrayAccessNode(Type t, std::shared_ptr<expressionNode> a) : value{std::move(a)} {type = t;};

    std::shared_ptr<expressionNode> value;
};


#endif //ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
