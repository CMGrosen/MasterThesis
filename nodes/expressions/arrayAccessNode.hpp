//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class arrayAccessNode : public expressionNode {
public:
    arrayAccessNode(Type t, std::shared_ptr<expressionNode> a, std::string n) : value{std::move(a)}, name{std::move(n)} {type = t; setNodeType(ArrayAccess);};

    expressionNode *getAccessor() const {return value.get();};
    std::string getName() const { return name;}

    std::string to_string() override {
        std::string res = nameToTikzName(name) + "[" + value->to_string() + "] ";
        if (getNext()) {
            res += getNext()->to_string();
        }
        return res;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _val = value->copy_expression();
        _val->setNext(value->copy_next());
        std::shared_ptr<expressionNode> _this = std::make_shared<arrayAccessNode>(arrayAccessNode(getType(), _val, name));
        _this->setNext(copy_next());
        return _this;
    }

    bool operator==(const expressionNode *expr) const override {
        //doesn't work
        return (nodetype == expr->getNodeType() && name == dynamic_cast<const arrayAccessNode*>(expr)->getName());
    }

private:
    std::shared_ptr<expressionNode> value;
    std::string name;
};


#endif //ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
