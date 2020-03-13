//
// Created by hu on 24/02/2020.
//

#include "symEngine.hpp"
#include <limits>
#include <string>

symEngine::symEngine(std::shared_ptr<CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg)}, symboltable{std::move(table)}, event_encountered{false} {}
symEngine::symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg->ccfg)}, symboltable(std::move(table)), event_encountered{false} {}
symEngine::symEngine(std::shared_ptr<CSSA_CFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg->ccfg)}, symboltable(std::move(table)), event_encountered{false} {}
std::vector<std::shared_ptr<trace>> symEngine::execute() {
    z3::context c;

    auto t = get_run(&c, nullptr, ccfg->startNode, ccfg->exitNode);

    std::vector<z3::expr> vec;
    for (const auto &read : ccfg->reads) {
        z3::expr name = c.int_const(read.c_str());
        vec.push_back(name >= INT16_MIN && name <= INT16_MAX);
    }

    z3::solver s(c);

    for (const auto &expr : vec) s.add(expr);

    std::cout << t.to_string() << "\n\n" << std::endl;
    s.add(t);


    if (s.check() == z3::sat) {
        auto model = s.get_model();
        std::cout << "sat\n";
        std::cout << model << "\n" << std::endl;
    } else {
        std::cout << "unsat" << std::endl;
        std::cout <<  s.unsat_core() << std::endl;
    }

    return std::vector<std::shared_ptr<trace>>{nullptr};

}

