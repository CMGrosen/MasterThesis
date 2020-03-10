//
// Created by hu on 24/02/2020.
//

#include "symEngine.hpp"
#include <limits>
#include <string>

symEngine::symEngine(std::shared_ptr<CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg)}, symboltable{std::move(table)} {}
symEngine::symEngine(std::shared_ptr<SSA_CCFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg->ccfg)}, symboltable(std::move(table)) {}
symEngine::symEngine(std::shared_ptr<CSSA_CFG> ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) : ccfg{std::move(ccfg->ccfg)}, symboltable(std::move(table)) {}
std::vector<std::shared_ptr<trace>> symEngine::execute() {
    std::set<std::shared_ptr<basicblock>> visited_blocks;
    std::vector<std::shared_ptr<trace>> traces;
    std::vector<std::string> inputs;

    //auto t = find_race_condition();
    //auto t = get_run();
    //return t;
    z3::context c;
    auto t = get_run(&c, nullptr, ccfg->startNode, ccfg->exitNode);

    z3::solver s(c);
    std::cout << t.to_string() << "\n\n" << std::endl;
    s.add(t);
    s.add(c.int_const("-T_b_2") == c.int_const("b_5"));

    //s.add(c.int_const("a_4") == c.int_val(8));
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
    std::stack<z3::expr> evaluated;


    if (node->type == Coend) {
        return c->bool_val(true);
    }

    for (const auto &stmt : node->statements) {
        if (stmt->getNodeType() == Phi) {
            auto phi = dynamic_cast<phiNode*>(stmt.get());
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
        } else if (stmt->getNodeType() == Pi) {
            auto pi = dynamic_cast<piNode*>(stmt.get());
            auto vars = pi->get_variables();
            std::stack<z3::expr> expressions;
            switch (pi->getType()) {
                case intType: {
                    z3::expr name = c->int_const(pi->getName().c_str());
                    for (const auto &conflict : *vars) {
                        expressions.push(name == c->int_const(conflict.c_str()));
                    }
                    break;
                } case boolType: {
                    z3::expr name = c->bool_const(pi->getName().c_str());
                    for (const auto &conflict : *vars) {
                        expressions.push(name == c->bool_const(conflict.c_str()));
                    }
                    break;
                } case arrayIntType: {
                        break;
                } case arrayBoolType: {
                    break;
                } default: break;
            }

            z3::expr final = expressions.top();
            expressions.pop();
            while (!expressions.empty()) {
                final = final || expressions.top();
                expressions.pop();
            }
            constraints.push_back(final);
        } else if (stmt->getNodeType() == Concurrent) {
            auto endConc = get_end_of_concurrent_node(node);
            std::stack<z3::expr> expressions;
            int i = 0;
            for (const auto &nxt : node->nexts) {
                expressions.push(get_run(c, node, nxt, endConc->parents[i++].lock()));
            }
            z3::expr final = expressions.top();
            expressions.pop();
            while (!expressions.empty()) {
                final = final && expressions.top();
                expressions.pop();
            }

            auto coend = dynamic_cast<endConcNode*>(endConc->statements.back().get());
            int threads = coend->getConcNode()->nexts.size();
            std::vector<std::vector<z3::expr>> phinodeAssignments;
            phinodeAssignments.reserve(threads);

            int index = 0;
            for (const auto &st : endConc->statements) {
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
                final = final && conjunction;
            }

            if (endConc->nexts.empty()) {
                return final;
            } else {
                return final && get_run(c, endConc.get(), endConc->nexts[0], end);
            }
        } else if (stmt->getNodeType() == EndConcurrent) {
            constraints.push_back(c->bool_val(true));
        } else {
    auto current = dynamic_cast<unpackedstmt *>(stmt.get())->_this;

    while (current) {
        switch (current->getNodeType()) {
            case Assign: {
                switch (current->getType()) {
                    case intType:
                        constraints.push_back(c->int_const(current->value.c_str()) == evaluated.top());
                        break;
                    case boolType:
                        constraints.push_back(c->bool_const(current->value.c_str()) == evaluated.top());
                        break;
                    case arrayIntType:
                        break;
                    case arrayBoolType:
                        break;
                    case okType:
                        break;
                    case errorType:
                        break;
                }
                evaluated.pop();
                break;
            }
            case If: {
                std::shared_ptr<basicblock> firstCommonChild = find_common_child(node);
                z3::expr final = z3::ite(evaluated.top(), get_run(c, node, node->nexts[0], firstCommonChild),
                                         get_run(c, node, node->nexts[1], firstCommonChild));

                for (const auto &constraint : constraints) {
                    final = final && constraint;
                }

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
            case Read: {
                z3::expr intermediate = c->int_const(current->value.c_str());
                constraints.push_back((intermediate >= INT16_MIN) && (intermediate <= INT16_MAX));
                evaluated.push(intermediate);
                break;
            }
            case Literal: {
                if (current->getType() == intType)
                    evaluated.push(c->int_val(std::stoi(current->value)));
                else
                    evaluated.push(c->bool_val(current->value == "true"));
                break;
            }
            case Event: {
                z3::expr finalconstraint = z3::ite(evaluated.top(), get_run(c, node, node->nexts[0], end),
                                                   c->bool_val(true));
                for (const auto &constraint : constraints) {
                    finalconstraint = finalconstraint && constraint;
                }
                return finalconstraint;
            }
            case Variable: {
                switch (current->getType()) {
                    case intType:
                        evaluated.push(c->int_const(current->value.c_str()));
                        break;
                    case boolType:
                        evaluated.push(c->bool_const(current->value.c_str()));
                        break;
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
            case BinaryExpression: {
                z3::expr right = evaluated.top();
                evaluated.pop();
                z3::expr left = evaluated.top();
                evaluated.pop();
                evaluated.push(evaluate_expression(left, right, current->_operator));
                break;
            }
            case UnaryExpression: {
                z3::expr top = evaluated.top();
                evaluated.pop();
                evaluated.push(evaluate_expression(top, top, current->_operator));
                break;
            }
            case ArrayAccess:

            case ArrayLiteral:

            case AssignArrField:

            case Skip:
            case Write:
            case BasicBlock:
            case Sequential:
            case While:
                constraints.push_back(c->bool_val(true));
                break;
            default:
                break;
        }
        current = current->next;
    }}}

    z3::expr final = constraints.back();
    constraints.pop_back();
    for (const auto &constraint : constraints) {
        final = final && constraint;
    }

    if (node == end.get() || node->nexts.empty()) {
        return final;
    } else {
        return final && get_run(c, node, node->nexts[0], end);
    }
}


std::shared_ptr<basicblock> symEngine::find_common_child(basicblock *parent) {
    std::set<std::shared_ptr<basicblock>> found;
    found.insert(parent->nexts[0]);
    basicblock *current = parent->nexts[0].get();
    for (const auto &blk : ccfg->nodes) {
        for (const auto &ifparent : blk->getIfParents()) {
            if (ifparent && ifparent == parent) {
                return blk;
            }
        }
    }
    return nullptr;
}

z3::expr symEngine::evaluate_expression(const z3::expr& left, const z3::expr& right, op _operator) {
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
    return nullptr;
}

std::vector<std::shared_ptr<trace>> symEngine::find_race_condition() {
    std::vector<std::shared_ptr<trace>> traces;

    for (const auto &e : ccfg->edges) {
        if (e.type == conflict) {
            z3::context c;
            std::vector<z3::expr> vec;
            std::stack<std::string> inputs;
            std::stack<edge> edges;
            std::set<std::shared_ptr<basicblock>> addedBlocks;
            auto define = e.neighbours[0];
            auto use = e.neighbours[1];
            //t->goal = std::vector<std::shared_ptr<basicblock>>{define, use};

            edges.push(edge(flow, define->parents[0].lock(), define));
            for (const auto &expr : reachParent(&c, ccfg->startNode, define->parents[0].lock(), &inputs, &edges, &addedBlocks))
                vec.push_back(expr);
            edges.push(edge(flow, use->parents[0].lock(), use));
            for (const auto &expr : reachParent(&c, ccfg->startNode, use->parents[0].lock(), &inputs, &edges, &addedBlocks))
                vec.push_back(expr);


            z3::solver s(c);

            for (const auto &expr : vec) s.add(expr);
            if (s.check() == z3::sat) {
                auto model = s.get_model();
                std::cout << "sat\n" << model << std::endl;
                std::vector<std::string> inputsVec;
                while (!inputs.empty()) {
                    inputsVec.push_back(model.eval(c.int_const(inputs.top().c_str())).to_string());
                    inputs.pop();
                }

                std::vector<edge> edgesVec;
                while (!edges.empty()) {
                    //if (edges.top() != edgesVec[0] && edges.top() != edgesVec[1])
                        edgesVec.push_back(edges.top());
                    edges.pop();
                }
                traces.push_back(std::make_shared<trace>(
                        trace(edgesVec, inputsVec, std::vector<std::shared_ptr<basicblock>>{define, use})));
                std::cout << "\n\n\n" << std::endl;

            } else {
                std::cout << "unsat\n";
            }
        }
    }
    return traces;
}

std::vector<z3::expr> symEngine::reachParent(z3::context *c, std::shared_ptr<basicblock> goal, std::shared_ptr<basicblock> current, std::stack<std::string> *inputsStack, std::stack<edge> *edgesStack, std::set<std::shared_ptr<basicblock>> *addedBlocks) {
    using namespace z3;
    std::vector<z3::expr> vec;
    std::vector<std::string> inputs;
    std::vector<edge> edges;
    int currentRead = 0;
    while (current) {
        if (!addedBlocks->insert(current).second) break;
        auto node = current;
        for (const auto &stmt : node->statements) {
            switch (stmt->getNodeType()) {
                case Assign: {
                    auto assNode = dynamic_cast<assignNode*>(stmt.get());
                    switch (symboltable.find(assNode->getOriginalName())->second->getType()) {
                        case intType: {
                            z3::expr x = c->int_const(assNode->getName().c_str());
                            int prevVal = currentRead;
                            z3::expr res = get_expr(c, assNode->getExpr(), &currentRead, &inputs, &edges);
                            if (prevVal < currentRead) {
                                //std::cout << res.to_string() << std::endl;
                                vec.push_back(x == c->int_const(("-" + std::to_string(currentRead-1) + "readVal").c_str()));
                            } else {
                                vec.push_back(x == res);
                            }
                            break;
                        } case boolType: {
                            z3::expr x = c->bool_const(assNode->getName().c_str());
                            vec.push_back(x == get_expr(c, assNode->getExpr(), &currentRead, &inputs, &edges));
                            break;
                        } case arrayIntType: {
                            auto arrLit = dynamic_cast<arrayLiteralNode*>(assNode->getExpr());
                            sort I = c->int_sort();
                            sort A = c->array_sort(I, I);
                            auto cStr = assNode->getName().c_str();
                            expr a1 = c->constant(cStr, A);
                            vec.push_back(a1);
                            const auto &lit = arrLit->getArrLit();
                            for (int i = 0; i < arrLit->getArrLit().size(); ++i) {
                                vec.push_back(z3::store(a1, i, get_expr(c, lit[i].get(), &currentRead, &inputs, &edges)));
                            }
                            break;
                        }
                        case arrayBoolType: {
                            auto arrLit = dynamic_cast<arrayLiteralNode*>(assNode->getExpr());
                            sort B = c->bool_sort();
                            sort BB = c->array_sort(B, B);
                            auto cStr = assNode->getName().c_str();
                            expr b1 = c->constant(cStr, BB);
                            vec.push_back(b1);
                            const auto &lit = arrLit->getArrLit();
                            for (int i = 0; i < arrLit->getArrLit().size(); ++i) {
                                vec.push_back(z3::store(b1, i, get_expr(c, lit[i].get(), &currentRead, &inputs, &edges)));
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
                    vec.push_back(!get_expr(c, wNode->getCondition(), &currentRead, &inputs, &edges));
                    break;
                }
                case If: {
                    auto ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                    z3::expr expr = get_expr(c, ifNode->getCondition(), &currentRead, &inputs, &edges);
                    if (!edgesStack->empty() && edgesStack->top().neighbours[1] == node->nexts[1]) // if recently added edge's second neighbour is current node's second next, then we took the false branch
                        vec.push_back(!expr);
                    else
                        vec.push_back(expr);
                    break;
                }
                case Event: {
                    auto evNode = dynamic_cast<eventNode*>(stmt.get());
                    vec.push_back(get_expr(c, evNode->getCondition(), &currentRead, &inputs, &edges));
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
                            vec.push_back(c->int_const(phi->getName().c_str()) == c->int_const(phi->get_variables()->at(1).c_str()));
                            break;
                        case boolType:
                            vec.push_back(c->bool_const(phi->getName().c_str()) == c->bool_const(phi->get_variables()->at(1).c_str()));
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
                    auto pi = dynamic_cast<piNode*>(stmt.get());
                    auto res = symboltable.find(pi->getVar());
                    switch (res->second->getType()) {
                        case intType: {
                            auto vars = *pi->get_variables();
                            z3::expr x = (c->int_const(pi->getName().c_str()) ==
                                          c->int_const(vars[0].c_str()));
                            for (auto i = 1; i < vars.size(); ++i) {
                                x = x || (c->int_const(pi->getName().c_str()) ==
                                          c->int_const(vars[i].c_str()));
                            }
                            vec.push_back(x);
                            break;
                        } case boolType: {
                            vec.push_back(c->bool_const(pi->getName().c_str()) ==
                                          c->bool_const(pi->get_variables()->at(0).c_str()));
                            break;
                        } case arrayIntType: {
                            break;
                        } case arrayBoolType: {
                            break;
                        }
                        default:break;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        for (auto i = inputs.rbegin(); i != inputs.rend(); ++i) {
            inputsStack->push(*i);
        }

        if (current == goal) break;
        edgesStack->push(edge(flow, current->parents[0].lock(), current));
        current = current->parents[0].lock();
    }
    return vec;
}



std::pair<std::shared_ptr<basicblock>, bool> symEngine::run_trace(std::shared_ptr<trace> t) {

}

z3::expr symEngine::get_expr (z3::context *c, expressionNode *node, int *currentRead, std::vector<std::string> *inputs, std::vector<edge> *edges) {
    switch (node->getNodeType()) {
        case Read: {
            z3::expr x = c->int_const(("-" + std::to_string(*currentRead) + "readVal").c_str());
            inputs->push_back("-" + std::to_string(*currentRead) + "readVal");
            *currentRead += 1;
            return (x <= c->int_val(INT16_MAX) && x >= c->int_val(INT16_MIN));
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
            z3::expr left = get_expr(c, binOp->getLeft(), currentRead, inputs, edges);
            z3::expr right = get_expr(c, binOp->getRight(), currentRead, inputs, edges);
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
            z3::expr val = get_expr(c, unOp->getExpr(), currentRead, inputs, edges);
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
                            vec.push_back(x == get_expr(&c, assNode->getExpr(), &currentRead, nullptr, nullptr));
                            break;
                        } case boolType: {
                            z3::expr x = c.bool_const(assNode->getName().c_str());
                            vec.push_back(x == get_expr(&c, assNode->getExpr(), &currentRead, nullptr, nullptr));
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
                                vec.push_back(z3::store(a1, i, get_expr(&c, lit[i].get(), &currentRead, nullptr, nullptr)));
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
                                vec.push_back(z3::store(b1, i, get_expr(&c, lit[i].get(), &currentRead, nullptr, nullptr)));
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
                    vec.push_back(!get_expr(&c, wNode->getCondition(), &currentRead, nullptr, nullptr));
                    break;
                }
                case If: {
                    auto ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                    vec.push_back(get_expr(&c, ifNode->getCondition(), &currentRead, nullptr, nullptr));
                    break;
                }
                case Event: {
                    auto evNode = dynamic_cast<eventNode*>(stmt.get());
                    vec.push_back(get_expr(&c, evNode->getCondition(), &currentRead, nullptr, nullptr));
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