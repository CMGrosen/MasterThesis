//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP

#include "binaryExpressionNode.hpp"

class subtractionNode : public  binaryExpressionNode{
public:
    subtractionNode(Type _type, std::shared_ptr<expressionNode> _l, std::shared_ptr<expressionNode> _r) {
        type = _type;
        _operator = "-";
        left = std::move(_l);
        right = std::move(_r);
    };
};
#endif //ANTLR_CPP_TUTORIAL_SUBTRACTIONNODE_HPP