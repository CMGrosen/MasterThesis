//
// Created by hu on 24/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
#define ANTLR_CPP_TUTORIAL_SYMENGINE_HPP

#define _run1 "run1-"
#define _run2 "run2-"

#include <src/transformers/SSA_CCFG.hpp>
#include <src/transformers/CSSA_CFG.hpp>
#include "VariableValue.hpp"
#include <memory>
#include <z3++.h>
#include "trace.hpp"

class symEngine {
    int boolname_counter;
    z3::context c;
    z3::solver s;

    void add_reads();
    z3::expr_vector get_run(const std::shared_ptr<basicblock>&, const std::shared_ptr<basicblock>&, const std::shared_ptr<basicblock>&, const std::string&, bool *);
    std::shared_ptr<basicblock> find_common_child(const std::shared_ptr<basicblock>&);
    static z3::expr evaluate_operator(z3::context *, const z3::expr&, const z3::expr&, op, z3::expr_vector *);
    static z3::expr evaluate_expression(z3::context *, const expressionNode*, const std::string&, z3::expr_vector *);

    std::shared_ptr<basicblock> get_end_of_concurrent_node(const std::shared_ptr<basicblock>&);
    z3::expr encoded_pis(const std::vector<std::pair<std::shared_ptr<basicblock>, int32_t>>&, const std::unordered_map<std::string, std::vector<std::string>>&);
    std::vector<std::string> includable_vars(const std::shared_ptr<statementNode>&, std::unordered_map<std::string, std::vector<std::string>>);
    static z3::expr encode_event_conditions_between_blocks(z3::context *, const std::shared_ptr<basicblock>&, const std::shared_ptr<basicblock>&, const std::string &run);
    std::vector<z3::expr> constraintset;

public:
    symEngine(const std::shared_ptr<CSSA_CFG>& ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);

    symEngine(const symEngine& a);
    symEngine& operator=(const symEngine&);

    symEngine(symEngine&&) noexcept;
    symEngine& operator=(symEngine&&) noexcept;

    std::shared_ptr<CCFG> ccfg;
    std::unordered_map<std::string, std::shared_ptr<expressionNode>> symboltable;
    std::pair<std::map<std::string, std::shared_ptr<VariableValue>>, std::map<std::string, bool>> getModel();

    bool execute(std::string method);
    bool updateModel(const std::vector<std::pair<std::string, Type>>&, const std::vector<std::string>&);
    bool updateModel(const z3::expr&);
    std::map<std::string, z3::expr> possible_raceconditions;
};

#endif //ANTLR_CPP_TUTORIAL_SYMENGINE_HPP