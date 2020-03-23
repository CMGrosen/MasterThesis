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

    z3::expr get_run(z3::context *, std::shared_ptr<basicblock>, const std::shared_ptr<basicblock>&, const std::shared_ptr<basicblock>&, const std::string&);
    std::shared_ptr<basicblock> find_common_child(const std::shared_ptr<basicblock>&);
    static z3::expr evaluate_operator(const z3::expr&, const z3::expr&, op);
    static z3::expr evaluate_expression(z3::context*, const expressionNode*, const std::string&);

    std::shared_ptr<basicblock> get_end_of_concurrent_node(const std::shared_ptr<basicblock>&);
    z3::expr encoded_pis(z3::context *, const std::vector<std::pair<std::shared_ptr<basicblock>, int32_t>>&, const std::unordered_map<std::string, std::vector<std::string>>&);
    std::vector<std::string> includable_vars(const std::shared_ptr<statementNode>&, std::unordered_map<std::string, std::vector<std::string>>);
    bool event_encountered;
public:
    symEngine(std::shared_ptr<CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    symEngine(std::shared_ptr<CSSA_CFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);
    std::vector<std::shared_ptr<trace>> execute();
};

#endif //ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
