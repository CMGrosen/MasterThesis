//
// Created by hu on 24/02/2020.
//

#include "symEngine.hpp"
#include "VariableValue.hpp"
#include <limits>
#include <stack>
#include <string>

symEngine::symEngine(const std::shared_ptr<CSSA_CCFG>& ccfg, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) :
    c{}, s{z3::solver(c)},
    constraintset{}, ccfg{ccfg}, symboltable(std::move(table)) {}

symEngine::symEngine(const symEngine &a) :
    c{}, s{z3::solver(c)},
    constraintset{}, ccfg{a.ccfg}, symboltable{a.symboltable} {}

symEngine &symEngine::operator=(const symEngine &a) {
    s = z3::solver(c);
    constraintset = a.constraintset;
    ccfg = a.ccfg;
    symboltable = a.symboltable;
    return *this;
}

symEngine::symEngine(symEngine &&a) noexcept :
    c{}, s{z3::solver(c)},
    constraintset{std::move(a.constraintset)}, ccfg{std::move(a.ccfg)}, symboltable{std::move(a.symboltable)} {}

symEngine &symEngine::operator=(symEngine &&a) noexcept {
    s = z3::solver(c);
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

void encode_boolnames_from_block(z3::context *c, const std::shared_ptr<basicblock>& blk, const std::shared_ptr<basicblock>& end, const bool val, const std::string &run, std::set<std::shared_ptr<basicblock>> *set, z3::expr_vector *res) {
    if (set->insert(blk).second) {
        res->push_back(encode_boolname(c, blk->get_name(), val, run));
        if (blk != end) {
            for (const auto &nxt : blk->nexts) {
                encode_boolnames_from_block(c, nxt, end, val, run, set, res);
            }
        }
    }
}

z3::expr encode_boolnames_from_block(z3::context *c, const std::shared_ptr<basicblock>& blk, const std::shared_ptr<basicblock>& end, const bool val, const std::string &run) {
    std::set<std::shared_ptr<basicblock>> set;
    z3::expr_vector vec(*c);
    for(const auto &nxt : blk->nexts) {
        encode_boolnames_from_block(c, nxt, end, val, run, &set, &vec);
    }
    return conjunction(vec);
}

static z3::expr encodepi(z3::context *c, Type t, const std::string& boolname, const std::string& name, std::vector<z3::expr> *constraintset) {
    z3::expr run1use = encode_boolname(c, boolname, true, _run1);
    z3::expr run2use = encode_boolname(c, boolname, true, _run2);
    /*constraintset->emplace_back(z3::ite
      ( !run1use
      , t == intType //If both are assigned, make this constraint
            ? c->int_const((name + _run1).c_str()) == c->int_const((name + _run2).c_str())
            : c->bool_const((name + _run1).c_str()) == c->bool_const((name + _run2).c_str())
      , c->bool_val(true)
      ));*/
    return z3::ite
      ( run1use || run2use
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

static z3::expr encodepi2(z3::context *c, Type t, const std::string& boolname, const std::string& name) {
    z3::expr run1use = encode_boolname(c, boolname, true, _run1);
    z3::expr run2use = encode_boolname(c, boolname, true, _run2);
    z3::expr res = run1use && run2use;

    /*constraintset->emplace_back(z3::ite
      ( !run1use
      , t == intType //If both are assigned, make this constraint
            ? c->int_const((name + _run1).c_str()) == c->int_const((name + _run2).c_str())
            : c->bool_const((name + _run1).c_str()) == c->bool_const((name + _run2).c_str())
      , c->bool_val(true)
      ));*/
    z3::expr constraint(*c);

    if (t == intType) {
        constraint = c->int_const((name + _run1).c_str()) != c->int_const((name + _run2).c_str());
    } else {
        constraint = c->bool_const((name + _run1).c_str()) != c->bool_const((name + _run2).c_str());
    }
    return res && constraint;
}

void find_events_between_blocks(const std::shared_ptr<basicblock> &first, const std::shared_ptr<basicblock> &last, std::set<std::shared_ptr<basicblock>> *encountered, std::vector<std::pair<std::shared_ptr<statementNode>, std::shared_ptr<basicblock>>> *res) {
    if (first != last) {
        if (encountered->insert(first).second) {
            if (first->statements.back()->getNodeType() == Event) {
                res->push_back({first->statements.back(), first});
            }
            for (const auto &nxt : first->nexts) find_events_between_blocks(nxt, last, encountered, res);
        }
    }
}

std::vector<std::pair<std::shared_ptr<statementNode>, std::shared_ptr<basicblock>>> find_events_between_blocks(const std::shared_ptr<basicblock> &first, const std::shared_ptr<basicblock> &last) {
    std::vector<std::pair<std::shared_ptr<statementNode>, std::shared_ptr<basicblock>>> vec;
    std::set<std::shared_ptr<basicblock>> set;
    for (const auto &nxt : first->nexts) find_events_between_blocks(nxt, last, &set, &vec);
    return vec;
}

bool symEngine::execute(std::string method) {
    add_reads();
    std::cout << constraintset.size() << std::endl;
    //z3::expr encoded = encoded_pis(&c, ccfg->pis_and_depth, {});

    //std::cout << "\n\n\n\n\nencoded:\n" << encoded.to_string() << std::endl;

    bool event_encountered = false;
    auto run1 = get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run1, &event_encountered);
    event_encountered = false;
    auto run2 = get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run2, &event_encountered);

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


    if (method == "old") {
        std::vector<z3::expr> vec;
        for (const auto &p : ccfg->pis_and_depth) {
            //Don't want to add these pi-functions if used in an event,
            // as race-conditions cannot occur here
            if (p.first->statements.back()->getNodeType() != Event) {
                for (const auto &stmt : p.first->statements) {
                    if (auto pi = dynamic_cast<piNode *>(stmt.get())) {
                        if (pi->getName().front() == '-') //don't encode endconc pis this way
                            vec.push_back(
                                    encodepi(&c, pi->getType(), p.first->get_name(), pi->getName(), &constraintset));
                    }
                }
            }
        }
        z3::expr dis = disjunction(&c, vec);
        std::cout << "debug:\n" << dis.to_string() << std::endl;
        std::cout << constraintset.size() << std::endl;
        constraintset.push_back(dis);
    } else {
        for (const auto &p : ccfg->pis_and_depth) {
            //Don't want to add these pi-functions if used in an event,
            // as race-conditions cannot occur here
            if (p.first->statements.back()->getNodeType() != Event) {
                for (const auto &stmt : p.first->statements) {
                    if (auto pi = dynamic_cast<piNode*>(stmt.get())) {
                        if (pi->getName().front() == '-') //don't encode endconc pis this way
                            possible_raceconditions.insert({pi->getName(), encodepi2(&c, pi->getType(), p.first->get_name(), pi->getName())});
                    }
                }
            }
        }
    }

/*
    for (int i = 1; i < boolname_counter; ++i) {
        std::string name = "-b_" + std::to_string(i);
        constraintset.emplace_back(c.bool_const((name + _run1).c_str()) == c.bool_const((name + _run2).c_str()));
    }*/
    for (const auto &expr : constraintset) s.add(expr);
/*
    std::string n = "reachable_4";
    s.add(c.bool_const((n + _run1).c_str()) == c.bool_val(true));
    s.add(c.bool_const((n + _run2).c_str()) == c.bool_val(true));
*/
    if (s.check() == z3::sat) {
        auto model = s.get_model();
        std::cout << "sat\n";
        std::cout << model << "\n" << std::endl;
        return true;
    } else {
        std::cout << "unsat\nProbably no race-conditions" << std::endl;
        std::stack<z3::expr> early_exits;
        std::vector<std::pair<std::string, std::string>> names;
        s.reset();
        add_reads();
        event_encountered = false;
        s.add(conjunction(get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run1, &event_encountered)));
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

z3::expr encode_unused_edges(z3::context *c, const std::string& blockboolname, const std::string &run, std::vector<option> *options) {
    z3::expr res = c->bool_const((blockboolname + run).c_str()) == c->bool_val(true);
    for (const auto &cc : *options) {
        if (blockboolname != cc.block_boolname) {
            if (cc.block_boolname.find("-brea") != std::string::npos) {
                std::cout << "break";
            }
            z3::expr inter = c->bool_const((cc.block_boolname + run).c_str()) == c->bool_val(false);
            res = res && inter;
        }
    }
    return res;
}

z3::expr possible_var_choices(z3::context *c, const std::shared_ptr<basicblock> &blk, const std::string &var_boolname, const std::string &run, std::shared_ptr<CSSA_CCFG> ccfg) {
    std::shared_ptr<basicblock> def = ccfg->boolnameBlocks[var_boolname];
    std::string assignment_var = reinterpret_cast<assignNode*>(blk->statements.back().get())->getName();
    z3::expr res = c->bool_val(true);
    for (const std::shared_ptr<edge> &conflict : ccfg->conflict_edges_from[blk]) {
        if (!CSSA_CCFG::concurrent(conflict->to(), def) && def->lessthan(conflict->to())) {
            for (const auto &stmt : conflict->to()->statements) {
                if (auto pi = dynamic_cast<piNode*>(stmt.get())) {
                    for (const auto &option : *pi->get_variables()) {
                        if (option.var == assignment_var) {
                            if (option.block_boolname.find("-brea") != std::string::npos) {
                                std::cout << "break";
                            }
                            z3::expr inter = c->bool_const((option.block_boolname + run).c_str()) == c->bool_val(false);
                            res = res && inter;
                            break;
                        }
                    }
                }
            }
        }
    }
    return res;
}

z3::expr encode_possible_outgoing(z3::context *c, const std::shared_ptr<basicblock> &blk, const std::string &run, std::shared_ptr<CSSA_CCFG> ccfg) {
    //only called from an assignment node that's concurrent and have multiple statements
    //meaning the statement prior to this assignment, is a pi-function
    auto options = reinterpret_cast<piNode*>((blk->statements.rbegin()+1)->get())->get_variables();

    z3::expr res = c->bool_val(true);
    for (const auto &o : *options) {
        if (o.block_boolname.find("-brea") != std::string::npos) {
            std::cout << "break";
        }
        z3::expr inter =
                z3::ite( c->bool_const((o.block_boolname + run).c_str())
                       , possible_var_choices(c, blk, o.var_boolname, run, ccfg)
                       , c->bool_val(true)
                       );
        res = res && inter;
    }
    return res;
}

z3::expr_vector symEngine::get_run(const std::shared_ptr<basicblock>& previous, const std::shared_ptr<basicblock> &start, const std::shared_ptr<basicblock> &end, const std::string &run, bool *encountered) {
    auto node = start;
    z3::expr_vector constraints(c);

    constraints.push_back(c.bool_const((start->get_name() + run).c_str()) == c.bool_val(true));
    for (const auto &stmt : node->statements) {
        switch (stmt->getNodeType()) {
            case Assign: {
                auto assnode = reinterpret_cast<assignNode *>(stmt.get());
                auto name = assnode->getExpr()->getType() == intType
                            ? c.int_const((assnode->getName() + run).c_str())
                            : c.bool_const((assnode->getName() + run).c_str());

                constraints.push_back(name == evaluate_expression(&c, assnode->getExpr(), run, &constraints));

                //concurrent and not the only statement in this block, meaning the one before is a pi-function
                if (start->concurrentBlock.first && node->statements.size() > 1) {
                    constraints.push_back(encode_possible_outgoing(&c, node, run, ccfg));
                }

                break;
            }
            case Concurrent: {
                auto endConc = get_end_of_concurrent_node(node);
                z3::expr_vector vec(c);
                //std::stack<z3::expr> expressions;
                int i = 0;
                bool event_encountered = false;
                for (const auto &nxt : node->nexts) {
                    bool event_found = false;
                    vec.push_back(conjunction(get_run(node, nxt, endConc->parents[i++].lock(), run, &event_found)));
                    if (event_found) {
                        event_encountered = true;
                    }
                }
                constraints.push_back(conjunction(vec));

                //if we encounter an event, we need to encode the remaining part of the program
                // if exitNode == end, then after the encoding of endConc, we're back to being sequential

                auto it = endConc->parents.begin();
                z3::expr condition = c.bool_const((it->lock()->get_name() + run).c_str()) == c.bool_val(true);
                while (++it != endConc->parents.end()) {
                    z3::expr inter = c.bool_const((it->lock()->get_name() + run).c_str()) == c.bool_val(true);
                    condition = condition && inter;
                }
                constraints.push_back(
                        z3::ite( condition
                               , conjunction(get_run(endConc->parents[0].lock(), endConc, end, run, encountered))
                               , encode_boolnames_from_block(&c, endConc->parents[0].lock(), end, false, run)
                               )
                        );
                node = end;
                break;
            }
            case If: {
                std::shared_ptr<basicblock> firstCommonChild = find_common_child(node);
                auto *ifNode = reinterpret_cast<ifElseNode*>(stmt.get());
                bool event_found_for_true = false;
                bool event_found_for_false = false;

                z3::expr truebranch = conjunction(get_run(node, node->nexts[0], firstCommonChild, run, &event_found_for_true))
                        && conjunction(&c, ifNode->boolnamesForFalseBranch, false, run);
                z3::expr falsebranch = conjunction(get_run(node, node->nexts[1], firstCommonChild, run, &event_found_for_false))
                        && conjunction(&c, ifNode->boolnamesForTrueBranch, false, run);


                z3::expr final = (
                        z3::ite(evaluate_expression(&c, ifNode->getCondition(), run, &constraints)
                                , truebranch
                                , falsebranch
                                ));

                constraints.push_back(final);

                z3::expr condition = c.bool_const((firstCommonChild->get_name() + run).c_str()) == c.bool_val(true);

                constraints.push_back(
                        z3::ite( condition
                               , conjunction(get_run(firstCommonChild, firstCommonChild->nexts[0], end, run, encountered))
                               , encode_boolnames_from_block(&c, firstCommonChild, end, false, run)
                               ));
                node = end;
                break;
            }
            case Event: {
                auto event = reinterpret_cast<eventNode*>(stmt.get());
                bool event_encountered = false;
                z3::expr condition = evaluate_expression(&c, event->getCondition(), run, &constraints);
                z3::expr truebranch = conjunction(get_run( node, node->nexts[0], end, run, &event_encountered));

                constraints.push_back(z3::ite( condition
                                             , truebranch
                                             , encode_boolnames_from_block(&c, node, end, false, run)
                                             )
                                       );
                /* ite:
                 *   condition: conjunction:
                       ite:
                         condition:      event-tracking-var
                         truebranch:     event's condition
                         falsebranch:    true
                 *   truebranch:  rest of the program following the Coend
                 *   falsebranch: false
                 */
                *encountered = true;
                node = end;
                break;
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
                break;
            case Phi: {
                auto phi = reinterpret_cast<phiNode *>(stmt.get());
                auto parents = node->parents;

                switch (phi->getType()) {
                    case intType: {
                        z3::expr name = c.int_const((phi->getName() + run).c_str());
                        for (size_t i = 0; i < parents.size(); ++i) {
                            if (previous == parents[i].lock()) {
                                constraints.push_back((name == c.int_const((phi->get_variables()->at(i).var + run).c_str())));
                                break;
                            }
                        }
                        break;
                    }
                    case boolType: {
                        z3::expr name = c.bool_const((phi->getName() + run).c_str());
                        for (size_t i = 0; i < parents.size(); ++i) {
                            if (previous == parents[i].lock()) {
                                constraints.push_back((name == c.bool_const((phi->get_variables()->at(i).var + run).c_str())));
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
                auto pi = reinterpret_cast<piNode *>(stmt.get());
                auto vars = pi->get_variables();
                std::vector<z3::expr> expressions;
                switch (pi->getType()) {
                    case intType: {
                        z3::expr name = c.int_const((pi->getName() + run).c_str());
                        for (const auto &conflict : *vars) {
                            z3::expr condition = c.bool_const((conflict.var_boolname + run).c_str()) == c.bool_val(true);
                            condition = condition && (c.bool_const((conflict.block_boolname + run).c_str()) == c.bool_val(true));
                            z3::expr tb = (name == c.int_const((conflict.var + run).c_str()));
                            if (node->type != Coend) {
                                z3::expr inter = encode_unused_edges(&c, conflict.block_boolname, run, vars);
                                tb = tb && inter;
                            }
                            expressions.push_back(z3::ite
                              ( condition
                              , tb
                              , c.bool_val(false) //unsatisfiable
                              ));
                        }
                        break;
                    }
                    case boolType: {
                        z3::expr name = c.bool_const((pi->getName() + run).c_str());
                        for (const auto &conflict : *vars) {
                            z3::expr condition = c.bool_const((conflict.var_boolname + run).c_str()) == c.bool_val(true);
                            condition = condition && (c.bool_const((conflict.block_boolname + run).c_str()) == c.bool_val(true));
                            z3::expr tb = (name == c.bool_const((conflict.var + run).c_str()));
                            if (node->type != Coend) {
                                z3::expr inter = encode_unused_edges(&c, conflict.block_boolname, run, vars);
                                tb = tb && inter;
                            }
                            expressions.push_back(z3::ite
                              ( condition
                              , tb
                              , c.bool_val(false) //unsatisfiable. Won't ever pick this option
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
                constraints.push_back(disjunction(&c, expressions));
                break;
            }
        }
    }
    if (node == end || node->nexts.empty()) {
        return constraints;
    } else {
        constraints.push_back(conjunction(get_run(node, node->nexts[0], end, run, encountered)));
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
            return c->int_const(reinterpret_cast<const readNode*>(expr)->getName().c_str()); }
        case Literal: {
            auto lit = reinterpret_cast<const literalNode*>(expr);
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
                ? c->int_const((reinterpret_cast<const variableNode*>(expr)->name + run).c_str())
                : c->bool_const((reinterpret_cast<const variableNode*>(expr)->name + run).c_str())
                ;
        }
        case BinaryExpression: {
            auto binexpr = reinterpret_cast<const binaryExpressionNode*>(expr);
            return evaluate_operator
                    ( c
                    , evaluate_expression(c, binexpr->getLeft(), run, constraints)
                    , evaluate_expression(c, binexpr->getRight(), run, constraints)
                    , binexpr->getOperator()
                    , constraints
                    );
        }
        case UnaryExpression: {
            auto unexpr = reinterpret_cast<const unaryExpressionNode*>(expr);
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
                possiblevars.push_back(var.var);
            } else {
                auto blk = ccfg->defs.find(var.var)->second;
                auto vec = constraints.find(name)->second;
                if (vec.back() != var.var
                    && contains(vec, var.var)) {
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
                    if (toinclude) possiblevars.push_back(var.var);
                }
            }
        }
        std::vector<std::string> varsToRemove;
        for (const auto &v : possiblevars) {
            auto blk = ccfg->defs.find(v)->second;
            for (const auto &s : blk->statements) {
                if (s->getNodeType() == Phi) {
                    auto phin = reinterpret_cast<phiNode*>(s.get());
                    if (phin->getName() == v) {
                        bool possiblevarsContainsOptions = false;
                        for (const auto &vv : possiblevars) {
                            std::vector<std::string> tmpvec;
                            tmpvec.reserve(phin->get_variables()->size());
                            for (const auto &vvv : *phin->get_variables()) {
                                tmpvec.push_back(vvv.var);
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
            vars.push_back(vvv.var);
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
    auto pin = reinterpret_cast<piNode*>(remaining.front().first->statements.front().get());

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
            if (blk->type == Coend && reinterpret_cast<endConcNode*>(blk->statements.back().get())->getConcNode().get() == remaining.front().first->concurrentBlock.first) {
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
    std::map<std::string, bool> booltrackconstants;
    z3::model m = s.get_model();
    //std::cout << m << std::endl;
    for (unsigned i = 0; i < m.size(); i++) {
        z3::func_decl v = m[i];
        std::string value = m.get_const_interp(v).to_string();
        if (v.name().str().front() == '-' && v.name().str()[1] == 'b') {
            booltrackconstants.insert({v.name().str(), value == "true"});
        }
    }
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

            bool defined;
            std::string run;
            std::string name; //remove run1 and run2 from names
            if (*v.name().str().rbegin() != '-') { name = v.name().str(); run = "";} //readVal
            else { name = v.name().str().substr(0, v.name().str().size()-5); run = v.name().str().substr(name.size());}
            std::string origname = ccfg->defs[name]->defmapping[name];

            if (run.empty()) defined = true; //readVal
            else defined = booltrackconstants.find(ccfg->defs[name]->get_name() + run)->second;

            values.insert({v.name().str(), std::make_shared<VariableValue>(VariableValue(t, name, origname, value, defined))});
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

bool symEngine::updateModel(const std::vector<std::pair<std::string, Type>>& conflicts, const std::vector<std::string>& boolnames) {
    s.reset();
    for (const auto &expr : constraintset) s.add(expr);
    for (const auto &p : conflicts) {
        p.second == intType
        ? s.add(c.int_const((p.first + _run1).c_str()) == c.int_const((p.first + _run2).c_str()))
        : s.add(c.bool_const((p.first + _run1).c_str()) == c.bool_const((p.first + _run2).c_str()));
    }
    for (const auto &b : boolnames) {
        s.add(c.bool_const((b + _run1).c_str()) == c.bool_const((b + _run2).c_str()));
    }

    bool satisfiable;
    switch (s.check()) {
        case z3::unsat:
            satisfiable = false;
            break;
        case z3::sat:
            satisfiable = true;
            break;
        case z3::unknown:
            satisfiable = false;
            break;
    }
    return satisfiable;
}

bool symEngine::updateModel(const z3::expr& expr) {
    s.reset();
    for (const auto &e : constraintset) s.add(e);
    s.add(expr);
    return s.check() == z3::sat;
}

z3::expr symEngine::encode_event_conditions_between_blocks(z3::context *c, const std::shared_ptr<basicblock> &first, const std::shared_ptr<basicblock> &last, const std::string &run) {
    std::vector<std::pair<std::shared_ptr<statementNode>, std::shared_ptr<basicblock>>> eventsAndBlks = find_events_between_blocks(first, last);
    z3::expr_vector vec(*c);
    for (const auto &eventBlk : eventsAndBlks) {
        vec.push_back(z3::ite( encode_boolname(c, eventBlk.second->get_name(), true, run)
                             , evaluate_expression(c, reinterpret_cast<eventNode*>(eventBlk.first.get())->getCondition(), run, &vec)
                             , c->bool_val(true)
                             ));
    }
    return conjunction(vec);
}
