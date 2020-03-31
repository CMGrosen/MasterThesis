//
// Created by hu on 23/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
#define ANTLR_CPP_TUTORIAL_INTERPRETER_HPP


#include "symEngine.hpp"

class interpreter {
    symEngine engine;
    std::map<std::string, std::pair<std::string, Type>> current_values;
    std::map<std::string, std::set<std::shared_ptr<VariableValue>>> variableValues;
    std::map<std::string, std::shared_ptr<VariableValue>> valuesFromModel;
    std::map<std::string, std::tuple<std::string, std::string, Type>> differences;
    void update();
    void refresh();
    static std::string compute_operator(const std::string&, const std::string&, op);
    std::string exec_expr(expressionNode*);
    bool exec_stmt(const std::shared_ptr<statementNode>&);
    void execute(bool, edge, std::set<std::shared_ptr<basicblock>> *);
    bool reachable(const std::pair<std::shared_ptr<basicblock>, std::string>&, const std::string&);
    bool reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>&);
public:
    interpreter(symEngine);
    bool run();
};


#endif //ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
