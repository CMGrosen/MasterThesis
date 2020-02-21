//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP

#include <nodes/basicblock.hpp>

class concurrentNode : public statementNode {
public:
    concurrentNode(Type type, std::vector<std::shared_ptr<statementNode>> _threads) :
        threads{std::move(_threads)} {
        setType(type);
        setNodeType(Concurrent);
    }/*
    concurrentNode(Type type, const std::vector<std::shared_ptr<basicblock>> &_threads) {
        setType(type);
        setNodeType(Concurrent);
        threads.reserve(_threads.size());
        for(auto _thread : _threads) threads.push_back(_thread);
    }*/
    std::vector<std::shared_ptr<statementNode>> threads;
    std::string to_string() override {
        return "fork with " + std::to_string(threads.size()) + " threads";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::vector<std::shared_ptr<statementNode>> _threads;
        for (auto thread : threads) {
            _threads.push_back(thread->copy_statement());
        }
        std::shared_ptr<statementNode> _this = std::make_shared<concurrentNode>(concurrentNode(type, std::move(_threads)));
        _this->setSSA(onSSA);
        return _this;
    }
};


#endif //ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
