//
// Created by hu on 23/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
#define ANTLR_CPP_TUTORIAL_INTERPRETER_HPP


#include "symEngine.hpp"
#include "intepreterData.hpp"
#include <deque>

class interpreter {
    symEngine engine;
    interpreterData data;
    void update();
    void refresh();

    static std::string compute_operator(const std::string&, const std::string&, op);
    static std::pair<bool, std::shared_ptr<expressionNode>> exec_expr(const expressionNode*, runInformation&);

    bool reach_potential_raceConditions(const std::string &piFunction);

    bool find_race(runInformation &run, const piNode &pi);
    std::pair<bool, std::vector<std::shared_ptr<basicblock>>> applied_semantics(std::shared_ptr<basicblock>&, runInformation&, std::deque<std::shared_ptr<basicblock>>&, bool &);
    std::string report_datarace(const piNode &pi);
public:
    explicit interpreter(symEngine);
    bool run();

    virtual ~interpreter();
};


#endif //ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
