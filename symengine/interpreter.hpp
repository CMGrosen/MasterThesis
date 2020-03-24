//
// Created by hu on 23/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
#define ANTLR_CPP_TUTORIAL_INTERPRETER_HPP


#include "symEngine.hpp"

class interpreter {
    symEngine engine;
    std::map<std::string, std::pair<std::set<std::string>, Type>> variableValues;
    std::map<std::string, std::pair<std::string, Type>> valuesFromModel;
    std::map<std::string, std::tuple<std::string, std::string, Type>> differences;
    void update();
    void refresh();
    bool reachable(const std::pair<std::shared_ptr<basicblock>, std::string>&, const std::string&);
    bool reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>&);
public:
    interpreter(symEngine);
    bool run();
};


#endif //ANTLR_CPP_TUTORIAL_INTERPRETER_HPP
