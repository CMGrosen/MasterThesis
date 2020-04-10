//
// Created by hu on 23/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
#define ANTLR_CPP_TUTORIAL_INTERPRETER_HPP


#include "symEngine.hpp"
#include "Difference.hpp"
#include "state.hpp"

class interpreter {
    symEngine engine;
    std::map<std::string, std::set<std::shared_ptr<VariableValue>>> variableValues;
    std::map<std::string, std::shared_ptr<VariableValue>> valuesFromModel;
    std::map<std::string, bool> statementsExecuted;
    std::map<std::string, Difference> differences;
    void update();
    void refresh();
    std::map<std::string, std::pair<std::string, Type>> get_current_values();
    std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> get_threads_to_finish();

    static std::string compute_operator(const std::string&, const std::string&, op);
    std::string exec_expr(expressionNode*, const std::map<std::string, std::pair<std::string, Type>> *);
    std::pair<bool, bool> exec_stmt(const std::shared_ptr<statementNode>&, std::map<std::string, std::pair<std::string, Type>> *, bool);
    bool execute(const std::shared_ptr<basicblock>& blk, state *s);

    bool recursive_read(const std::shared_ptr<basicblock>&, state);
    bool reachable(const std::pair<std::shared_ptr<basicblock>, std::string>&, const std::string&, const std::string&);
    bool reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>&, std::vector<std::string> *);

public:
    explicit interpreter(symEngine);
    bool run();
};


#endif //ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
