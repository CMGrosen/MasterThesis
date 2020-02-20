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

    std::string to_string() override {
        std::string res = operatorToString[_operator] + " ";
        if (getNext()) res += getNext()->to_string();
        return res;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<unaryExpressionNode>(unaryExpressionNode(type, _operator));
        _this->setNext(this->copy_next());
        return _this;
    }

    bool operator==(const expressionNode *expr) const override {
        return (nodetype == expr->getNodeType() && _operator == dynamic_cast<const binaryExpressionNode *>(expr)->getOperator());
    }

private:
    op _operator;
};

#endif //ANTLR_CPP_TUTORIAL_UNARYEXPRESSIONNODE_HPP
