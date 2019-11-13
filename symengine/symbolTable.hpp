//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOLTABLE_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOLTABLE_HPP

#include "Constraint.hpp"
class SymbolTable{
public:
    void updateVariable(std::string name, std::string expression){
        if(isConcrete(expression)){

        }
    }
private:
    std::map<std::string, constraint> symbolicVariables;
    std::map<std::string, std::string> concreteVariables;

    bool isConcrete(std::string expr){
        std::vector<std::string> variables = splitstrings(expr);
        for(auto var : variables){
            if(symbolicVariables.find(var) != symbolicVariables.end()){
                return false;
            }
        }
        // fix så den kan håndtere at var ikke findes
        return true;
    }
    std::vector<std::string> splitstrings(std::string expr){
        std::vector<std::string> result = std::vector<std::string>();
        while(expr != ""){
            std::string temp = "";
            for(auto t : expr){
                switch(t){
                    case '+':
                        break;
                    default:
                        break;

                }
            }
        }
    }
};
#endif //ANTLR_CPP_TUTORIAL_SYMBOLTABLE_HPP
