//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_NAMENODE_HPP
#define ANTLR_CPP_TUTORIAL_NAMENODE_HPP

#include <string>

class variableNode : public expressionNode {
public:
    variableNode(Type _type, std::string n) {
        type = _type;
        name = std::move(n);
        setNodeType(Variable);
    };

    bool operator<(const variableNode& s) const {
        return name < s.name;
    }
    bool operator==(const variableNode& s) const {
        return name < s.name;
    }

    std::string to_string() override {
        std::string res = nameToTikzName(name, onSSA);
        return res;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<variableNode>(variableNode(type, name));
        _this->setSSA(onSSA);
        return _this;
    }
    std::string name;

    bool operator==(const expressionNode *expr) const override {
        return (nodetype == expr->getNodeType() && name == dynamic_cast<const variableNode *>(expr)->name);
    }
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP