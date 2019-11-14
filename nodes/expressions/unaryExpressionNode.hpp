//
// Created by hu on 14/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP

class unaryExpressionNode : public expressionNode {
public:
    unaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _n) :
            expr{std::move(_n)} {
        type = _type;
        _operator = _op;
    }
    expressionNode* getExpression() {return expr.get();};
private:
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
