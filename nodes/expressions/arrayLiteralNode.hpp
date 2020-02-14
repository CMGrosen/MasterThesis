//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYLITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYLITERALNODE_HPP
#include <nodes/expressions/expressionNode.hpp>

class arrayLiteralNode : public expressionNode {
public:
    arrayLiteralNode(Type t, std::vector<std::shared_ptr<expressionNode>> a) : value{std::move(a)} {
        setType(t);
        setNodeType(ArrayLiteral);
    };
    arrayLiteralNode(std::vector<std::shared_ptr<expressionNode>> a) : value{std::move(a)} {
        bool wellTyped = true;
        Type t = value[0]->getType();
        if (t == intType || t == boolType) {
            for (const auto &e : value) {
                if (t != e->getType()) {
                    wellTyped = false;
                    break;
                }
            }
            if (wellTyped) {
                if (t == intType) type = arrayIntType;
                else type = arrayBoolType;
            } else type = errorType;
        } else type = errorType;
        setNodeType(ArrayLiteral);
    };

    std::string to_string() override {
        std::string res = "[";
        for (int i = 0; i < value.size(); ++i) {
            res += value[i]->to_string();
            if (i != value.size()) res += ", ";
        }
        if (getNext()) {
            res += " " + getNext()->to_string();
        }
        return res;
    }

    const std::vector<std::shared_ptr<expressionNode>> getArrLit() const {return value;};
private:
    std::vector<std::shared_ptr<expressionNode>> value;
};

#endif //ANTLR_CPP_TUTORIAL_ARRAYLITERALNODE_HPP