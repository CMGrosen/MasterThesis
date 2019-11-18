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
};

#endif //ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
