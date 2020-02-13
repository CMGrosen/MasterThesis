//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_READNODE_HPP
#define ANTLR_CPP_TUTORIAL_READNODE_HPP

#include <nodes/expressions/variableNode.hpp>

class readNode : public expressionNode {
public:
    readNode(int16_t pin) : _pin{pin} {setNodeType(Read); setType(intType);}

    int16_t getPin() const {return _pin;};

    std::string to_string() override {
        return "read(" + std::to_string(_pin) + ")";
    }
private:
    int16_t _pin;
};

#endif //ANTLR_CPP_TUTORIAL_READNODE_HPP
