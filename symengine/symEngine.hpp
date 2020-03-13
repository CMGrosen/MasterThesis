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

    z3::expr get_run(z3::context *, basicblock*, std::shared_ptr<basicblock>, std::shared_ptr<basicblock>);
    std::shared_ptr<basicblock> find_common_child(basicblock *);
    static z3::expr evaluate_operator(const z3::expr&, const z3::expr&, op);
    static z3::expr evaluate_expression(z3::context*, const expressionNode*);

    std::shared_ptr<basicblock> get_end_of_concurrent_node(basicblock *);
    bool event_encountered;
public:
    symEngine(std::shared_ptr<CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    symEngine(std::shared_ptr<CSSA_CFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    std::vector<std::shared_ptr<trace>> execute();
};

#endif //ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
