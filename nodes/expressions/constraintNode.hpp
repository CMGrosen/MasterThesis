//
// Created by CMG on 29/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINTNODE_HPP

#include <nodes/node.hpp>
#include <z3++.h>

class constraintNode : public node {
public:
    constraintNode(Type t) : node(t, ConstraintNode) {}

    bool isSatisfiable() {
        z3::context c;
        const char *n = getValue().c_str();
        bool leftConstraint = false;
        bool rightConstraint = false;
        z3::expr name = (type == intType) ? c.int_const(n) : c.bool_const(n);
        //if (checkedAndSatisfiable.first) return std::pair<bool, z3::expr>(checkedAndSatisfiable.second, name);
        /*bool satisfiable = std::pair<bool, z3::expr>(true, name);
        z3::expr l = name;
        z3::expr r = name;
        z3::solver s(c);
        //z3::expr completeExpr = c.string_val("true");
        //bool completeExprChanged = false;
        for (auto &e : _constraints) {
            if (auto ce = dynamic_cast<constraintNode *>(e->getLeft())) {
                satisfiable = ce->isSatisfiable();
                l = satisfiable.second;
                leftConstraint = true;
            }
            if (satisfiable.first) {
                if (auto ce = dynamic_cast<constraintNode *>(e->getRight())) {
                    satisfiable = ce->isSatisfiable();
                    r = satisfiable.second;
                    rightConstraint = true;
                }
            }

            if (!satisfiable.first) return std::pair<bool, z3::expr>(false, name);
            std::string val;
            if (e->getLeft()->getNodeType() == Variable) {
                l = name;
            }
            if (e->getRight()->getNodeType() == Variable) {
                r = name;
            }
            if (e->getLeft()->getNodeType() == Literal) {
                val = e->getLeft()->getValue();
                l = e->getLeft()->getType() == intType ? c.int_val(val.c_str()) : c.bool_val(val.c_str());
            }
            if (e->getRight()->getNodeType() == Literal) {
                val = e->getRight()->getValue();
                r = e->getRight()->getType()  == intType ? c.int_val(val.c_str()) : c.bool_val(val.c_str());
            }

            switch (e->getOperator()) {
                case PLUS:
                    l = l + r;
                    break;
                case LE:
                    l = l < r;
                    break;
                case LEQ:
                    l = l <= r;
                    break;
                case GE:
                    l = l > r;
                    break;
                case GEQ:
                    l = l >= r;
                    break;
                case EQ:
                    l = l == r;
                    break;
                case NEQ:
                    l = l != r;
                    break;
                default:
                    std::cout << "something went wrong" << std::endl;
            }
            s.add(l);

        }
        checkedAndSatisfiable.second = s.check() == z3::sat;
        checkedAndSatisfiable.first = true;
        //expr = completeExpr;
        return std::pair<bool, z3::expr>(checkedAndSatisfiable.second, l);*/
    }

private:
    std::vector<std::shared_ptr<binaryExpressionNode>> _constraints;
    std::pair<bool, bool> checkedAndSatisfiable = std::pair<bool,bool>(false, false);
    //z3::expr expr = z3::expr(z3::context().string_val(""));
};


#endif //ANTLR_CPP_TUTORIAL_CONSTRAINTNODE_HPP
