//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP

class concurrentNode : public node {
public:
    concurrentNode(Type type, std::vector<std::shared_ptr<node>> threads) :
        node(type, Concurrent),
        threads{std::move(threads)} {
            setType(type);
            setNodeType(Concurrent);
        }
    concurrentNode(Type type, std::vector<std::shared_ptr<node>> threads, node *parent) :
            node(type, Concurrent),
            threads{std::move(threads)},
            orig{parent} {
        setType(type);
        setNodeType(Concurrent);
    }
    std::vector<std::shared_ptr<node>> threads;
    node *orig = nullptr;
};

#endif //ANTLR_CPP_TUTORIAL_CONCURRENTNODE_HPP
