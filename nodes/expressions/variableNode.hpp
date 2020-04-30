//
// Created by hu on 23/10/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_NAMENODE_HPP
#define ANTLR_CPP_TUTORIAL_NAMENODE_HPP

#include <string>

class variableNode : public expressionNode {
public:
    variableNode(Type _type, std::string n) : name{n}, origName{std::move(n)} {
        type = _type;
        setNodeType(Variable);
    };

    variableNode(Type _type, std::string n, std::string origName) : name{std::move(n)}, origName{std::move(origName)} {
        type = _type;
        setNodeType(Variable);
    };

    variableNode(const variableNode &other) : name{other.name}, origName{other.origName} {
        type = other.type;
        setNodeType(Variable);
    };

    bool operator<(const variableNode& s) const {
        return name < s.name;
    }
    bool operator==(const variableNode& s) const {
        return name < s.name;
    }

    std::string to_string() const override {
        std::string res = nameToTikzName(name, onSSA);
        return res;
    }

    std::string strOnSourceForm() const override {
        return origName;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<variableNode>(variableNode(type, name, origName));
        _this->setSSA(onSSA);
        return _this;
    }
    std::string name;
    std::string origName;

    bool operator==(const expressionNode *expr) const override {
        return (nodetype == expr->getNodeType() && name == dynamic_cast<const variableNode *>(expr)->name);
    }
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP