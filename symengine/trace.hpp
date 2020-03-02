//
// Created by hu on 24/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_TRACE_HPP
#define ANTLR_CPP_TUTORIAL_TRACE_HPP


#include <vector>
#include <nodes/edge.hpp>

struct trace {
    std::vector<edge> edgesToTake;
    std::vector<std::string> inputs;
    std::vector<std::shared_ptr<basicblock>> goals;

    trace() = default;
    trace(std::vector<edge> edges, std::vector<std::string> inputs, std::shared_ptr<basicblock> _goal)
            : edgesToTake{std::move(edges)}, inputs{std::move(inputs)}, goals{std::vector<std::shared_ptr<basicblock>>{std::move(_goal)}} {};
    trace(std::vector<edge> edges, std::vector<std::string> inputs, std::vector<std::shared_ptr<basicblock>> _goals)
            : edgesToTake{std::move(edges)}, inputs{std::move(inputs)}, goals{std::move(_goals)} {};
};


#endif //ANTLR_CPP_TUTORIAL_TRACE_HPP
