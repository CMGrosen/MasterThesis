//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
#define ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP

#define SEQUENTIAL std::hash<std::string>{}("sequential")

#include "statementNode.hpp"

class sequentialNode : public statementNode {
public:

    void setBody( std::shared_ptr<statementNode> _body ){ body = _body; }
    void setNext(std::shared_ptr<statementNode> _next) {next = _next; }
    std::shared_ptr<statementNode> getBody() { return body; }
    std::shared_ptr<statementNode> getNext() { return next; }

    NodeType getNodeType() override { return Sequential; }

private:
    std::shared_ptr<statementNode> body;
    std::shared_ptr<statementNode> next;
};

#endif //ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
