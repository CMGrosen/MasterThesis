//
// Created by hu on 24/02/2020.
//

#include "symEngine.hpp"
#include "VariableValue.hpp"
#include <limits>
#include <string>

symEngine::symEngine(const std::shared_ptr<CSSA_CFG>& ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) :
    boolname_counter{ccfg->boolname_counter}, c{}, s{z3::solver(c)},
    event_encountered{false}, constraintset{}, ccfg{ccfg->ccfg}, symboltable(std::move(table)) {}

symEngine::symEngine(const symEngine &a) :
    boolname_counter{a.boolname_counter}, c{}, s{z3::solver(c)},
    event_encountered{false}, constraintset{}, ccfg{a.ccfg}, symboltable{a.symboltable} {}

symEngine &symEngine::operator=(const symEngine &a) {
    boolname_counter = a.boolname_counter;
    s = z3::solver(c);
    event_encountered = false;
    constraintset = a.constraintset;
    ccfg = a.ccfg;
    symboltable = a.symboltable;
    return *this;
}

symEngine::symEngine(symEngine &&a) noexcept :
    boolname_counter{a.boolname_counter}, c{}, s{z3::solver(c)},
    event_encountered{false}, constraintset{std::move(a.constraintset)}, ccfg{std::move(a.ccfg)}, symboltable{std::move(a.symboltable)} {}

symEngine &symEngine::operator=(symEngine &&a) noexcept {
    boolname_counter = a.boolname_counter;
    s = z3::solver(c);
    event_encountered = false;
    constraintset = std::move(a.constraintset);
    ccfg = std::move(a.ccfg);
    symboltable = std::move(a.symboltable);
    return *this;
}


void symEngine::add_reads() {
    size_t i = 0;
    while(i++<ccfg->readcount) {
        z3::expr name = c.int_const(("-readVal_" + std::to_string(i)).c_str());
        z3::expr r = name >= INT16_MIN && name <= INT16_MAX;
        std::cout << r.to_string() << "\n";
        constraintset.push_back(r);
    }
}

static z3::expr encode_boolname (z3::context *c, const std::string& name, bool val, const std::string& run) {
    return c->bool_const((name + run).c_str()) == c->bool_val(val);
}

static z3::expr conjunction(const z3::expr_vector& vec) {
    z3::expr final = *vec.begin();
    for (auto it = ++vec.begin(); it != vec.end(); ++it) final = final && *it;
    return final;
}

static z3::expr conjunction(z3::context *c, const std::vector<std::string> &vec, bool val, const std::string& run) {
    z3::expr final = encode_boolname(c, *vec.begin(), val, run);
    for (auto it = ++vec.begin(); it != vec.end(); ++it)
        final = final && encode_boolname(c, *it, val, run);
    return final;
}

static z3::expr disjunction(z3::context *c, const std::vector<z3::expr>& vec) {
    if (vec.empty()) return c->bool_val(true);
    z3::expr final = *vec.begin();
    for (auto it = ++vec.begin(); it != vec.end(); ++it) final = final || *it;
    return final;
}

static z3::expr encodepi(z3::context *c, Type t, const std::string& boolname, const std::string& name, std::vector<z3::expr> *constraintset) {
    z3::expr run1use = encode_boolname(c, boolname, true, _run1);
    z3::expr run2use = encode_boolname(c, boolname, true, _run2);
    constraintset->emplace_back(z3::ite
      ( !run1use
      , t == intType //If both are assigned, make this constraint
            ? c->int_const((name + _run1).c_str()) == c->int_const((name + _run2).c_str())
            : c->bool_const((name + _run1).c_str()) == c->bool_const((name + _run2).c_str())
      , c->bool_val(true)
      ));
    return z3::ite
      ( run1use
      , t == intType //If both are assigned, make this constraint
          ? c->int_const((name + _run1).c_str()) != c->int_const((name + _run2).c_str())
          : c->bool_const((name + _run1).c_str()) != c->bool_const((name + _run2).c_str())
      , c->bool_val(false)
      /*t == intType //As tracking booleans are false, we just assign the same value
        ? c->int_const((name + _run1).c_str()) == c->int_const((name + _run2).c_str())
        : c->bool_const((name + _run1).c_str()) == c->bool_const((name + _run2).c_str())
        /* z3::ite //If both are false, force Z3 to pick another option
        ( !run1use && !run2use
        , c->bool_val(false)
        , t == intType //Usage in one run but not the other. Force them to have same value
          ? c->int_const((name + _run1).c_str()) == c->int_const((name + _run2).c_str())
          : c->bool_const((name + _run1).c_str()) == c->bool_const((name + _run2).c_str())
        )*/
      );
}


