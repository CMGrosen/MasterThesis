
//
// Created by hu on 13/03/2020.
//

#include "readNode.hpp"

readNode::readNode(int16_t pin) : _pin{pin}, read_name{"-readVal"} {
    setNodeType(Read);
    setType(intType);
}

int16_t readNode::getPin() const {return _pin;}

std::string readNode::to_string() const {
    return "read(" + std::to_string(_pin) + ")";
}
std::string readNode::strOnSourceForm() const {
    return to_string();
}
std::shared_ptr<expressionNode> readNode::copy_expression() const {
    std::shared_ptr<expressionNode> _this = std::make_shared<readNode>(readNode(_pin, read_name));
    _this->setSSA(onSSA);
    return _this;
}
std::string readNode::getName() const {return read_name;}


bool readNode::operator==(const expressionNode *expr) const {
    return (nodetype == expr->getNodeType() && _pin == dynamic_cast<const readNode *>(expr)->getPin());
}

void readNode::setName(std::string newname) {
    read_name = std::move(newname);
}

readNode::readNode(int16_t pin, std::string name) : _pin{pin}, read_name{std::move(name)} {
    setNodeType(Read);
    setType(intType);
}



