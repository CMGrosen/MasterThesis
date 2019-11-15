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

    std::vector<std::shared_ptr<statementNode>> debug_getAllNodes() override {
        std::vector<std::shared_ptr<statementNode>> nexts{_body};
        if(auto next = dynamic_cast<sequentialNode*>(_next.get())) {
             for (auto &s : next->debug_getAllNodes())
                 nexts.push_back(s);
        } else {
            nexts.push_back(_next);
        }
        return nexts;
    }

private:
    std::shared_ptr<statementNode> _body;
    std::shared_ptr<statementNode> _next;
};

#endif //ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