bool symEngine::execute() {
    add_reads();
    std::cout << constraintset.size() << std::endl;
    //z3::expr encoded = encoded_pis(&c, ccfg->pis_and_depth, {});

    //std::cout << "\n\n\n\n\nencoded:\n" << encoded.to_string() << std::endl;

    auto run1 = get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run1);
    event_encountered = false;
    auto run2 = get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run2);

    //std::cout << "\n" << t.to_string() << "\n\n" << std::endl;

    z3::expr inter = conjunction(run1);
    std::cout << inter.to_string() << std::endl;
    z3::expr exp = inter && conjunction(run2);
    //std::cout << exp.to_string() << std::endl;
    constraintset.push_back(exp);
    std::cout << constraintset.size() << std::endl;
/*
    z3::goal g(c);
    g.add(get_run(&c, ccfg->exitNode->parents[0].lock().get(), ccfg->exitNode, ccfg->exitNode));

    auto tt = z3::tactic(c, "simplify")(g)[0];
    for (size_t i = 0; i < tt.size(); ++i) s.add(tt[i]);

    */


    std::vector<z3::expr> vec;
    for (const auto &p : ccfg->pis_and_depth) {
        //Don't want to add these pi-functions if used in an event,
        // as race-conditions cannot occur here
        if (p.first->statements.back()->getNodeType() != Event) {
            for (const auto &stmt : p.first->statements) {
                if (auto pi = dynamic_cast<piNode*>(stmt.get())) {
                    vec.push_back(encodepi(&c, pi->getType(), pi->get_boolname(), pi->getName(), &constraintset));
                }
            }
        }
    }
    z3::expr dis = disjunction(&c, vec);
    std::cout << "debug:\n" << dis.to_string() << std::endl;
    std::cout << constraintset.size() << std::endl;
    constraintset.push_back(dis);


    std::cout << constraintset.size() << std::endl;

    for (int i = 1; i < boolname_counter; ++i) {
        std::string name = "-b_" + std::to_string(i);
        constraintset.emplace_back(c.bool_const((name + _run1).c_str()) == c.bool_const((name + _run2).c_str()));
    }
    for (const auto &expr : constraintset) s.add(expr);

    if (s.check() == z3::sat) {
        auto model = s.get_model();
        std::cout << "sat\n";
        //std::cout << model << "\n" << std::endl;
        return true;
    } else {
        std::cout << "unsat\nProbably no race-conditions" << std::endl;
        std::stack<z3::expr> early_exits;
        std::vector<std::pair<std::string, std::string>> names;
        s.reset();
        add_reads();
        s.add(conjunction(get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run1)));
        for (const auto &blk : ccfg->fiNodes) {
            for (const auto &stmt : blk->statements) {
                if (auto phi = dynamic_cast<phiNode*>(stmt.get())) {
                    if (phi->getOriginalName().find("early") != std::string::npos) {
                        //std::cout << phi->getName() << " = " << s.re
                        early_exits.push(c.bool_const(phi->getName().c_str()) == c.int_val(true));
                        names.emplace_back(phi->getName(), phi->getOriginalName());
                    }
                }
            }
        }

        if (early_exits.empty()) {
            std::cout << "no early exit variables. Program has been fully explored" << std::endl;
        } else {
            z3::expr final = early_exits.top();
            early_exits.pop();
            while (!early_exits.empty()) {
                final = final || early_exits.top();
                early_exits.pop();
            }
            s.add(final);

            if (s.check() == z3::sat) {
                z3::model m = s.get_model();
                for (const auto &pair : names) {
                    std::cout << pair.second << " = " << m.eval(c.bool_const(pair.first.c_str())).to_string() << std::endl;
                }
                return true;
            } else {
                std::cout << "not satisfiable even when removing additional constraints\n";
            }
        }

        std::cout <<  s.unsat_core() << std::endl;
    }

    return false;

}

