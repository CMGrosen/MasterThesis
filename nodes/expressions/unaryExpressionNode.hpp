//
// Created by hu on 14/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP

class unaryExpressionNode : public expressionNode {
public:
    unaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _n) :
            expr{std::move(_n)}, _operator{_op} {
        type = _type;
        setNodeType(UnaryExpression);
    }
    expressionNode* getExpression() {return expr.get();};
    op getOperator() {return _operator;};
private:
    std::shared_ptr<expressionNode> expr;
    op _operator;
};

#endif //ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