z3::expr symEngine::get_run(z3::context *c, basicblock *previous, std::shared_ptr<basicblock> start, std::shared_ptr<basicblock> end) {
    auto node = start.get();
    std::vector<z3::expr> constraints;

    if (node->type == Coend) {
        auto coend = dynamic_cast<endConcNode*>(node->statements.back().get());
        int threads = coend->getConcNode()->nexts.size();
        std::stack<z3::expr> expressions;
        std::vector<std::vector<z3::expr>> phinodeAssignments;
        phinodeAssignments.reserve(threads);

        int index = 0;
        for (const auto &st : node->statements) {
            if (auto phi = dynamic_cast<phiNode*>(st.get())) {
                phinodeAssignments.emplace_back();
                auto vars = *phi->get_variables();
                for (const auto &var : vars) {
                    switch (phi->getType()) {
                        case intType: {
                            z3::expr name = c->int_const(phi->getName().c_str());
                            phinodeAssignments[index].push_back(name == c->int_const(var.c_str()));
                            break;
                        }
                        case boolType: {
                            z3::expr name = c->bool_const(phi->getName().c_str());
                            phinodeAssignments[index].push_back(name == c->bool_const(var.c_str()));
                            break;
                        }
                        case arrayIntType: {
                            break;
                        }
                        case arrayBoolType: {
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            ++index;
        }

        for (const auto &exprVec : phinodeAssignments) {
            for (const auto &expr : exprVec) {
                expressions.push(expr);
            }
            z3::expr conjunction = expressions.top();
            expressions.pop();
            while (!expressions.empty()) {
                conjunction = conjunction || expressions.top();
                expressions.pop();
            }
            constraints.push_back(conjunction);
        }
    } else {
        for (auto stmt : node->statements) {
            switch (stmt->getNodeType()) {
                case Assign: {
                    auto assnode = dynamic_cast<assignNode *>(stmt.get());
                    auto name = assnode->getExpr()->getType() == intType
                                ? c->int_const(assnode->getName().c_str())
                                : c->bool_const(assnode->getName().c_str());
                    constraints.push_back(name == evaluate_expression(c, assnode->getExpr()));
                    break;
                }
                case Concurrent: {
                    auto endConc = get_end_of_concurrent_node(node);
                    std::stack<z3::expr> expressions;
                    int i = 0;
                    bool changed_event = false;
                    for (const auto &nxt : node->nexts) {
                        if (event_encountered) {
                            event_encountered = false;
                            changed_event = true;
                        }
                        expressions.push(get_run(c, node, nxt, endConc->parents[i++].lock()));
                    }
                    z3::expr final = expressions.top();
                    expressions.pop();
                    while (!expressions.empty()) {
                        final = final && expressions.top();
                        expressions.pop();
                    }

                    if (changed_event) {
                        for (const auto &constraint : constraints) {
                            final = final && constraint;
                        }
                        event_encountered = true;
                        return final;
                    }
                    constraints.push_back(final);
                    node = endConc->parents[0].lock().get();
                    break;
                }
                case If: {
                    std::shared_ptr<basicblock> firstCommonChild = find_common_child(node);
                    bool changed_event = false;
                    if (event_encountered) {
                        event_encountered = false;
                        changed_event = true;
                    }
                    z3::expr truebranch = get_run(c, node, node->nexts[0], firstCommonChild);
                    if (event_encountered) {
                        event_encountered = false;
                        changed_event = true;
                    }
                    z3::expr falsebranch = get_run(c, node, node->nexts[1], firstCommonChild);
                    if (event_encountered) {
                        changed_event = true;
                    }
                    z3::expr final = z3::ite(
                            evaluate_expression(c, dynamic_cast<ifElseNode *>(stmt.get())->getCondition()),
                            truebranch, falsebranch);

                    for (const auto &constraint : constraints) {
                        final = final && constraint;
                    }
                    if (changed_event) return final;

                    while (firstCommonChild && firstCommonChild->type == Condition) {
                        if (firstCommonChild->statements.back()->getNodeType() == If) {
                            firstCommonChild = find_common_child(firstCommonChild.get());
                        } else { //Event
                            return final;
                        }
                    }
                    if (firstCommonChild && !firstCommonChild->nexts.empty() && firstCommonChild != end)
                        return final && get_run(c, firstCommonChild.get(), firstCommonChild->nexts[0], end);
                    else return final;
                }
                case Event: {
                    z3::expr condition = evaluate_expression(c, dynamic_cast<eventNode*>(stmt.get())->getCondition());
                    bool changed_event = false;
                    if (event_encountered) {
                        changed_event = true;
                        event_encountered = false;
                    }
                    z3::expr truebranch = get_run(c, node, node->nexts[0], end);

                    if (changed_event) event_encountered = true;
                    if (end != ccfg->exitNode) {
                        auto res = ccfg->concurrent_events.find(node);
                        if (res != ccfg->concurrent_events.end()) {
                            if (!event_encountered) {
                                z3::expr finalCond = condition;
                                for (const auto &event : res->second) {
                                    finalCond = finalCond && evaluate_expression(c, dynamic_cast<eventNode*>(event->statements.back().get())->getCondition());
                                }
                                z3::expr final = z3::ite(condition, truebranch, c->bool_val(true));
                                for (const auto &constraint : constraints) {
                                    final = final && constraint;
                                }
                                z3::expr finalConstraint =
                                    z3::ite(finalCond
                                           , final && get_run(c, end.get(), end->nexts[0], ccfg->exitNode)
                                           , c->bool_val(true)
                                           );
                                event_encountered = true;
                                return finalConstraint;
                            } else {
                                return z3::ite(condition, truebranch, c->bool_val(true));
                            }
                        }
                        for (const auto &nxt : end->nexts) {
                            truebranch = truebranch && get_run(c, end.get(), nxt, ccfg->exitNode);
                        }
                    }
                    event_encountered = true;
                    z3::expr final = z3::ite(condition, truebranch, c->bool_val(true));
                    for (const auto &constraint : constraints) {
                        final = final && constraint;
                    }
                    return final;
                }
                case AssignArrField:
                case EndConcurrent:
                case Sequential:
                case While:
                case EndFi:
                case Write:
                case Read:
                case Literal:
                case ArrayAccess:
                case ArrayLiteral:
                case Variable:
                case BinaryExpression:
                case UnaryExpression:
                case Skip:
                case BasicBlock:
                    constraints.push_back(c->bool_val(true));
                    break;
                case Phi: {
                    auto phi = dynamic_cast<phiNode *>(stmt.get());
                    auto parents = node->parents;

                    switch (phi->getType()) {
                        case intType: {
                            z3::expr name = c->int_const(phi->getName().c_str());
                            for (auto i = 0; i < parents.size(); ++i) {
                                if (previous == parents[i].lock().get()) {
                                    constraints.push_back(name == c->int_const(phi->get_variables()->at(i).c_str()));
                                    break;
                                }
                            }
                            break;
                        }
                        case boolType: {
                            z3::expr name = c->bool_const(phi->getName().c_str());
                            for (auto i = 0; i < parents.size(); ++i) {
                                if (previous == parents[i].lock().get()) {
                                    constraints.push_back(name == c->bool_const(phi->get_variables()->at(i).c_str()));
                                    break;
                                }
                            }
                            break;
                        }
                        case arrayIntType:
                            break;
                        case arrayBoolType:
                            break;
                        case okType:
                            break;
                        case errorType:
                            break;
                    }
                    break;
                }
                case Pi: {
                    auto pi = dynamic_cast<piNode *>(stmt.get());
                    auto vars = pi->get_variables();
                    std::stack<z3::expr> expressions;
                    switch (pi->getType()) {
                        case intType: {
                            z3::expr name = c->int_const(pi->getName().c_str());
                            for (const auto &conflict : *vars) {
                                expressions.push(name == c->int_const(conflict.c_str()));
                            }
                            break;
                        }
                        case boolType: {
                            z3::expr name = c->bool_const(pi->getName().c_str());
                            for (const auto &conflict : *vars) {
                                expressions.push(name == c->bool_const(conflict.c_str()));
                            }
                            break;
                        }
                        case arrayIntType: {
                            break;
                        }
                        case arrayBoolType: {
                            break;
                        }
                        default:
                            break;
                    }

                    z3::expr final = expressions.top();
                    expressions.pop();
                    while (!expressions.empty()) {
                        final = final || expressions.top();
                        expressions.pop();
                    }
                    constraints.push_back(final);
                    break;
                }
            }
        }
    }
    z3::expr final = constraints.back();
    constraints.pop_back();
    for (const auto &constraint : constraints) {
        final = final && constraint;
    }

    if (node == end.get() || node->nexts.empty() || event_encountered) {
        return final;
    } else {
        return final && get_run(c, node, node->nexts[0], end);
    }
}

std::shared_ptr<basicblock> symEngine::find_common_child(basicblock *parent) {
    for (const auto &blk : ccfg->nodes) {
        if (auto fi = dynamic_cast<fiNode*>(blk->statements.back().get())) {
            if (fi->get_parent() == parent) {
                return blk;
            }
        }
    }
    return nullptr;
}

z3::expr symEngine::evaluate_expression(z3::context *c, const expressionNode *expr) {
    switch (expr->getNodeType()) {
        case Read: {
            return c->int_const(dynamic_cast<const readNode*>(expr)->getName().c_str());
        }
        case Literal: {
            auto lit = dynamic_cast<const literalNode*>(expr);
            return lit->getType() == intType
                ?  c->int_val(std::stoi(lit->value))
                : c->bool_val(lit->value == "true")
                ;
        }
        case ArrayAccess:
            break;
        case ArrayLiteral:
            break;
        case Variable: {
            return expr->getType() == intType
                ? c->int_const(dynamic_cast<const variableNode*>(expr)->name.c_str())
                : c->bool_const(dynamic_cast<const variableNode*>(expr)->name.c_str())
                ;
        }
        case BinaryExpression: {
            auto binexpr = dynamic_cast<const binaryExpressionNode*>(expr);
            return evaluate_operator
                    ( evaluate_expression(c, binexpr->getLeft())
                    , evaluate_expression(c, binexpr->getRight())
                    , binexpr->getOperator()
                    );
        }
        case UnaryExpression: {
            auto unexpr = dynamic_cast<const unaryExpressionNode*>(expr);
            z3::expr exp = evaluate_expression(c, unexpr->getExpr());
            return evaluate_operator(exp, exp, unexpr->getOperator());
        }
        case Assign:
        case AssignArrField:
        case Concurrent:
        case EndConcurrent:
        case Sequential:
        case While:
        case If:
        case EndFi:
        case Write:
        case Event:
        case Skip:
        case BasicBlock:
        case Phi:
        case Pi:
            assert(false);
            break;
    }
}

z3::expr symEngine::evaluate_operator(const z3::expr& left, const z3::expr& right, op _operator) {
    switch (_operator) {
        case PLUS:
            return left + right;
        case MINUS:
            return left - right;
        case MULT:
            return left * right;
        case DIV:
            return left / right;
        case MOD:
            return left % right;
        case NOT:
            return !left;
        case AND:
            return left && right;
        case OR:
            return left || right;
        case LE:
            return left < right;
        case LEQ:
            return left <= right;
        case GE:
            return left > right;
        case GEQ:
            return left >= right;
        case EQ:
            return left == right;
        case NEQ:
            return left != right;
        default:
            return -left;
    }
}

std::shared_ptr<basicblock> symEngine::get_end_of_concurrent_node(basicblock *node) {
    for (const auto &blk : ccfg->nodes) {
        if (blk->type == Coend) {
            if (auto coend = dynamic_cast<endConcNode*>(blk->statements.back().get())) {
                if (coend->getConcNode().get() == node) {
                    return blk;
                }
            }
        }
    }
    assert(false);
    return nullptr;
}