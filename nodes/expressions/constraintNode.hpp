//
// Created by CMG on 29/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_CONSTRAINTNODE_HPP
#define ANTLR_CPP_TUTORIAL_CONSTRAINTNODE_HPP

#include "binaryExpressions/binaryExpressionNode.hpp"
#include <z3++.h>

class constraintNode : public expressionNode {
public:
    constraintNode(Type t) : constraints{std::vector<std::shared_ptr<binaryExpressionNode>>{}} {type = t;}
    constraintNode(std::shared_ptr<binaryExpressionNode> constraint) :
        constraints{std::vector<std::shared_ptr<binaryExpressionNode>>{std::move(constraint)}} {type = constraints[0]->getType();}
    constraintNode(std::vector<std::shared_ptr<binaryExpressionNode>> constraints) :
        constraints{std::move(constraints)} {type = constraints[0]->getType();}

    std::vector<std::shared_ptr<binaryExpressionNode>> getConstraints() const {return constraints;}

    bool isSatisfiable(const variableNode *var) {
        z3::context c;
        const char *n = (var->name).c_str();
        z3::expr name = (type == intType) ? c.int_const(n) : c.bool_const(n);

        z3::solver s(c);

        for (auto &e : constraints) {
            std::string val = dynamic_cast<literalNode*>(e->getRight())->value;
            switch (e->getOperator()) {
                case LE:
                    s.add(name < std::stoi(val));
                case LEQ:
                    s.add(name <= std::stoi(val));
                case GE:
                    s.add(name > std::stoi(val));
                case GEQ:
                    s.add(name >= std::stoi(val));
                case EQ:
                    s.add(name == ((type == intType) ? std::stoi(val) : (val == "true")));
                case NEQ:
                    s.add(name == ((type == intType) ? std::stoi(val) : (val == "true")));
                default:
                    std::cout << "something went wrong" << std::endl;
            }
        }

        return s.check() == z3::sat;
    }

private:
    std::vector<std::shared_ptr<binaryExpressionNode>> constraints;
};


#endif //ANTLR_CPP_TUTORIAL_CONSTRAINTNODE_HPP
