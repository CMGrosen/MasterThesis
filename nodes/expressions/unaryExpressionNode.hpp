//
// Created by hu on 14/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP

class unaryExpressionNode : public expressionNode {
public:
    unaryExpressionNode(Type _type, op _op) : _operator{_op} {
        type = _type;
        setNodeType(UnaryExpression);
    }
    op getOperator() const {return _operator;};
private:
    op _operator;
};

#endif //ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
