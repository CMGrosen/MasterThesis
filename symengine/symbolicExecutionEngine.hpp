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
        auto minmax = getMinMax("read");
        state s = state(treeAndSymTable.first, treeAndSymTable.second);
        min_constraint = minmax[0];
        max_constraint = minmax[1];

        constraints.insert(min_constraint);
        constraints.insert(max_constraint);

        frontier.push(s);
        auto res = compute_statements();
        return res;
    }

private:
    std::shared_ptr<constraint> min_constraint;
    std::shared_ptr<constraint> max_constraint;
    std::unordered_set<state> explored;
    stateQueue<state> frontier;
    std::unordered_set<std::shared_ptr<constraint>> constraints;
    std::vector<state> testCases;

    std::vector<std::shared_ptr<constraint>> getMinMax(const std::string &name) const {
        std::shared_ptr<node> start = std::make_shared<node>(node(intType,Variable, name));
        std::shared_ptr<node> last = std::make_shared<node>(node(intType,Literal, std::to_string(INT16_MIN)));
        start->setNext(last);
        last->setNext(std::make_shared<node>(node(intType, BinaryExpression, GEQ)));

        std::shared_ptr<node> start2 = std::make_shared<node>(node(intType,Variable, name));
        std::shared_ptr<node> last2 = std::make_shared<node>(node(intType,Literal, std::to_string(INT16_MAX)));
        start2->setNext(last2);
        last2->setNext(std::make_shared<node>(node(intType, BinaryExpression, LEQ)));

        auto leq = std::make_shared<constraint>(constraint(intType, start));
        auto geq = std::make_shared<constraint>(constraint(intType, start2));

        return std::vector<std::shared_ptr<constraint>>{std::move(leq), std::move(geq)};
    }
    std::vector<z3::expr> compute_statements() {
        while(!frontier.empty()) {
            state s = frontier.myPop();
            auto inserted = explored.insert(s);
            if (inserted.second) {
                std::vector<state> states = compute_statements_helper(s);
                for (auto state : states)
                    frontier.push(state);
            }
            std::cout << "here\n";
        }
        return std::vector<z3::expr>{};
    }

    std::vector<state> compute_statements_helper(state &s) {
        std::shared_ptr<node> n = s.get_position();
        std::vector<state> states = std::vector<state>{};
        stateStack<std::shared_ptr<node>> stack = s.stack;
        std::vector<std::shared_ptr<constraint>> path_condition = s.path_condition;
        std::unordered_map<std::string, int32_t> ssa_map = s.ssa_map;
        if (!n) {return states;}
        std::vector<std::shared_ptr<node>> nexts = n->getNexts();
        while (n->getNodeType() == Literal) {
            stack.push(std::make_shared<node>(node(n->getType(), Literal, n->getValue())));
            n = nexts[0];
            nexts = n->getNexts();
        }
        auto table = s.getTable();
        switch (n->getNodeType()) {
            case Assign: {
                std::string name = n->getValue();
                std::shared_ptr<node> top = stack.myPop();
                table.find(name)->second = top;
                auto it = ssa_map.find(name);
                it->second++;
                if (top->getNodeType() == Read) {
                    auto minmax = getMinMax(std::to_string(it->second) + name);
                    for (auto &i : minmax) {
                        auto res = constraints.insert(i);
                        path_condition.push_back(*res.first);
                    }
                } else if (top->getNodeType() == Variable || top->getNodeType() == Literal) {
                    top->setNext(std::make_shared<node>(node(top->getType(), Assign, std::to_string(it->second) + n->getValue())));
                    auto res = constraints.insert(std::make_shared<constraint>(constraint(top->getType(),top)));
                    path_condition.push_back(*res.first);
                } else {
                    auto last = top->getLast();
                    if (!last) last = top;
                    last->setNext(std::make_shared<node>(node(top->getType(), Assign, std::to_string(it->second)+ n->getValue())));
                    auto res = constraints.insert(std::make_shared<constraint>(constraint(top->getType(),top)));
                    path_condition.push_back(*res.first);
                }
                if (!nexts.empty())
                    states.emplace_back(state(nexts[0], table, stack, path_condition, ssa_map));
                break;
            }
            case BinaryExpression: {
                std::shared_ptr<node> right = stack.myPop();
                std::shared_ptr<node> left = stack.myPop();
                if (left->getNodeType() == Literal && right->getNodeType() == Literal) {
                    auto res = DST::compute_new_literal(left, right, n->getOperator(), n->getType());
                    if (res.first) {
                        stack.push(res.second);
                        states.emplace_back(state(nexts[0], table, stack, path_condition, ssa_map));
                    } else
                        testCases.emplace_back(s);
                } else {
                    std::shared_ptr<node> last = left->getLast();
                    if (!last) last = left;
                    last->setNext(right);
                    last = right->getLast();
                    if (!last) last = right;
                    last->setNext(std::make_shared<node>(node(n->getType(),n->getNodeType(), n->getOperator())));
                    stack.push(std::make_shared<constraint>(constraint(n->getType(),left)));
                    states.emplace_back(state(nexts[0], table, stack, path_condition, ssa_map));
                }
            }
            break;
            case Read: {
                std::shared_ptr<constraintNode> cn = std::make_shared<constraintNode>(constraintNode(intType));
                cn->constraints.push_back(*constraints.find(min_constraint));
                cn->constraints.push_back(*constraints.find(max_constraint));
                cn->setNodeType(Read);
                stack.push(cn);
                states.emplace_back(state(nexts[0], table, stack, path_condition, ssa_map));
                break;
            }
            case Variable: {
                auto res = table.find(n->getValue());
                if (res->second->getNodeType() == Literal)
                    stack.push(std::make_shared<node>(node(n->getType(),Literal, res->second->getValue())));
                else
                    stack.push(std::make_shared<node>(node(n->getType(),Variable,std::to_string(ssa_map.find(n->getValue())->second) + n->getValue())));
                states.emplace_back(state(nexts[0],table,stack, path_condition, ssa_map));
                break;
            }
            case If: {
                std::shared_ptr<node> top = stack.myPop();
                if (top->getNodeType() == ConstraintNode) {
                    auto conNode = dynamic_cast<constraint*>(top.get());
                    if (conNode->isSatisfiable()) {
                        auto res = constraints.insert(deepcopy(*conNode));
                        path_condition.push_back(*res.first);
                        states.emplace_back(state(nexts[0], table, stack, path_condition, ssa_map));
                        path_condition.pop_back();
                    }
                    auto last = conNode->_constraint->getLast();
                    if (!last) last = conNode->_constraint;
                    last->setNext(std::make_shared<node>(node(boolType, UnaryExpression, NOT)));
                    if (conNode->isSatisfiable()) {
                        auto res = constraints.insert(deepcopy(*conNode));
                        path_condition.push_back(*res.first);
                        states.emplace_back(state(nexts[1], table, stack, path_condition, ssa_map));
                    }
                } else {
                    if (top->getValue() == "true") {
                        states.emplace_back(state(nexts[0], table, stack, path_condition, ssa_map));
                    } else {
                        states.emplace_back(state(nexts[1], table, stack, path_condition, ssa_map));
                    }
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



    std::shared_ptr<constraint> deepcopy(const constraint con) {
        return deepcopy_helper(con._constraint, std::make_shared<constraint>(constraint(con.getType(), nullptr)), nullptr);
    }
    std::shared_ptr<constraint> deepcopy_helper(const std::shared_ptr<node> &n, std::shared_ptr<constraint> result, const std::shared_ptr<node> &last) {
        auto np = std::make_shared<node>(node(n->getType(),n->getNodeType(),n->getOperator(),n->getValue()));
        if (!result->_constraint) {
            result->_constraint = np;
        } else {
            last->setNext(np);
        }
        if (n->getNexts().empty()) {
            return result;
        }
        return deepcopy_helper(n->getNexts()[0], result, np);
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