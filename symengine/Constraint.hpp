//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP

#include <nodes/node.hpp>

class constraint{
public:
    constraint(Type _type){
        type = _type;
        rule  = "";
        value = "";
    }

    Type type;

    void updateValue(std::string val){
        if(value != ""){
            value = value + val;
        } else {
            value = val;
        }
    }
    void setValue(std::string val){
        value = val;
    }
    std::string getValue(){
        return value;
    }

    void updateRule(std::string _rule){
        if(rule != ""){
            rule = rule + " && " + _rule;
        } else {
            rule = _rule;
        }
    }
    void setRule(std::string _rule){
        rule = _rule;
    }
    std::string getRule(){
        return rule;
    }

private:
    std::string value;
    std::string rule;
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
