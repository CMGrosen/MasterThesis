//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
enum TyPe {boolean, number};
class constraint{
public:
    constraint( ){
        rule  = "";
        value = "";
    }

    void setRule(std::string _rule){
        if(rule != ""){
            rule = rule + " && " + _rule;
        } else {
            rule = _rule;
        }
    }
    void setValue(std::string val){
        if(value != ""){
            value = value + val;
        } else {
            value = val;
        }
    }

    std::string getRule(){
        return rule;
    }
    std::string getValue(){
        return value;
    }

private:
    std::string value;
    std::string rule;
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
