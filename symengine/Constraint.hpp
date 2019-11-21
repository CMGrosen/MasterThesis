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
    std::string getValue() {
        return value;
    }

    void updateRule(std::vector<std::shared_ptr<node>> _rule){

    }

    void setRule(std::vector<std::shared_ptr<node>> _rule){
        rule = std::move(_rule);
    }
    std::vector<std::shared_ptr<node>> getRule(){
        return rule;
    }

private:
    std::string value;
    std::vector<std::shared_ptr<node>> rule;
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
