//
// Created by hu on 31/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_DIFFERENCE_HPP
#define ANTLR_CPP_TUTORIAL_DIFFERENCE_HPP

#include <nodes/expressions/variableNode.hpp>
#include "VariableValue.hpp"

struct Difference : public variableNode {
    std::string run1;
    std::string run2;

    Difference(const std::shared_ptr<VariableValue>& val1, const std::shared_ptr<VariableValue>& val2)
        : variableNode
          ( val1->getType() == errorType ? val2->getType() : val1->getType()
          , val1->getType() == errorType ? val2->name : val1->name
          , val1->getType() == errorType ? val2->origName : val1->origName
          ), run1{val1->value}, run2{val2->value} {}
};

#endif //ANTLR_CPP_TUTORIAL_DIFFERENCE_HPP
