//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOLICEXECUTIONENGINE_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOLICEXECUTIONENGINE_HPP

#include "Constraint.hpp"
#include "expressionVisitor.hpp"
#include "state.hpp"
#include "z3++.h"

using namespace z3;

class symbolicExecutionEngine {
public:

    std::vector<z3::expr> execute(std::pair<const std::shared_ptr<statementNode>, const std::unordered_map<std::string, constraint>> treeAndSymTable) {
        testExpr();
        return std::vector<z3::expr>{};
    }

    void updateRule(const std::string& name, std::shared_ptr<expressionNode> expression){
        /*if(expression->getNodeType() == Literal){
            auto pair = variables.find(name);
            if(variables.find(name) != variables.end()) {
                if (pair->second.type == intType) {
                    pair->second.setRule(evaluateExpression(expression, intType));
                } else {
                    pair->second.setRule(evaluateExpression(expression, boolType));
                }
            }
        }*/
    }
private:
    //std::map<std::string, constraint> variables;

    std::shared_ptr<expressionNode> evaluateExpression(std::shared_ptr<expressionNode> expression, Type type){
        // use expression visitor
        /*
        std::vector<std::string> tokens =  stringSplit(expr);
        if(type == intType){
            return  std::to_string(evaluateArithmetic(tokens));
        } else if(type == boolType){
            return std::to_string(evaluateLogic(tokens));
        } else {
            return "ERROR";
        }
        */
    }
/*
    bool isConcrete(std::shared_ptr<expressionNode> expression){
        std::vector<std::string> variables = getVariables(expression);
        for(const auto& var : variables){
            if(var == "Read" ||symbolicVariables.find(var) != symbolicVariables.end()){
                return false;
            }
        }
        // since all variables are global we should never find a variable name not in the lists
        return true;
    }*/

    static std::vector<std::string> getVariables(std::shared_ptr<expressionNode> expression){
        std::vector<std::string> result = std::vector<std::string>();
        // besøg alle noder og returner alle variable navne
        return result;
    }

    //definitely have to think carefully about how we do this
    std::vector<state> f(state* s) {
        switch(s->get_position()->getNodeType()) {
            case BinaryExpression:
                return std::vector<state>();
            case Literal:
                return std::vector<state>();
            case Assign:
                if(auto a = dynamic_cast<assignNode*>(s->get_position())) {
                    if (auto b = dynamic_cast<binaryExpressionNode*>(a->getExpr())) {
                        auto table = std::unordered_map<std::string, std::shared_ptr<expressionNode>>{};
                        std::shared_ptr<expressionNode> expr = DST::deepCopy(b);
                        if (expr->getNodeType() == BinaryExpression && expr->getType() == intType) {
                            if(((binaryExpressionNode*)expr.get())->getLeft()->getNodeType() == Literal
                               && ((binaryExpressionNode*)expr.get())->getRight()->getNodeType() == Literal) {
                                std::string lVal = ((literalNode*)((binaryExpressionNode*)expr.get())->getLeft())->value;
                                std::string rVal = ((literalNode*)((binaryExpressionNode*)expr.get())->getRight())->value;
                                table.insert({a->getName(), std::make_shared<literalNode>(expr->getType(), std::to_string(stoi(lVal) + stoi(rVal)))});
                            }
                        }
                        return std::vector<state>{state(nullptr, table, nullptr)};
                    }
                }
                return std::vector<state>();
            default:
                return std::vector<state>();
        }
    }

    void testExpr() {
        std::cout << "test example for (a > 34) && !(a <= 50) && a > 80\n";

        z3::context c;
        auto a = c.int_const("a");

        z3::solver s(c);
    /*
        s.add(a > 10);
        s.add(a < 30);*/

        z3::expr l = a > 34;
        z3::expr r = a <= 50;

        z3::expr l2 = l && !r;
        z3::expr r2 = a > 80;
        z3::expr fin = l2 && r2;
        s.add(l2 && r2);

        std::string aaaa = "(and (> a 34) (not (<= a 50)))";
        tactic t1 = tactic(c, "simplify");
        tactic t2 = tactic(c, "solve-eqs");
        tactic t = t1 & t2;

        solver ss = t.mk_solver();
        expr con = c.int_val(10);
        std::cout << con.to_string() << std::endl;
        std::cout << "fin: " << fin.num_args() << std::endl;

        std::cout << "fin simpl: " << fin.num_args() << std::endl;
        switch (s.check()) {
            case z3::unsat:
                std::cout << "(a > 34) && !(a <= 50) && a > 80 is not satisfiable \n";
                break;
            case z3::sat:
                std::cout << "(a > 34) && !(a <= 50) && a > 80 is satisfiable\n";
                break;
            case z3::unknown:
                std::cout << "unknown\n";
                break;
        }
        /*
        while (s.check() == z3::sat) {
            std::cout << s.get_model().eval(a);
            s.add(a != s.get_model().eval(a)); //prevent next model from using the same assignment as a previous model
        }*/
        //auto unsats = s.unsat_core();
        if (s.check() == z3::sat) {
            model m = s.get_model();
            std::cout << m.eval(a) << std::endl;
            //std::cout <<  << std::endl;
            /*for (auto l : unsats)
                std::cout << l.to_string() << std::endl;*/
        }
    }





    /*
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
        op _op = NOTUSED;
        op prevOp = NOTUSED;
        std::vector<std::string> left;
        std::vector<std::string> right;
        for(auto v : tokens){
            if(getBoolOp(v,_op)){
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
                prevOp = _op;
            } else {
                right.emplace_back(v);
            }
        }
        return result;
    }

    bool getBoolOp(const std::string& token, op& op){
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
    // bool to string
    static std::string btos(bool boolean){
        if(boolean){
            return "true";
        }
        return "false";
    }
    // string to bool
    static bool stob(const std::string& str){
        return str == "true";
    }
    */
};

#endif //ANTLR_CPP_TUTORIAL_SYMBOLICEXECUTIONENGINE_HPP