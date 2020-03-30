//
// Created by hu on 16/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_ENDCONCNODE_HPP
#define ANTLR_CPP_TUTORIAL_ENDCONCNODE_HPP

#include <nodes/basicblock.hpp>

class endConcNode : virtual public statementNode {
    const int threadCount;
    std::shared_ptr<basicblock> concNode;

public:
    endConcNode(int number_of_threads) : threadCount{number_of_threads} {
        setNodeType(EndConcurrent);
        setType(okType);
    }
    endConcNode(int number_of_threads, std::shared_ptr<basicblock> concurrent_node)
        : threadCount{number_of_threads}, concNode{std::move(concurrent_node)} {
        setNodeType(EndConcurrent);
        setType(okType);
    }

    int get_number_of_threads () {return threadCount;}

    std::shared_ptr<basicblock> getConcNode() const {return concNode;}
    void setConcNode(std::shared_ptr<basicblock> blk) {concNode = std::move(blk);}
    std::string to_string() const override {
        return "end conc-node of " + std::to_string(threadCount) + " threads";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _this = std::make_shared<endConcNode>(endConcNode(threadCount, concNode));
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
    }
};

#endif //ANTLR_CPP_TUTORIAL_ENDCONCNODE_HPP
