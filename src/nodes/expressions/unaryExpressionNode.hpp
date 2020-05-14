//
// Created by hu on 14/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
#define ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP

class unaryExpressionNode : public expressionNode {
public:
    unaryExpressionNode(Type _type, op _op, std::shared_ptr<expressionNode> _expr) : _operator{_op}, expr{std::move(_expr)} {
        type = _type;
        setNodeType(UnaryExpression);
    }
    op getOperator() const {return _operator;};

    std::string to_string() const override {
        return operatorToTikz[_operator] + expr->to_string();
    }

    std::string strOnSourceForm() const override {
        return operatorToString[_operator] + expr->strOnSourceForm();
    }

    expressionNode *getExpr() const {return expr.get();}

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<unaryExpressionNode>(unaryExpressionNode(type, _operator, expr->copy_expression()));
        _this->setSSA(onSSA);
        return _this;
    }

    void setSSA(bool t) override {
        onSSA = t;
        expr->setSSA(t);
    }

    bool operator==(const expressionNode *_expr) const override {
        if (nodetype == _expr->getNodeType()) {
            if (auto unNode = dynamic_cast<const unaryExpressionNode *>(_expr)) {
                if (unNode->getOperator() == _operator) {
                    return expr.get() == unNode->getExpr();
                }
            }
        }
        return false;
    }

private:
    op _operator;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
