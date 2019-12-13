//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SYMBOLICEXECUTIONENGINE_HPP
#define ANTLR_CPP_TUTORIAL_SYMBOLICEXECUTIONENGINE_HPP

//#include "Constraint.hpp"
//#include "expressionVisitor.hpp"
#include "state.hpp"
#include "z3++.h"
#include <symengine/stateQueue.hpp>
#include <exception>

using namespace z3;

class symbolicExecutionEngine {
public:

    std::vector<z3::expr> execute(std::pair<const std::shared_ptr<node>, const std::unordered_map<std::string, std::shared_ptr<node>>> treeAndSymTable) {
        //testExpr();
        //expressionVisistor eVisitor;
        state s = state(treeAndSymTable.first, treeAndSymTable.second);
        frontier.push(s);
        auto res = compute_statements();
        return res;
    }

private:
    std::unordered_set<state> explored;
    stateQueue<state> frontier;
    std::vector<state> testCases;

    std::vector<z3::expr> compute_statements() {
        while(!frontier.empty()) {
            state s = frontier.myPop();
            auto inserted = explored.insert(s);
            if (inserted.second) {
                std::vector<state> states = compute_statements_helper(std::move(s));
                for (auto state : states)
                    frontier.push(state);
            }
            std::cout << "here\n";
        }
        return std::vector<z3::expr>{};
    }

    std::vector<state> compute_statements_helper(state s) {
        std::shared_ptr<node> n = s.get_position();
        std::vector<state> states = std::vector<state>{};
        stateStack<std::shared_ptr<node>> stack = s.stack;
        if (!n) {return states;}
        std::vector<std::shared_ptr<node>> nexts = n->getNexts();
        auto table = s.getTable();
        switch (n->getNodeType()) {
            case Literal:
                stack.push(n);
                if (nexts.empty()) states.emplace_back(state(nullptr, s.getTable(), std::move(stack)));
                else states.emplace_back(state(n->getNexts()[0], s.getTable(), std::move(stack)));
                break;
            case Assign: {
                std::shared_ptr<node> top = stack.myPop();
                table.find(n->getValue())->second = top;
                if (nexts.empty()) states.emplace_back(state(nullptr, table, stack));
                else states.emplace_back(state(n->getNexts()[0], table, stack));
                break;
            }
            case BinaryExpression: {
                std::shared_ptr<node> right = stack.myPop();
                std::shared_ptr<node> left = stack.myPop();
                if (left->getNodeType() == Literal && right->getNodeType() == Literal) {
                    auto res = DST::compute_new_literal(left, right, n->getOperator(), n->getType());
                    if (res.first) {
                        stack.push(res.second);
                        states.emplace_back(state(nexts[0], table, stack));
                    } else
                        testCases.emplace_back(s);
                } else {
                    std::shared_ptr<constraintNode> leftConstraint = nullptr;
                    std::shared_ptr<constraintNode> rightConstraint = nullptr;
                    if (left->getNodeType() == ConstraintNode) {
                        leftConstraint = std::make_shared<constraintNode>(*dynamic_cast<constraintNode*>(left.get()));
                    }
                    if (right->getNodeType() == ConstraintNode) {
                        rightConstraint = std::make_shared<constraintNode>(*dynamic_cast<constraintNode *>(right.get()));
                    }

                    std::shared_ptr<constraintNode> c;
/*
                    if (leftConcrete) {
                        c = std::make_shared<constraintNode>(std::make_shared<binaryExpressionNode>(binaryExpressionNode(n->getType(), n->getOperator(), left, std::make_shared<constraintNode>(constraintNode(v)))));
                    } else {
                        c = std::make_shared<constraintNode>(std::make_shared<binaryExpressionNode>(binaryExpressionNode(n->getType(), n->getOperator(), std::make_shared<constraintNode>(constraintNode(v)), right)));
                    }

                    if (c->isSatisfiable().first) {
                        stack.push(c);
                        states.emplace_back(state(nexts[0],table,stack));
                    } else {
                        stack.push(std::make_shared<node>(node(boolType,BinaryExpression,n->getOperator(),"false")));
                        states.emplace_back(state(nexts[0],table,stack));
                    }*/
                }
            }
            break;
            case Read: {
                std::shared_ptr<node> top = stack.myPop();
                if (top->getNodeType() == ConstraintNode) {
                    if(dynamic_cast<constraintNode*>(top.get())->isSatisfiable()) {
                        auto l = std::make_shared<node>(node(intType,Variable));
                        auto last = std::make_shared<node>(node(intType, Literal, std::to_string(INT32_MIN)));
                        auto prev = last;
                        l->setNext(prev);
                        last = std::make_shared<node>(node(intType, BinaryExpression, GEQ));
                        prev->setNext(last);
                        prev = last;
                        last = std::make_shared<node>(node(intType, Variable));
                        prev->setNext(last);
                        prev = last;
                        last = std::make_shared<node>(node(intType, Literal, std::to_string(INT32_MAX)));
                        prev->setNext(last);
                        prev = last;
                        last = std::make_shared<node>(node(intType, BinaryExpression, LEQ));
                        prev->setNext(last);
                        last = std::make_shared<node>(node(intType, BinaryExpression, AND));
                        prev->setNext(last);
                        std::shared_ptr<constraintNode> nod = std::make_shared<constraintNode>(constraintNode(intType));
                        nod->setNext(l);
                        stack.push(nod);
                        states.emplace_back(state(nexts[0],table,stack));
                    }
                } else {
                    /*auto l = std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, GEQ, std::make_shared<node>(node(intType,Variable)), std::make_shared<node>(node(intType, Literal, std::to_string(INT32_MIN)))));
                    auto r = std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, LEQ, std::make_shared<node>(node(intType,Variable)), std::make_shared<node>(node(intType, Literal, std::to_string(INT32_MAX)))));
                    stack.push(std::make_shared<constraintNode>(constraintNode(std::vector<std::shared_ptr<binaryExpressionNode>>{l,r})));
                    states.emplace_back(state(nexts[0],table,stack));*/
                }
                break;
            }
            case Variable: {
                stack.push(table.find(n->getValue())->second);
                states.emplace_back(state(nexts[0],table,stack));
                break;
            }
            case If: {
                std::shared_ptr<node> top = stack.myPop();
                if (top->getNodeType() == ConstraintNode) {
                    if (!(dynamic_cast<constraintNode*>(top.get())->isSatisfiable())) {
                        states.emplace_back(state(nexts[1], table, stack));
                    } else {
                        states.emplace_back(state(nexts[0], table, stack));
                        states.emplace_back(state(nexts[1], table, stack));
                    }
                } else {
                    states.emplace_back(state(nexts[0], table, stack));
                    states.emplace_back(state(nexts[1], table, stack));
                }
                break;
            }
            default:
                break;

        }
        return states;
    }

    //definitely have to think carefully about how we do this
    std::vector<state> f(state* s) {
        switch(s->get_position()->getNodeType()) {
            case BinaryExpression:
                return std::vector<state>();
            case Literal:
                return std::vector<state>();
            case Assign:
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

        std::cout << r.body().to_string() << "\n";
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