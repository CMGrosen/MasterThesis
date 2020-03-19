//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP

#include <nodes/expressions/variableNode.hpp>

class arrayAccessNode : virtual public expressionNode {
public:
    arrayAccessNode(Type t, std::shared_ptr<expressionNode> a, std::shared_ptr<variableNode> n) : value{std::move(a)}, var{std::move(n)} {type = t; setNodeType(ArrayAccess);};

    expressionNode *getAccessor() const {return value.get();};
    std::string getName() const { return var->name;}
    void setName(std::string _name) {var->name = std::move(_name);}
    variableNode *getVar() const {return var.get();}

    std::string to_string() const override {
        return nameToTikzName(var->name, onSSA) + "[" + value->to_string() + "] ";
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _val = value->copy_expression();
        std::shared_ptr<variableNode> _var = std::make_shared<variableNode>(variableNode(*var));
        std::shared_ptr<expressionNode> _this = std::make_shared<arrayAccessNode>(arrayAccessNode(getType(), _val, var));
        _this->setSSA(onSSA);
        return _this;
    }

    void setSSA(bool t) override {
        onSSA = t;
        value->setSSA(t);
    }

    bool operator==(const expressionNode *expr) const override {
        //doesn't work
        return (nodetype == expr->getNodeType() && value == dynamic_cast<const arrayAccessNode*>(expr)->value);
    }

private:
    std::shared_ptr<expressionNode> value;
    std::shared_ptr<variableNode> var;
};


#endif //ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
