//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
#define ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP

#define SEQUENTIAL std::hash<std::string>{}("sequential")

#include "statementNode.hpp"

class sequentialNode : public statementNode {
public:
    sequentialNode(std::shared_ptr<statementNode> body, std::shared_ptr<statementNode> next) : _body{std::move(body)}, _next{std::move(next)} {
        if (_body->getType() == okType && _next->getType() == okType) setType(okType);
        else setType(errorType);
    };
    const statementNode* getBody() { return _body.get(); }
    const statementNode* getNext() { return _next.get(); }

    NodeType getNodeType() override { return Sequential; }

private:
    std::shared_ptr<statementNode> _body;
    std::shared_ptr<statementNode> _next;
};

#endif //ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
