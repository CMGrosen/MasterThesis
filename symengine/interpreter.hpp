//
// Created by hu on 23/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
#define ANTLR_CPP_TUTORIAL_INTERPRETER_HPP


#include "symEngine.hpp"
#include "Difference.hpp"

class interpreter {
    symEngine engine;
    std::map<std::string, std::pair<std::string, Type>> current_values;
    std::map<std::string, std::set<std::shared_ptr<VariableValue>>> variableValues;
    std::map<std::string, std::shared_ptr<VariableValue>> valuesFromModel;
    std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> threadsToFinish;
    std::set<std::shared_ptr<basicblock>> visitLater;
    std::map<std::string, bool> statementsExecuted;
    std::map<std::string, Difference> differences;
    void update();
    void refresh();
    void refreshConcs();

    static std::string compute_operator(const std::string&, const std::string&, op);
    std::string exec_expr(expressionNode*);
    std::pair<bool, bool> exec_stmt(const std::shared_ptr<statementNode>&);
    bool execute(const std::shared_ptr<basicblock>& blk, std::set<std::shared_ptr<basicblock>> *blks);

    bool reachable(const std::pair<std::shared_ptr<basicblock>, std::string>&, const std::string&);
    bool reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>&, std::vector<std::string> *);
    std::pair<bool, std::vector<edge>> edges_to_take(std::shared_ptr<basicblock>, std::shared_ptr<basicblock>, const std::string&);
    static std::string report_racecondition(std::string, std::shared_ptr<statementNode>, std::pair<std::shared_ptr<basicblock>, std::string>, std::pair<std::shared_ptr<basicblock>, std::string>);

public:
    interpreter(symEngine);
    bool run();
};


#endif //ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
