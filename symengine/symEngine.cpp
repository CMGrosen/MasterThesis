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
    z3::solver s(c);

    for (const auto &read : ccfg->reads) {
        z3::expr name = c.int_const(read.c_str());
        z3::expr r = name >= INT16_MIN && name <= INT16_MAX;
        std::cout << r.to_string() << "\n";
        s.add(r);
    }

    z3::expr encoded = encoded_pis(&c, ccfg->pis_and_depth, {});

    std::cout << "\n\n\n\n\nencoded:\n" << encoded.to_string() << std::endl;

    auto t = get_run(&c, nullptr, ccfg->startNode, ccfg->exitNode);

    //std::cout << "\n" << t.to_string() << "\n\n" << std::endl;
    s.add(encoded && t);

    z3::goal g(c);
    g.add(get_run(&c, ccfg->exitNode->parents[0].lock().get(), ccfg->exitNode, ccfg->exitNode));

    auto tt = z3::tactic(c, "simplify")(g)[0];
    for (auto i = 0; i < tt.size(); ++i) s.add(tt[i]);

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
                                z3::expr final = z3::ite(condition, truebranch, c->bool_val(false));
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
                                return z3::ite(condition, truebranch, c->bool_val(false));
                            }
                        }
                        for (const auto &nxt : end->nexts) {
                            truebranch = truebranch && get_run(c, end.get(), nxt, ccfg->exitNode);
                        }
                    }
                    event_encountered = true;
                    z3::expr final = z3::ite(condition, truebranch, c->bool_val(false));
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
                    break;
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

bool contains(const std::vector<std::string> &vars, const std::string &var) {
    for (const auto &v : vars) if (v == var) return true;
    return false;
}

