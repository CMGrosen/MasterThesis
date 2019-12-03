//
// Created by hu on 14/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP

class unaryExpressionNode : public node {
public:
    unaryExpressionNode(Type _type, op _op, std::shared_ptr<node> _n) :
        node(_type, UnaryExpression, _op), expr{std::move(_n)} {}
    node* getExpression() const {return expr.get();};
private:
    std::shared_ptr<node> expr;
};

#endif //ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
