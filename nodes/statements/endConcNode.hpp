//
// Created by hu on 16/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_ENDCONCNODE_HPP
#define ANTLR_CPP_TUTORIAL_ENDCONCNODE_HPP

class endConcNode : public statementNode {
    const int threadCount;
    const std::shared_ptr<basicblock> concNode;

public:
    endConcNode(int number_of_threads, std::shared_ptr<basicblock> concurrent_node)
        : threadCount{number_of_threads}, concNode{std::move(concurrent_node)} {
        setNodeType(EndConcurrent);
        setType(okType);
    }

    int get_number_of_threads () {return threadCount;}

    std::string to_string() override {
        return "end conc-node of " + std::to_string(threadCount) + " threads";
    }
};

#endif //ANTLR_CPP_TUTORIAL_ENDCONCNODE_HPP
