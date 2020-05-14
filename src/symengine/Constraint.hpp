//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP

#include <nodes/nodes.hpp>
#include <symengine/stateStack.hpp>
#include <z3++.h>

class constraint : public node {
public:
    constraint(Type _type, std::shared_ptr<node> _expression) : node(_type, ConstraintNode), _constraint{std::move(_expression)} {}

    std::shared_ptr<node> _constraint;

    bool isSatisfiable() const {
        z3::context c;
        stateStack<std::string> stack;
        stateStack<z3::expr> exprStack;
        std::shared_ptr<node> current = _constraint;
        while (current) {
            switch (current->getNodeType()) {
                case Literal:
                    if (current->getType() == intType)
                        exprStack.push(c.int_val(std::stoi(current->getValue())));
                    else
                        exprStack.push(c.bool_val(stob(current->getValue())));
                break;
                case Variable:
                    if (current->getType() == intType) {
                        exprStack.push(c.int_const(current->getValue().c_str()));
                    } else {
                        exprStack.push(c.bool_const(current->getValue().c_str()));
                    }
                break;
                case BinaryExpression: {
                    z3::expr r = exprStack.myPop();
                    z3::expr l = exprStack.myPop();
                    switch (current->getOperator()) {
                        case PLUS:
                            exprStack.push(l + r);
                            break;
                        case LE:
                            exprStack.push(l < r);
                            break;
                        case LEQ:
                            exprStack.push(l <= r);
                            break;
                        case GE:
                            exprStack.push(l > r);
                            break;
                        case GEQ:
                            exprStack.push(l >= r);
                            break;
                        case EQ:
                            exprStack.push(l == r);
                            break;
                        case NEQ:
                            exprStack.push(l != r);
                            break;
                        case MINUS:
                            exprStack.push(l - r);
                            break;
                        case MULT:
                            exprStack.push(l * r);
                            break;
                        case DIV:
                            exprStack.push(l / r);
                            break;
                        case MOD:
                            exprStack.push(l % r);
                            break;
                        default:
                            std::cout << "this shouldn't happen\n";
                            break;
                    }
                    break;
                } case UnaryExpression: {
                    z3::expr top = exprStack.myPop();
                    switch (current->getOperator()) {
                        case NEG:
                            exprStack.push(-top);
                            break;
                        case NOT:
                            exprStack.push(!top);
                            break;
                        default:
                            std::cout << "this shouldn't happen\n";
                            break;
                    }
                    break;
                }
                default:
                    std::cout << "this shouldn't happen\n";
                    break;
            }
            if (!current->getNexts().empty())
                current = current->getNexts()[0];
            else current = nullptr;
        }
        z3::solver s(c);
        s.add(exprStack.myPop());
        return s.check() == z3::sat;

    }
private:
    bool stob (const std::string &val) const {
        return val == "true";
    }
};

#endif //ANTLR_CPP_TUTORIAL_CONSTRAINT_HPP
