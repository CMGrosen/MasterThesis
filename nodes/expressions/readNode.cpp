
//
// Created by hu on 13/03/2020.
//

#include "readNode.hpp"

readNode::readNode(int16_t pin) : _pin{pin}, read_name{"-readVal_" + std::to_string(++count)} {
    setNodeType(Read);
    setType(intType);
    num = count;
}

int16_t readNode::getPin() const {return _pin;};

std::string readNode::to_string() {
    return "read(" + std::to_string(_pin) + ")";
}
std::shared_ptr<expressionNode> readNode::copy_expression() const {
    std::shared_ptr<expressionNode> _this = std::make_shared<readNode>(readNode(_pin));
    _this->setSSA(onSSA);
    return _this;
}
std::string readNode::getName() const {return read_name;}

bool readNode::operator==(const expressionNode *expr) const {
    return (nodetype == expr->getNodeType() && _pin == dynamic_cast<const readNode *>(expr)->getPin());
}

int32_t readNode::count = 0;