bool variation (const std::string &variable, const std::string &other) {
    for (int i = 0; i < variable.length(); ++i) {
        if (variable[i] != other[i]) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> symEngine::includable_vars(std::shared_ptr<statementNode> stmt, std::unordered_map<std::string, std::vector<std::string>> constraints) {
    std::vector<std::string> possiblevars;
    if (auto pin = dynamic_cast<piNode*>(stmt.get())) {
        std::string name = pin->getVar();
        for (const auto &var : *pin->get_variables()) {
            if (constraints.find(name) == constraints.end()) {
                possiblevars.push_back(var);
            } else {
                auto blk = ccfg->defs.find(var)->second;
                auto vec = constraints.find(name)->second;
                if (vec.back() != var
                    && contains(vec, var)) {
                    //This variable was previously defined and has since been replaced. Cannot use it
                } else {
                    bool toinclude = true;
                    for (const auto &o : vec) {
                        std::shared_ptr<basicblock> otherdefsite = ccfg->defs.find(o)->second;
                        if (blk->concurrentBlock == otherdefsite->concurrentBlock && blk->depth < otherdefsite->depth) {
                            //Variable is defined in a thread that defines a new version of said variable.
                            //Since the new variable has already been used, "var" cannot possibly be used again
                            toinclude = false;
                            break;
                        }
                    }
                    if (toinclude) possiblevars.push_back(var);
                }
            }
        }
        std::vector<std::string> varsToRemove;
        for (const auto &v : possiblevars) {
            auto blk = ccfg->defs.find(v)->second;
            for (const auto &stmt : blk->statements) {
                if (stmt->getNodeType() == Phi) {
                    auto phin = dynamic_cast<phiNode*>(stmt.get());
                    if (phin->getName() == v) {
                        bool possiblevarsContainsOptions = false;
                        for (const auto &vv : possiblevars) {
                            if (contains(*phin->get_variables(), vv)) {
                                possiblevarsContainsOptions = true;
                                break;
                            }
                        }
                        if (!possiblevarsContainsOptions) varsToRemove.push_back(v);
                    }
                }
            }
        }
        std::vector<std::string> final;

        for (const auto &v : possiblevars) {
            bool remove = false;
            for (const auto &vv : varsToRemove) {
                if (v == vv) {
                    remove = true;
                    break;
                }
            }
            if (!remove) final.push_back(v);
        }
        return final;
    } else if (auto phi = dynamic_cast<phiNode*>(stmt.get())) {
        std::string name = phi->getOriginalName();
        std::vector<std::string> vars = *phi->get_variables();
        if (constraints.find(name) == constraints.end()) {
            return *phi->get_variables();
        } else {
            std::vector<std::string> consts = constraints.find(name)->second;
            if (contains(vars, consts.back())) {
                return {consts.back()};
            } else {
                std::unordered_set<std::string> impossible_vars;
                for (auto it = consts.rbegin(); it != consts.rend(); ++it) {
                    if (contains(vars, *it)) {
                        impossible_vars.insert(*it);
                    }
                }
                std::vector<std::string> final;
                for (const auto &v : vars) {
                    if (impossible_vars.find(v) == impossible_vars.end()) {
                        final.push_back(v);
                    }
                }
                return final;
            }
            return {};
        }
    } else {
        return {};
    }
}

z3::expr symEngine::encoded_pis(z3::context *c, const std::vector<std::pair<std::shared_ptr<basicblock>, int32_t>> &remaining, const std::unordered_map<std::string, std::vector<std::string>> &constraints) {
    int32_t current_depth = remaining.front().second;
    auto pin = dynamic_cast<piNode*>(remaining.front().first->statements.front().get());

    if (pin->getName() == "-T_a_2") {
        std::cout << "test";
    }
    if (pin->getName() == "-T_a_3") {
        std::cout << "test";
    }
    if (pin->getName() == "-T_a_1") {
        std::cout << "test";
    }
    if (pin->getName() == "-T_a_0") {
        std::cout << "test";
    }

    if (remaining.size() == 1) {
        std::vector<std::string> possiblevars = includable_vars(remaining.front().first->statements.front(), constraints);
        std::shared_ptr<basicblock> endconc;
        for (const auto &blk : ccfg->nodes) {
            if (blk->type == Coend && dynamic_cast<endConcNode*>(blk->statements.back().get())->getConcNode().get() == remaining.front().first->concurrentBlock.first) {
                endconc = blk;
                break;
            }
        }
        if (possiblevars.empty()) return c->bool_val(false);
        else {
            std::vector<std::string> posvarsforendnode;
            std::stack<z3::expr> end;
            std::vector<z3::expr> finals;
            for (const auto &stmt : endconc->statements) {
                if (auto phi = dynamic_cast<phiNode*>(stmt.get())) {
                    for (const auto &str : includable_vars(stmt, constraints)) {
                        posvarsforendnode.push_back(str);
                    }
                    for (const auto &str : posvarsforendnode) {
                        end.push(phi->getType() == intType
                                 ? c->int_const(phi->getName().c_str()) == c->int_const(str.c_str())
                                 : c->bool_const(phi->getName().c_str()) == c->bool_const(str.c_str()));
                    }

                    if (!end.empty()) {
                        z3::expr final = end.top();
                        end.pop();
                        while (!end.empty()) {
                            final = final || end.top();
                            end.pop();
                        }
                        finals.push_back(final);
                    }
                    posvarsforendnode.clear();
                }
            }

            for (const auto &f : finals) {
                end.push(f);
            }
            z3::expr inter = end.top();
            end.pop();
            while(!end.empty()) {
                inter = inter && end.top();
                end.pop();
            }

            z3::expr final = pin->getType() == intType
                ? c->int_const(pin->getName().c_str()) == c->int_const(possiblevars[0].c_str())
                : c->bool_const(pin->getName().c_str()) == c->bool_const(possiblevars[0].c_str());



            for (int i = 1; i < possiblevars.size(); ++i) {
                final = final
                        || pin->getType() == intType
                        ? c->int_const(pin->getName().c_str()) == c->int_const(possiblevars[i].c_str())
                        : c->bool_const(pin->getName().c_str()) == c->bool_const(possiblevars[i].c_str());
            }

            final = final && inter;
            return final;
        }
    } else {//same depth. order probably matters
        int i = 0;
        /*for (i = 0; i < remaining.size()-1; ++i) {
            if (remaining[i].second != remaining[i+1].second) {
                break;
            }
        }*/
//        if (i > 0){

//        } else {
        std::vector<std::string> possiblevars = includable_vars(remaining.front().first->statements.front(), constraints);
        std::vector<std::pair<std::shared_ptr<basicblock>, int32_t>> newremains;
        for (i = 1; i < remaining.size(); ++i) newremains.push_back(remaining[i]);
        if (possiblevars.empty()) return c->bool_val(false);
        std::unordered_map<std::string, std::vector<std::string>> newconsts = constraints;
        auto res = newconsts.insert({pin->getVar(), {possiblevars[0]}});
        if (!res.second) res.first->second.push_back(possiblevars[0]);

        z3::expr final = (pin->getType() == intType
                         ? c->int_const(pin->getName().c_str()) == c->int_const(possiblevars[0].c_str())
                         : c->bool_const(pin->getName().c_str()) == c->bool_const(possiblevars[0].c_str()));
        z3::expr inter = encoded_pis(c, newremains, newconsts);
        final = final && inter;
        for (int i = 1; i < possiblevars.size(); ++i) {
            newconsts = constraints;
            res = newconsts.insert({pin->getVar(), {possiblevars[i]}});
            if (!res.second) res.first->second.push_back(possiblevars[i]);
            inter = (pin->getType() == intType
                              ? c->int_const(pin->getName().c_str()) == c->int_const(possiblevars[i].c_str())
                              : c->bool_const(pin->getName().c_str()) == c->bool_const(possiblevars[i].c_str()));
            inter = inter && encoded_pis(c, newremains, newconsts);
            final = final || (inter);
            //std::cout << "final: " << final.to_string() << std::endl;
        }
        return final;

    }
}
