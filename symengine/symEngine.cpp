//
// Created by hu on 24/02/2020.
//

#include "symEngine.hpp"
#include <limits>
#include <z3++.h>
#include <string>

symEngine::symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg)}, symboltable(std::move(table)) {}

std::vector<std::shared_ptr<trace>> symEngine::execute() {
    std::set<std::shared_ptr<basicblock>> visited_blocks;
    std::vector<std::shared_ptr<trace>> traces;
    std::vector<std::string> inputs;

    return std::vector<std::shared_ptr<trace>>{get_trace(ccfg->ccfg->startNode)};

}

std::pair<std::shared_ptr<basicblock>, bool> symEngine::run_trace(std::shared_ptr<trace> t) {

}

z3::expr get_expr (z3::context *c, expressionNode *node, int *currentRead) {
    switch (node->getNodeType()) {
        case Read: {
            z3::expr x = c->int_const(("-" + std::to_string(*currentRead++) + "readVal").c_str());
            return (c->int_val(INT16_MIN) <= x <= c->int_val(INT16_MAX));
        } case Literal: {
            auto lit = dynamic_cast<literalNode*>(node);
            if (lit->getType() == intType) {
                return (c->int_val(std::stoi(lit->value)));
            } else {
                return (c->bool_val(lit->value == "true"));
            }
        } case ArrayAccess: {
            auto arrAcc = dynamic_cast<arrayAccessNode*>(node);
            //stack.pu
            break;
        } case Variable: {
            auto var = dynamic_cast<variableNode*>(node);
            if (var->getType() == intType)
                return (c->int_const(var->name.c_str()));
            else
                return (c->bool_const(var->name.c_str()));
        } case BinaryExpression: {
            auto binOp = dynamic_cast<binaryExpressionNode*>(node);
            z3::expr left = get_expr(c, binOp->getLeft(), currentRead);
            z3::expr right = get_expr(c, binOp->getRight(), currentRead);
            switch (binOp->getOperator()) {
                case PLUS:
                    return (left + right);
                case MINUS:
                    return (left - right);
                case MULT:
                    return (left * right);
                case DIV:
                    return (left / right);
                case MOD:
                    return (left % right);
                case AND:
                    return (left && right);
                case OR:
                    return (left || right);
                case LE:
                    return (left < right);
                case LEQ:
                    return (left <= right);
                case GE:
                    return (left > right);
                case GEQ:
                    return (left >= right);
                case EQ:
                    return (left == right);
                case NEQ:
                    return (left != right);
                default:
                    break;
            }
            break;
        } case UnaryExpression: {
            auto unOp = dynamic_cast<unaryExpressionNode*>(node);
            z3::expr val = get_expr(c, unOp->getExpr(), currentRead);
            if (unOp->getOperator() == NOT) {
                return (!val);
            } else {
                return (-val);
            }
        } default:
            break;
    }
}

std::shared_ptr<trace> symEngine::get_trace(std::shared_ptr<basicblock> node) {
    using namespace z3;
    z3::context c;
    std::vector<z3::expr> vec;
    int currentRead = 0;
    int it = 1;
    while (node) {
        for (const auto &stmt : node->statements) {
            switch (stmt->getNodeType()) {
                case Assign: {
                    auto assNode = dynamic_cast<assignNode*>(stmt.get());
                    switch (symboltable.find(assNode->getOriginalName())->second->getType()) {
                        case intType: {
                            z3::expr x = c.int_const(assNode->getName().c_str());
                            vec.push_back(x == get_expr(&c, assNode->getExpr(), &currentRead));
                            break;
                        } case boolType: {
                            z3::expr x = c.bool_const(assNode->getName().c_str());
                            vec.push_back(x == get_expr(&c, assNode->getExpr(), &currentRead));
                            break;
                        } case arrayIntType: {
                            auto arrLit = dynamic_cast<arrayLiteralNode*>(assNode->getExpr());
                            sort I = c.int_sort();
                            sort A = c.array_sort(I, I);
                            auto cStr = assNode->getName().c_str();
                            expr a1 = c.constant(cStr, A);
                            vec.push_back(a1);
                            const auto &lit = arrLit->getArrLit();
                            for (int i = 0; i < arrLit->getArrLit().size(); ++i) {
                                vec.push_back(z3::store(a1, i, get_expr(&c, lit[i].get(), &currentRead)));
                            }
                            break;
                        }
                        case arrayBoolType: {
                            auto arrLit = dynamic_cast<arrayLiteralNode*>(assNode->getExpr());
                            sort B = c.bool_sort();
                            sort BB = c.array_sort(B, B);
                            auto cStr = assNode->getName().c_str();
                            expr b1 = c.constant(cStr, BB);
                            vec.push_back(b1);
                            const auto &lit = arrLit->getArrLit();
                            for (int i = 0; i < arrLit->getArrLit().size(); ++i) {
                                vec.push_back(z3::store(b1, i, get_expr(&c, lit[i].get(), &currentRead)));
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                case AssignArrField: {

                    break;
                }
                case Concurrent: {

                    break;
                }
                case EndConcurrent: {

                    break;
                }
                case Sequential: {

                    break;
                }
                case While: {
                    auto wNode = dynamic_cast<whileNode*>(stmt.get());
                    vec.push_back(!get_expr(&c, wNode->getCondition(), &currentRead));
                    break;
                }
                case If: {
                    auto ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                    vec.push_back(get_expr(&c, ifNode->getCondition(), &currentRead));
                    break;
                }
                case Event: {
                    auto evNode = dynamic_cast<eventNode*>(stmt.get());
                    vec.push_back(get_expr(&c, evNode->getCondition(), &currentRead));
                    break;
                }
                case Skip: {

                    break;
                }
                case Phi: {
                    auto phi = dynamic_cast<phiNode*>(stmt.get());
                    auto res = symboltable.find(phi->getOriginalName());
                    switch (res->second->getType()) {
                        case intType:
                            vec.push_back(c.int_const(phi->getName().c_str()) == c.int_const(phi->get_variables()->at(1).c_str()));
                            break;
                        case boolType:
                            vec.push_back(c.bool_const(phi->getName().c_str()) == c.bool_const(phi->get_variables()->at(1).c_str()));
                            break;
                        case arrayIntType: {
                            break;
                        }
                        case arrayBoolType: {
                            break;
                        }
                        default:
                            break;
                        }
                        //vec.push_back(get_expr(&c, phi->get_variables(), &currentRead));
                    break;
                }
                case Pi:{

                    break;
                }
                default:
                    break;
            }
        }
        if (node->nexts.empty()) break;
        node = node->type == Loop ? node->nexts[1] : node->nexts[0];
    }

    z3::solver s(c);
    for (const auto &e : vec) s.add(e);
    if (s.check() == z3::sat) {
        std::cout << "sat\n";
        std::cout << s.get_model() << "\n";
    } else {
        std::cout << "unsat\n";
    }
    return std::make_shared<trace>(trace(std::vector<edge>{}, std::vector<std::string>{}, nullptr));
}