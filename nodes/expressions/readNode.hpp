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

    bool operator==(const expressionNode *expr) const override;

private:
    int16_t _pin;
    std::string read_name;
    uint32_t num;
    static int32_t count;
};

#endif //ANTLR_CPP_TUTORIAL_READNODE_HPP
