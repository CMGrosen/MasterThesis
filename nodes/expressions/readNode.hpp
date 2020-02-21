//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_READNODE_HPP
#define ANTLR_CPP_TUTORIAL_READNODE_HPP

class readNode : public expressionNode {
public:
    readNode(int16_t pin) : _pin{pin} {setNodeType(Read); setType(intType);}

    int16_t getPin() const {return _pin;};

    std::string to_string() override {
        return "read(" + std::to_string(_pin) + ")";
    }
    std::shared_ptr<expressionNode> copy_expression() const override {
        std::shared_ptr<expressionNode> _this = std::make_shared<readNode>(readNode(_pin));
        _this->setNext(this->copy_next());
        _this->setSSA(onSSA);
        return _this;
    }

    bool operator==(const expressionNode *expr) const override {
        return (nodetype == expr->getNodeType() && _pin == dynamic_cast<const readNode *>(expr)->getPin());
    }

private:
    int16_t _pin;
};

#endif //ANTLR_CPP_TUTORIAL_READNODE_HPP
