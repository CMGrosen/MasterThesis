//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP

class concurrentNode : public statementNode {
public:
    concurrentNode(Type type, std::vector<std::shared_ptr<statementNode>> threads) :
        threads{std::move(threads)} {
            setType(type);
            setNodeType(Concurrent);
        }
    std::vector<std::shared_ptr<statementNode>> threads;
};

#endif //ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
