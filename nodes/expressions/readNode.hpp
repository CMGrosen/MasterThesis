//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_READNODE_HPP
#define ANTLR_CPP_TUTORIAL_READNODE_HPP

#include "expressionNode.hpp"

class readNode : public expressionNode {
public:
    readNode(int16_t);
    int16_t getPin() const;

    std::string to_string() override;

    std::shared_ptr<expressionNode> copy_expression() const override;

    std::string getName() const;
    void setName(std::string);

    bool operator==(const expressionNode *expr) const override;

private:
    readNode(int16_t, std::string);
    int16_t _pin;
    std::string read_name;
};

#endif //ANTLR_CPP_TUTORIAL_READNODE_HPP
