//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP

#define CONCURRENT std::hash<std::string>{}("concurrent")

#include "statementNode.hpp"

class concurrentNode : public statementNode{
    std::vector<std::shared_ptr<statementNode>> _threads;
public:
    concurrentNode(Type type, std::vector<std::shared_ptr<statementNode>> threads) : _threads{std::move(threads)} {
        setType(type);
        setNodeType(Concurrent);
    }
    const std::vector<std::shared_ptr<statementNode>> getThreads() const {return _threads;};

    std::vector<std::shared_ptr<statementNode>> debug_getAllNodes() override {
        std::vector<std::shared_ptr<statementNode>> nexts;
        for (auto e : _threads) {
            if(auto seq = dynamic_cast<sequentialNode*>(e.get())) {
                for (auto &s : seq->debug_getAllNodes())
                    nexts.push_back(s);
            } else if (auto con = dynamic_cast<concurrentNode*>(e.get())) {
                for (auto &ss : con->debug_getAllNodes())
                    nexts.push_back(ss);
            }
            return nexts;
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
