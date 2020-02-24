//
// Created by hu on 24/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_TRACE_HPP
#define ANTLR_CPP_TUTORIAL_TRACE_HPP


#include <vector>
#include <nodes/edge.hpp>

class trace {
    std::vector<edge> edgesToTake;
    std::vector<std::string> inputs;
    std::shared_ptr<basicblock> goal;
public:
    trace(std::vector<edge> edges, std::vector<std::string> inputs, std::shared_ptr<basicblock> _goal)
            : edgesToTake{std::move(edges)}, inputs{std::move(inputs)}, goal{std::move(_goal)} {};
};


#endif //ANTLR_CPP_TUTORIAL_TRACE_HPP
