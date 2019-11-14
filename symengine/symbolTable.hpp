//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOLTABLE_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOLTABLE_HPP
enum BoolOperators {EQ, LEQ, GEQ, LE, GE, AND, OR, NOT, NEQ, NOTUSED};
#include "Constraint.hpp"
class SymbolTable{
public:
    void updateValue(const std::string& name, std::string expression){
        if(isConcrete(expression)){
            std::string value;
            if(symbolicVariables.find(name) != symbolicVariables.end()){
                auto pair = symbolicVariables.find(name);
                pair->second.setRule("");
                concreteVariables.insert({pair->first, pair->second});
                symbolicVariables.erase(name);
            }
            if(concreteVariables.find(name)->second.type = intType) {
                value = evaluateExpression(expression);
            } else {
                value = evaluateExpression()
            }
            concreteVariables.find(name)->second.setValue(value);
        }
    }
    void updateRule(){

    }
private:
    std::map<std::string, constraint> symbolicVariables;
    std::map<std::string, constraint> concreteVariables;

    std::string evaluateExpression(std::string& expr, Type type){
        std::vector<std::string> tokens =  stringSplit(expr);
        if(type == intType){
            return  std::to_string(evaluateArithmetic(tokens));
        } else if(type == boolType){
            return std::to_string(evaluateLogic(tokens));
        } else {
            return "ERROR";
        }

    }

    int evaluateArithmetic(const std::vector<std::string>& tokens){
        int result = 0;
        char op = ' ';
        for(auto v : tokens){
            switch (v[0]){
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                    op = v[0];
                    break;
                default:
                    int temp;
                    if(isValue(v)){
                        temp = std::stoi(v);
                    } else {
                        temp = std::stoi(concreteVariables.find(v)->second.getValue());
                    }
                    switch(op){
                        case '+':
                            result = result + temp;
                            break;
                        case '-':
                            result = result - temp;
                            break;
                        case '*':
                            result = result * temp;
                            break;
                        case '/':
                            result = result / temp;
                            break;
                        case '%':
                            result = result % temp;
                            break;
                        default:
                            op = ' ';
                            result = temp;
                    }
                    break;
            }
        }
        return result;
    }

    bool evaluateLogic(const std::vector<std::string>& tokens){
        bool result;
        BoolOperators op = NOTUSED;
        BoolOperators prevOp = NOTUSED;
        std::vector<std::string> left;
        std::vector<std::string> right;
        for(auto v : tokens){
            if(getBoolOp(v,op)){
                switch(prevOp){
                    case EQ:
                        if(isBool(left[0])){
                            left[0] = btos((left[0] == "true") == (right[0] == "true"));
                        } else {
                            left[0] = btos(evaluateArithmetic(left) == evaluateArithmetic(right));
                        }
                        right.clear();
                        break;
                    case LEQ:
                        left[0] = btos(evaluateArithmetic(left) <= evaluateArithmetic(right));
                        right.clear();
                        break;
                    case GEQ:
                        left[0] = btos(evaluateArithmetic(left) >= evaluateArithmetic(right));
                        right.clear();
                        break;
                    case LE:
                        left[0] = btos(evaluateArithmetic(left) < evaluateArithmetic(right));
                        right.clear();
                        break;
                    case GE:
                        left[0] = btos(evaluateArithmetic(left) > evaluateArithmetic(right));
                        right.clear();
                        break;
                    case AND:
                        left[0] = btos((left[0] == "true") && (right[0] == "true"));
                        right.clear();
                        break;
                    case OR:
                        left[0] = btos((left[0] == "true") || (right[0] == "true"));
                        right.clear();
                        break;
                    case NOT:
                        left[0] = btos(!(stob(left[0])));
                        right.clear();
                        break;
                    case NEQ:
                        left[0] = btos((left[0] == "true") != (right[0] == "true"));
                        right.clear();
                        break;
                    case NOTUSED:
                        left = right;
                        right.clear();
                        break;
                    default:
                        right.emplace_back(v);
                        break;
                }
                prevOp = op;
            } else {
                right.emplace_back(v);
            }
        }
        return result;
    }

    bool getBoolOp(const std::string& token, BoolOperators& op){
        if( token == "&&"){
            op = AND;
            return true;
        } else if(token == "||"){
            op = OR;
            return true;
        } else if(token == ">="){
            op = GEQ;
            return true;
        } else if(token == "<="){
            op = LEQ;
            return true;
        } else if(token == ">"){
            op = GE;
            return true;
        } else if(token == "<"){
            op = LE;
            return true;
        } else if(token == "!"){
            op = NOT;
            return true;
        } else if(token == "!="){
            op = NEQ;
            return true;
        } else if(token == "=="){
            op = EQ;
            return true;
        } else {
            return false;
        }
    }


    static std::vector<std::string> stringSplit(const std::string& str){
        std::vector<std::string> result = std::vector<std::string>();
        std::string temp = "";
        for(auto c : str){
            switch(c){
                case ' ':
                    if(!temp.empty()) {
                        result.emplace_back(temp);
                        temp = "";
                    }
                    break;
                default:
                    temp += c;
                    break;
            }
        }
        return result;
    }
    bool isConcrete(const std::string& expr){
        std::vector<std::string> variables = getVariables(expr);
        for(const auto& var : variables){
            if(var == "Read" ||symbolicVariables.find(var) != symbolicVariables.end()){
                return false;
            }
        }
        // fix så den kan håndtere at var ikke findes
        return true;
    }
    static std::vector<std::string> getVariables(const std::string& expr){
        std::vector<std::string> result = std::vector<std::string>();
        std::string temp = "";
        for(auto c : expr){
            switch(c){
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '<':
                case '>':
                case '=':
                case '!':
                case '&':
                case '|':
                case ' ':
                    if(!temp.empty() && !isValue(temp)) {
                        result.emplace_back(temp);
                        temp = "";
                    }
                    break;
                default:
                    temp += c;
                    break;
            }
        }
        return result;
    }

    static bool isValue(const std::string& str){
        char c = str[0];
            switch (c){
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    break;
                default:
                    return isBool(str);
            }
        return true;
    }
    static bool isBool(const std::string& str){
        return str == "true" || str == "false";
    }

    static std::string btos(bool boolean){
        if(boolean){
            return "true";
        }
        return "false";
    }
    static bool stob(const std::string& str){
        return str == "true";
    }
};
#endif //ANTLR_CPP_TUTORIAL_SYMBOLTABLE_HPP