//
// Created by hu on 15/04/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTEPRETERDATA_HPP
#define ANTLR_CPP_TUTORIAL_INTEPRETERDATA_HPP

#include "Difference.hpp"

struct interpreterData {
    std::map<std::string, std::set<std::shared_ptr<VariableValue>>> variableValues;
    std::map<std::string, std::shared_ptr<VariableValue>> valuesFromModel;
    std::map<std::string, bool> statementsExecuted;
    std::map<std::string, Difference> differences;
};

#endif //ANTLR_CPP_TUTORIAL_INTEPRETERDATA_HPP
