//
// Created by hu on 24/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
#define ANTLR_CPP_TUTORIAL_SYMENGINE_HPP


#include <SSA_CCFG.hpp>
#include <memory>
#include "trace.hpp"

class symEngine {
    std::shared_ptr<SSA_CCFG> ccfg;
    std::unordered_map<std::string, std::shared_ptr<expressionNode>> symboltable;

public:
    symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    std::vector<std::shared_ptr<trace>> execute();
    std::pair<std::shared_ptr<basicblock>, bool> run_trace(std::shared_ptr<trace>);
    std::shared_ptr<trace> get_trace(std::shared_ptr<basicblock>);
};


#endif //ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