z3::expr_vector symEngine::get_run(const std::shared_ptr<basicblock>& previous, const std::shared_ptr<basicblock> &start, const std::shared_ptr<basicblock> &end, const std::string &run) {
    auto node = start;
    z3::expr_vector constraints(c);

    for (const auto &stmt : node->statements) {
        switch (stmt->getNodeType()) {
            case Assign: {
                auto assnode = dynamic_cast<assignNode *>(stmt.get());
                auto name = assnode->getExpr()->getType() == intType
                            ? c.int_const((assnode->getName() + run).c_str())
                            : c.bool_const((assnode->getName() + run).c_str());

                constraints.push_back(
                        (name == evaluate_expression(&c, assnode->getExpr(), run, &constraints)) &&
                        (encode_boolname(&c, stmt->get_boolname(), true, run)));

                break;
            }
            case Concurrent: {
                auto endConc = get_end_of_concurrent_node(node);
                z3::expr_vector vec(c);
                //std::stack<z3::expr> expressions;
                int i = 0;
                bool changed_event = false;
                for (const auto &nxt : node->nexts) {
                    if (event_encountered) {
                        event_encountered = false;
                        changed_event = true;
                    }
                    vec.push_back(conjunction(get_run(node, nxt, endConc->parents[i++].lock(), run)));
                }
                constraints.push_back(conjunction(vec) && encode_boolname(&c, stmt->get_boolname(), true, run));

                if (changed_event) {
                    event_encountered = true;
                    return constraints;
                }
                node = endConc->parents[0].lock();
                break;
            }
            case If: {
                std::shared_ptr<basicblock> firstCommonChild = find_common_child(node);
                bool changed_event = false;
                auto *ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                if (event_encountered) {
                    event_encountered = false;
                    changed_event = true;
                }
                z3::expr truebranch = conjunction(get_run(node, node->nexts[0], firstCommonChild, run))
                        && conjunction(&c, ifNode->boolnamesForFalseBranch, false, run);
                if (event_encountered) {
                    event_encountered = false;
                    changed_event = true;
                }
                z3::expr falsebranch = conjunction(get_run(node, node->nexts[1], firstCommonChild, run))
                        && conjunction(&c, ifNode->boolnamesForTrueBranch, false, run);
                if (event_encountered) {
                    changed_event = true;
                }
                z3::expr final = (z3::ite(
                        evaluate_expression(&c, ifNode->getCondition(), run, &constraints),
                        truebranch, falsebranch)) && encode_boolname(&c, stmt->get_boolname(), true, run);

                constraints.push_back(final);
                if (changed_event) return constraints;

                while (firstCommonChild && firstCommonChild->type == Condition) {
                    if (firstCommonChild->statements.back()->getNodeType() == If) {
                        firstCommonChild = find_common_child(firstCommonChild);
                    } else { //Event
                        return constraints;
                    }
                }
                if (firstCommonChild && !firstCommonChild->nexts.empty() && firstCommonChild != end) {
                    constraints.push_back(conjunction(get_run(firstCommonChild, firstCommonChild->nexts[0], end, run)));
                }
                return constraints;
            }
            case Event: {
                z3::expr condition = evaluate_expression(&c, dynamic_cast<eventNode*>(stmt.get())->getCondition(), run, &constraints);
                bool changed_event = false;
                if (event_encountered) {
                    changed_event = true;
                    event_encountered = false;
                }
                z3::expr truebranch = conjunction(get_run( node, node->nexts[0], end, run));

                if (changed_event) event_encountered = true;
                if (end != ccfg->exitNode) {
                    auto res = ccfg->concurrent_events.find(node.get());
                    if (res != ccfg->concurrent_events.end()) {
                        if (!event_encountered) {
                            z3::expr_vector finalCond(c);
                            for (const auto &event : res->second) {
                                finalCond.push_back(c.bool_const((event->statements.back()->get_boolname() + run).c_str()));
                            }
                            constraints.push_back(z3::ite(condition, truebranch, c.bool_val(false)) && encode_boolname(&c, stmt->get_boolname(), true, run));

                            z3::expr finalConstraint =
                                z3::ite(conjunction(finalCond)
                                       , conjunction(constraints) && conjunction(get_run(end, end->nexts[0], ccfg->exitNode, run))
                                       , c.bool_val(true)
                                       );
                            event_encountered = true;
                            z3::expr_vector v(c);
                            v.push_back(finalConstraint);
                            return v;
                        } else {
                            z3::expr_vector v(c);
                            v.push_back(z3::ite(condition, truebranch, c.bool_val(false)) && encode_boolname(&c, stmt->get_boolname(), true, run));
                            return v;
                        }
                    }
                    for (const auto &nxt : end->nexts) {
                        truebranch = truebranch && get_run(end, nxt, ccfg->exitNode, run);
                    }
                }
                event_encountered = true;
                constraints.push_back(z3::ite(condition, truebranch, c.bool_val(false)) && encode_boolname(&c, stmt->get_boolname(), true, run));
                return constraints;
            }
            case EndConcurrent:
            case AssignArrField:
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
                constraints.push_back(encode_boolname(&c, stmt->get_boolname(), true, run));
                break;
            case Phi: {
                auto phi = dynamic_cast<phiNode *>(stmt.get());
                auto parents = node->parents;

                switch (phi->getType()) {
                    case intType: {
                        z3::expr name = c.int_const((phi->getName() + run).c_str());
                        for (size_t i = 0; i < parents.size(); ++i) {
                            if (previous == parents[i].lock()) {
                                constraints.push_back((name == c.int_const((phi->get_variables()->at(i).first + run).c_str())) && encode_boolname(&c, stmt->get_boolname(), true, run));
                                break;
                            }
                        }
                        break;
                    }
                    case boolType: {
                        z3::expr name = c.bool_const((phi->getName() + run).c_str());
                        for (size_t i = 0; i < parents.size(); ++i) {
                            if (previous == parents[i].lock()) {
                                constraints.push_back((name == c.bool_const((phi->get_variables()->at(i).first + run).c_str())) && encode_boolname(&c, stmt->get_boolname(), true, run));
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
                std::vector<z3::expr> expressions;
                switch (pi->getType()) {
                    case intType: {
                        z3::expr name = c.int_const((pi->getName() + run).c_str());
                        for (const auto &conflict : *vars) {
                            expressions.push_back(z3::ite
                              ( c.bool_const((conflict.second + run).c_str())
                              , name == c.int_const((conflict.first + run).c_str())
                              , name == c.int_val(0) && name == c.int_val(1) //unsatisfiable
                              ));
                        }
                        break;
                    }
                    case boolType: {
                        z3::expr name = c.bool_const((pi->getName() + run).c_str());
                        for (const auto &conflict : *vars) {
                            expressions.push_back(z3::ite
                              ( c.bool_const((conflict.second + run).c_str())
                              , name == c.bool_const((conflict.first + run).c_str())
                              , name == c.bool_val(true) && name == c.bool_val(false) //unsatisfiable. Won't ever pick this option
                              ));
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
                constraints.push_back(disjunction(&c, expressions) && encode_boolname(&c, stmt->get_boolname(), true, run));
                break;
            }
        }
    }
    if (node == end || node->nexts.empty() || event_encountered) {
        return constraints;
    } else {
        constraints.push_back(conjunction(get_run(node, node->nexts[0], end, run)));
        return constraints;
    }
}

std::shared_ptr<basicblock> symEngine::find_common_child(const std::shared_ptr<basicblock>& parent) {
    for (const auto &blk : ccfg->fiNodes) {
        if (auto fi = dynamic_cast<fiNode*>(blk->statements.back().get())) {
            if (fi->get_parents()->find(parent) != fi->get_parents()->end()) {
                return blk;
            }
        }
    }
    return nullptr;
}

z3::expr symEngine::evaluate_expression(z3::context *c, const expressionNode *expr, const std::string &run, z3::expr_vector *constraints) {
    switch (expr->getNodeType()) {
        case Read: {
            //don't use run here. That way, reads will be identical across runs
            return c->int_const(dynamic_cast<const readNode*>(expr)->getName().c_str()); }
        case Literal: {
            auto lit = dynamic_cast<const literalNode*>(expr);
            return lit->getType() == intType
                ? c->int_val(std::stoi(lit->value))
                : c->bool_val(lit->value == "true")
                ;
        }
        case ArrayAccess:
            break;
        case ArrayLiteral:
            break;
        case Variable: {
            return expr->getType() == intType
                ? c->int_const((dynamic_cast<const variableNode*>(expr)->name + run).c_str())
                : c->bool_const((dynamic_cast<const variableNode*>(expr)->name + run).c_str())
                ;
        }
        case BinaryExpression: {
            auto binexpr = dynamic_cast<const binaryExpressionNode*>(expr);
            return evaluate_operator
                    ( c
                    , evaluate_expression(c, binexpr->getLeft(), run, constraints)
                    , evaluate_expression(c, binexpr->getRight(), run, constraints)
                    , binexpr->getOperator()
                    , constraints
                    );
        }
        case UnaryExpression: {
            auto unexpr = dynamic_cast<const unaryExpressionNode*>(expr);
            z3::expr exp = evaluate_expression(c, unexpr->getExpr(), run, constraints);
            return evaluate_operator(c, exp, exp, unexpr->getOperator(), constraints);
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
        case Assert:
            break;
    }
    assert(false);
}

z3::expr symEngine::evaluate_operator(z3::context *c, const z3::expr& left, const z3::expr& right, op _operator, z3::expr_vector *constraints) {
    switch (_operator) {
        case PLUS:
            return left + right;
        case MINUS:
            return left - right;
        case MULT:
            return left * right;
        case DIV: constraints->push_back(right != c->int_val(0));
            return (left / right); //don't want right-hand side to be 0, even if this is possible (for now)
        case MOD: constraints->push_back(right != c->int_val(0));
            return (left % right); //don't want right-hand side to be 0, even if this is possible (for now)
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

std::shared_ptr<basicblock> symEngine::get_end_of_concurrent_node(const std::shared_ptr<basicblock>& node) {
    for (const auto &blk : ccfg->endconcNodes) {
        if (dynamic_cast<endConcNode*>(blk->statements.back().get())->getConcNode() == node) {
            return blk;
        }
    }
    assert(false);
    return nullptr;
}

bool contains(const std::vector<std::string> &vars, const std::string &var) {
    for (const auto &v : vars) if (v == var) return true;
    return false;
}

std::vector<std::string> symEngine::includable_vars(const std::shared_ptr<statementNode> &stmt, std::unordered_map<std::string, std::vector<std::string>> constraints) {
    std::vector<std::string> possiblevars;
    if (auto pin = dynamic_cast<piNode*>(stmt.get())) {
        std::string name = pin->getVar();
        for (const auto &var : *pin->get_variables()) {
            if (constraints.find(name) == constraints.end()) {
                possiblevars.push_back(var.first);
            } else {
                auto blk = ccfg->defs.find(var.first)->second;
                auto vec = constraints.find(name)->second;
                if (vec.back() != var.first
                    && contains(vec, var.first)) {
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
                    if (toinclude) possiblevars.push_back(var.first);
                }
            }
        }
        std::vector<std::string> varsToRemove;
        for (const auto &v : possiblevars) {
            auto blk = ccfg->defs.find(v)->second;
            for (const auto &s : blk->statements) {
                if (s->getNodeType() == Phi) {
                    auto phin = dynamic_cast<phiNode*>(s.get());
                    if (phin->getName() == v) {
                        bool possiblevarsContainsOptions = false;
                        for (const auto &vv : possiblevars) {
                            std::vector<std::string> tmpvec;
                            tmpvec.reserve(phin->get_variables()->size());
                            for (const auto &vvv : *phin->get_variables()) {
                                tmpvec.push_back(vvv.first);
                            }
                            if (contains(tmpvec, vv)) {
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
        std::vector<std::string> vars;
        vars.reserve(phi->get_variables()->size());
        for (const auto &vvv : *phi->get_variables()) {
            vars.push_back(vvv.first);
        }
        if (constraints.find(name) == constraints.end()) {
            return vars;
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
        }
    } else {
        return {};
    }
}

z3::expr symEngine::encoded_pis(const std::vector<std::pair<std::shared_ptr<basicblock>, int32_t>> &remaining, const std::unordered_map<std::string, std::vector<std::string>> &constraints) {
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
        if (possiblevars.empty()) return c.bool_val(false);
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
                                 ? c.int_const(phi->getName().c_str()) == c.int_const(str.c_str())
                                 : c.bool_const(phi->getName().c_str()) == c.bool_const(str.c_str()));
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
                ? c.int_const(pin->getName().c_str()) == c.int_const(possiblevars[0].c_str())
                : c.bool_const(pin->getName().c_str()) == c.bool_const(possiblevars[0].c_str());



            for (size_t i = 1; i < possiblevars.size(); ++i) {
                final = final
                        || pin->getType() == intType
                        ? c.int_const(pin->getName().c_str()) == c.int_const(possiblevars[i].c_str())
                        : c.bool_const(pin->getName().c_str()) == c.bool_const(possiblevars[i].c_str());
            }

            final = final && inter;
            return final;
        }
    } else {//same depth. order probably matters
        size_t i = 0;
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
        if (possiblevars.empty()) return c.bool_val(false);
        std::unordered_map<std::string, std::vector<std::string>> newconsts = constraints;
        auto res = newconsts.insert({pin->getVar(), {possiblevars[0]}});
        if (!res.second) res.first->second.push_back(possiblevars[0]);

        if (auto assn = dynamic_cast<assignNode*>(remaining.front().first->statements.back().get())) {
            res = newconsts.insert({assn->getOriginalName(), {assn->getName()}});
            if (!res.second) res.first->second.push_back(assn->getName());
        }

        z3::expr final = (pin->getType() == intType
                         ? c.int_const(pin->getName().c_str()) == c.int_const(possiblevars[0].c_str())
                         : c.bool_const(pin->getName().c_str()) == c.bool_const(possiblevars[0].c_str()));
        z3::expr inter = encoded_pis(newremains, newconsts);
        final = final && inter;
        for (i = 1; i < possiblevars.size(); ++i) {
            newconsts = constraints;
            res = newconsts.insert({pin->getVar(), {possiblevars[i]}});
            if (!res.second) res.first->second.push_back(possiblevars[i]);

            if (auto assn = dynamic_cast<assignNode*>(remaining.front().first->statements.back().get())) {
                res = newconsts.insert({assn->getOriginalName(), {assn->getName()}});
                if (!res.second) res.first->second.push_back(assn->getName());
            }

            inter = (pin->getType() == intType
                              ? c.int_const(pin->getName().c_str()) == c.int_const(possiblevars[i].c_str())
                              : c.bool_const(pin->getName().c_str()) == c.bool_const(possiblevars[i].c_str()));
            inter = inter && encoded_pis(newremains, newconsts);
            final = final || (inter);
            //std::cout << "final: " << final.to_string() << std::endl;
        }
        return final;

    }
}

std::pair<std::map<std::string, std::shared_ptr<VariableValue>>, std::map<std::string, bool>> symEngine::getModel() {
    std::map<std::string, std::shared_ptr<VariableValue>> values;
    std::map<std::string, bool> paths;
    z3::model m = s.get_model();
    //std::cout << m << std::endl;
    for (unsigned i = 0; i < m.size(); i++) {
        z3::func_decl v = m[i];
        // this problem contains only constants
        assert(v.arity() == 0);
        std::string value = m.get_const_interp(v).to_string();
        Type t;
        if (!(v.name().str().front() == '-' && v.name().str()[1] == 'b')) { // Don't include implication-tracking boolean constants from model
            if (value == "true" || value == "false") t = boolType; else t = intType;
            if (value.front() == '(') //the number is negative. Remove z3 formatting ( "(- 2)" => "-2" )
                value = "-" + value.substr(3, value.size() - 4); //remove "(- " from the front and ")" from the back

            std::string name; //remove run1 and run2 from names
            if (*v.name().str().rbegin() != '-') name = v.name().str();
            else name = v.name().str().substr(0, v.name().str().size()-5);
            std::string origname = ccfg->defs[name]->defmapping[name];
            values.insert({v.name().str(), std::make_shared<VariableValue>(VariableValue(t, name, origname, value))});
            //std::cout << v.name() << " = " << value << "\n";
        } else if (*(v.name().str().rbegin()+1) == '1') { //boolean tracking constant for run1-
            std::string name = v.name().str().substr(0, v.name().str().size()-5);
            paths.insert({name, value == "true"});
        }
        //std::cout << v.name().str() << " = " << value << std::endl;
    }
    //std::cout << m << std::endl;
    return {values, paths};
}

bool symEngine::updateModel(const std::vector<std::pair<std::string, Type>>& conflicts) {
    for (const auto &p : conflicts) {
        p.second == intType
        ? constraintset.emplace_back(c.int_const((p.first + _run1).c_str()) == c.int_const((p.first + _run2).c_str()))
        : constraintset.emplace_back(c.bool_const((p.first + _run1).c_str()) == c.bool_const((p.first + _run2).c_str()));
    }
    s.reset();
    for (const auto &expr : constraintset) s.add(expr);
    return s.check() == z3::sat;
}
