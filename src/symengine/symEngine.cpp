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

static z3::expr encodepi2(z3::context *c, Type t, const std::string& boolname, const std::string& name) {
    z3::expr run1use = encode_boolname(c, boolname, true, _run1);
    z3::expr run2use = encode_boolname(c, boolname, true, _run2);
    z3::expr res = run1use && run2use;

    z3::expr constraint(*c);

    if (t == intType) {
        constraint = c->int_const((name + _run1).c_str()) != c->int_const((name + _run2).c_str());
    } else {
        constraint = c->bool_const((name + _run1).c_str()) != c->bool_const((name + _run2).c_str());
    }
    return res && constraint;
}

bool symEngine::execute() {
    add_reads();
    std::cout << constraintset.size() << std::endl;

    auto run1 = get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run1);
    auto run2 = get_run(nullptr, ccfg->startNode, ccfg->exitNode, _run2);


    z3::expr inter = conjunction(run1);
    std::cout << inter.to_string() << std::endl;
    z3::expr exp = inter && conjunction(run2);
    //std::cout << exp.to_string() << std::endl;
    constraintset.push_back(exp);
    std::cout << constraintset.size() << std::endl;

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

z3::expr encode_invalid_edges
(z3::context &c, const std::shared_ptr<basicblock> &node, z3::expr &name, const std::string &varOrigname,
 const std::string &varOptionName, const std::string &run, Type type,
 const std::vector<std::shared_ptr<edge>> &conflicts, CSSA_CCFG *ccfg)
 {
    auto currentOptionsDefBlock = ccfg->defs[varOptionName];
    z3::expr blockname = c.bool_const((currentOptionsDefBlock->get_name() + run).c_str());
    z3::expr thisEdge(c);
    z3::expr thisAssign(c);
    if (type == intType) thisAssign = (name == c.int_const((varOptionName + run).c_str()));
    else thisAssign = (name == c.bool_const((varOptionName + run).c_str()));
    z3::expr inter = c.bool_val(true);

    //Make all other options false except this one
    for (const std::shared_ptr<edge> &ed : conflicts) {
        if (ed->from() != currentOptionsDefBlock) {
            z3::expr i = (c.bool_const((ed->name + run).c_str()) == c.bool_val(false));
            inter = inter && i;
        } else {
            thisEdge = c.bool_const((ed->name + run).c_str());
        }
    }

    //Make all other conflict edges false from the option we took
    for (const std::shared_ptr<edge> &e : ccfg->conflict_edges_from[currentOptionsDefBlock]) {
        if (e->to() != node) {
            inter = inter && (c.bool_const((e->name + run).c_str()) == c.bool_val(false));
        }
    }

    /*In case there is an outgoing conflict edge from this block to the chosen option's block
    * add a constraint stating this edge cannot be taken.
    * If the last statement is an assignment to the same variable used in this pi-function,
    * then there is a conflict
    */
    if (node->statements.back()->getNodeType() == Assign) {
        if (reinterpret_cast<assignNode*>(node->statements.back().get())->getOriginalName() == varOrigname) {
            for (const std::shared_ptr<edge> &cf : ccfg->conflict_edges_from[node]) {
                if (cf->to() == currentOptionsDefBlock) {
                    inter = inter && (c.bool_const((cf->name + run).c_str()) == c.bool_val(false));
                }
            }
        }
    }
    /* For the other options, if that definition is earlier than the picked option,
     * invalidate all of them from this block and below
     */
    for (const std::shared_ptr<edge> &cf : ccfg->conflict_edges_to[node]) {
        if (cf->from() != currentOptionsDefBlock) {
            if (!CSSA_CCFG::concurrent(cf->from(), currentOptionsDefBlock) && cf->from()->lessthan(currentOptionsDefBlock)) {
                for (const std::shared_ptr<edge> &ToseqNode : ccfg->conflict_edges_from[cf->from()]) {
                    if (!CSSA_CCFG::concurrent(node, ToseqNode->to()) && node->lessthan(ToseqNode->to())) {
                        inter = inter && (c.bool_const((ToseqNode->name + run).c_str()) == c.bool_val(false));
                    }
                }
            }
        }
    }

    return z3::ite(thisEdge && blockname, inter && thisAssign, c.bool_val(false));
}

