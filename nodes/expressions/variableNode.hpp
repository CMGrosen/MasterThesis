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
        std::string res = name + " ";
        if (getNext()) res += getNext()->to_string();
        return res;
    }

    std::string name;
};
#endif //ANTLR_CPP_TUTORIAL_NAMENODE_HPP