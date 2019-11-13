//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_MULTIPLICATIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_MULTIPLICATIONNODE_HPP


#include "binaryExpressionNode.hpp"

class multiplicationNode : public binaryExpressionNode {
public:
    multiplicationNode(Type _type, std::shared_ptr<expressionNode> _l, std::shared_ptr<expressionNode> _r) {
        type = _type;
        _operator = MULT;
        left = std::move(_l);
        right = std::move(_r);
    };
};
#endif //ANTLR_CPP_TUTORIAL_MULTIPLICATIONNODE_HPP