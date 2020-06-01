//
// Created by hu on 24/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
#define ANTLR_CPP_TUTORIAL_SYMENGINE_HPP

#define _run1 "run1-"
#define _run2 "run2-"

#include <src/CFGs/CSSA_CCFG.hpp>
#include <memory>
#include <z3++.h>

struct Model {
    std::map<std::string, literalNode> values;
    std::map<std::string, std::pair<std::string, bool>> paths;
    std::map<std::string, std::pair<std::string, bool>> interleavings;
    std::map<std::string, std::pair<std::string, int32_t>> piOptionChoices;

    Model( std::map<std::string, literalNode> _values
         , std::map<std::string, std::pair<std::string, bool>> _paths
         , std::map<std::string, std::pair<std::string, bool>> _interleavings
         , std::map<std::string, std::pair<std::string, int32_t>> _piOption
         ) : values{std::move(_values)}, paths{std::move(_paths)}, interleavings{std::move(_interleavings)}, piOptionChoices{std::move(_piOption)} {}
};

class symEngine {
    z3::context c;
    z3::solver s;

    void add_reads();
    z3::expr_vector get_run(const std::shared_ptr<basicblock>&, const std::shared_ptr<basicblock>&, const std::shared_ptr<basicblock>&, const std::string&);
    std::shared_ptr<basicblock> find_common_child(const std::shared_ptr<basicblock>&);
    static z3::expr evaluate_operator(z3::context *, const z3::expr&, const z3::expr&, op, z3::expr_vector *);
    static z3::expr evaluate_expression(z3::context *, const expressionNode*, const std::string&, z3::expr_vector *);

    std::shared_ptr<basicblock> get_end_of_concurrent_node(const std::shared_ptr<basicblock>&);
    std::vector<z3::expr> constraintset;

public:
    symEngine(const std::shared_ptr<CSSA_CCFG>& ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table);

    symEngine(const symEngine& a);
    symEngine& operator=(const symEngine&);

    symEngine(symEngine&&) noexcept;
    symEngine& operator=(symEngine&&) noexcept;

    std::shared_ptr<CSSA_CCFG> ccfg;
    std::unordered_map<std::string, std::shared_ptr<expressionNode>> symboltable;
    Model getModel();

    bool execute();
    bool updateModel(const std::vector<std::pair<std::string, Type>>&, const std::vector<std::string>&);
    bool updateModel(const z3::expr&);
    std::map<std::string, z3::expr> possible_raceconditions;
};

#endif //ANTLR_CPP_TUTORIAL_SYMENGINE_HPP
