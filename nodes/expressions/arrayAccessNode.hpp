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
    void setName(std::string _name) {name = _name;}

    std::string to_string() override {
        return nameToTikzName(name, onSSA) + "[" + value->to_string() + "] ";
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _val = value->copy_expression();
        std::shared_ptr<expressionNode> _this = std::make_shared<arrayAccessNode>(arrayAccessNode(getType(), _val, name));
        _this->setSSA(onSSA);
        return _this;
    }

    void setSSA(bool t) override {
        onSSA = t;
        value->setSSA(t);
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
