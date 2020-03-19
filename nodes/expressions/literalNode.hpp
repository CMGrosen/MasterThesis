//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_LITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_LITERALNODE_HPP

class literalNode : virtual public expressionNode {
public:
    literalNode(std::string a) : value{std::move(a)} {
        if (value == "true" || value == "false") {
            setType(boolType);
        } else {
            setType(intType);
        }
        setNodeType(Literal);
    };
    literalNode(Type t, std::string a) : value{std::move(a)} {
        setType(t);
        setNodeType(Literal);
    }

    std::string to_string() const override {
        return value;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<literalNode>(literalNode(type, value));
        _this->setSSA(onSSA);
        return _this;
    }

    bool operator==(const expressionNode *expr) const override {
        return (nodetype == expr->getNodeType() && value == dynamic_cast<const literalNode*>(expr)->value);
    }
    std::string value;
};
#endif //ANTLR_CPP_TUTORIAL_LITERALNODE_HPP