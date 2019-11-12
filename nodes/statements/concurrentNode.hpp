//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP

#define CONCURRENT std::hash<std::string>{}("concurrent")

#include "statementNode.hpp"

class concurrentNode : public statementNode{
public:
    std::vector<std::shared_ptr<statementNode>> threads;

    void addThread(std::shared_ptr<statementNode> thread){
        threads.emplace_back(thread);
    }

    uint getNodeType() override { return CONCURRENT; }
};

#endif //ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