z3::expr_vector symEngine::get_run(const std::shared_ptr<basicblock>& previous, const std::shared_ptr<basicblock> &start, const std::shared_ptr<basicblock> &end, const std::string &run) {
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
                break;
            }
            case Concurrent: {
                auto endConc = get_end_of_concurrent_node(node);
                z3::expr_vector vec(c);
                //std::stack<z3::expr> expressions;
                int i = 0;
                for (const auto &nxt : node->nexts) {
                    vec.push_back(conjunction(get_run(node, nxt, endConc->parents[i++].lock(), run)));
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
                               , conjunction(get_run(endConc->parents[0].lock(), endConc, end, run))
                               , encode_boolnames_from_block(&c, endConc->parents[0].lock(), end, false, run)
                               )
                        );
                node = end;
                break;
            }
            case If: {
                std::shared_ptr<basicblock> firstCommonChild = find_common_child(node);
                auto *ifNode = reinterpret_cast<ifElseNode*>(stmt.get());

                z3::expr truebranch = conjunction(get_run(node, node->nexts[0], firstCommonChild, run))
                        && conjunction(&c, ifNode->boolnamesForFalseBranch, false, run);
                z3::expr falsebranch = conjunction(get_run(node, node->nexts[1], firstCommonChild, run))
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
                               , conjunction(get_run(firstCommonChild, firstCommonChild->nexts[0], end, run))
                               , encode_boolnames_from_block(&c, firstCommonChild, end, false, run)
                               ));
                node = end;
                break;
            }
            case Event: {
                auto event = reinterpret_cast<eventNode*>(stmt.get());
                z3::expr condition = evaluate_expression(&c, event->getCondition(), run, &constraints);
                z3::expr truebranch = conjunction(get_run( node, node->nexts[0], end, run));

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
                        z3::expr condition = (name == c.int_const((vars->begin()->var + run).c_str()));
                        expressions.push_back(condition);
                        const auto &conflicts = ccfg->conflict_edges_to[node];
                        if (node->type != Coend) {
                            for (auto it = vars->begin() + 1; it != vars->end(); ++it) {
                                z3::expr inter = encode_invalid_edges(c, node, name, pi->getVar(), it->var, run, intType, conflicts, ccfg.get());
                                expressions.push_back(inter);
                            }
                        } else {
                            for (auto it = vars->begin() + 1; it != vars->end(); ++it) {
                                z3::expr inter = (name == (c.int_const((it->var + run).c_str())));
                                expressions.push_back(inter);
                            }
                        }
                        break;
                    }
                    case boolType: {
                        z3::expr name = c.bool_const((pi->getName() + run).c_str());
                        z3::expr condition = (name == c.bool_const((vars->begin()->var + run).c_str()));
                        expressions.push_back(condition);
                        const auto &conflicts = ccfg->conflict_edges_to[node];
                        if (node->type != Coend) {
                            for (auto it = vars->begin() + 1; it != vars->end(); ++it) {
                                z3::expr inter = encode_invalid_edges(c, node, name, pi->getVar(), it->var, run, boolType, conflicts, ccfg.get());
                                expressions.push_back(inter);
                            }
                        } else {
                            for (auto it = vars->begin() + 1; it != vars->end(); ++it) {
                                z3::expr inter = (name == (c.bool_const((it->var + run).c_str())));
                                expressions.push_back(inter);
                            }
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
        // Don't include implication-tracking boolean constants from model, or conflict edges
        if (!(v.name().str().front() == '-' && v.name().str()[1] == 'b') && v.name().str().front() != '&') {
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

