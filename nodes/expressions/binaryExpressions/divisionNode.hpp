//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_DIVISIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_DIVISIONNODE_HPP


#include "binaryExpressionNode.hpp"

class divisionNode : public binaryExpressionNode {
public:
    divisionNode(Type _type, std::shared_ptr<expressionNode> _l, std::shared_ptr<expressionNode> _r) {
        type = _type;
        _operator = "/";
        left = std::move(_l);
        right = std::move(_r);
    };
};

#endif //ANTLR_CPP_TUTORIAL_DIVISIONNODE_HPP