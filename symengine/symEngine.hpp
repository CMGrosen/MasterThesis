//
// Created by hu on 24/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
#define ANTLR_CPP_TUTORIAL_SYMENGINE_HPP


#include <SSA_CCFG.hpp>
#include <CSSA_CFG.hpp>
#include <memory>
#include <z3++.h>
#include "trace.hpp"

class symEngine {
    std::shared_ptr<CCFG> ccfg;
    std::unordered_map<std::string, std::shared_ptr<expressionNode>> symboltable;

    z3::expr get_expr (z3::context *, expressionNode *, int *, std::vector<std::string> *, std::vector<edge> *);
    std::vector<z3::expr> reachParent(z3::context *, std::shared_ptr<basicblock>, std::shared_ptr<basicblock>, std::stack<std::string> *, std::stack<edge> *, std::set<std::shared_ptr<basicblock>> *);

public:
    symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    symEngine(std::shared_ptr<CSSA_CFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    std::vector<std::shared_ptr<trace>> execute();
    std::pair<std::shared_ptr<basicblock>, bool> run_trace(std::shared_ptr<trace>);
    std::shared_ptr<trace> get_trace(std::shared_ptr<basicblock>);
    std::vector<std::shared_ptr<trace>> find_race_condition();

};

#endif //ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
