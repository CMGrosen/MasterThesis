//
// Created by hu on 17/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
#define ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP

#include <cassert>
#include <basicblockTreeConstructor.hpp>

struct CSSA_CFG {
    std::shared_ptr<CCFG> ccfg;
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> symboltable;
    int boolname_counter;

    CSSA_CFG(const CCFG &_ccfg, std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> table, int boolname_count)
    : ccfg{std::make_shared<CCFG>(CCFG(_ccfg))}, symboltable{std::move(table)}, boolname_counter{boolname_count} {
        update_mapstoMap(_ccfg.startNode, ccfg->startNode);
        ccfg->updateConflictEdges();
        std::map<std::string, std::string> var_to_SSAvar;
        for (const auto &s : *symboltable) {
            counter.insert({s.first, 1});
            var_to_SSAvar.insert({s.first, s.first});
        }

        place_phi_functions();
        std::cout << "here";

        for (const auto &blk : ccfg->endconcNodes) {
            std::vector<std::shared_ptr<statementNode>> vec;
            vec.reserve(blk->statements.size());
            for (size_t i = 0; i < blk->statements.size()-1; ++i) {
                vec.push_back(std::make_shared<piNode>(piNode(dynamic_cast<phiNode*>(blk->statements[i].get()))));
            }
            vec.push_back(blk->statements.back());
            blk->statements = std::move(vec);
        }
        std::set<std::shared_ptr<basicblock>> visited;
        get_thread_and_depth_level(&visited, ccfg->startNode, 0);
        for (const auto &blk : visited) {
            ccfg->pis_and_depth.emplace_back(blk, blk->depth);
        }

        std::sort(ccfg->pis_and_depth.begin(), ccfg->pis_and_depth.end(),
                [&](const std::pair<std::shared_ptr<basicblock>, int32_t>& a, const std::pair<std::shared_ptr<basicblock>, int32_t>& b) {
            return a.second < b.second;
        });

        for (const auto &blk : ccfg->nodes) blk->updateUsedVariables();

        updatePiStatements(ccfg->startNode, &var_to_SSAvar);

        update_if_statements_boolname_branches();

        update_conflict_edges_names();
    }

private:
    std::map<std::string, int> counter;
    std::map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> oldMapsTo;
    std::vector<std::shared_ptr<piNode>> pinodes;
    std::vector<std::shared_ptr<basicblock>> pinodeblocks;
    std::map<std::string, std::string> origvar_for_pis;

    void update_mapstoMap(const std::shared_ptr<basicblock> &oldNode, const std::shared_ptr<basicblock> &newNode) {
        if (oldMapsTo.insert({oldNode, newNode}).second) {
            for (size_t i = 0; i < oldNode->nexts.size(); ++i) {
                update_mapstoMap(oldNode->nexts[i], newNode->nexts[i]);
            }
        }
    }

    static std::pair<size_t, std::vector<std::string*>> find_usages_in_expression(expressionNode *expr, const std::string &var) {
        std::pair<size_t, std::vector<std::string*>> usages;
        switch (expr->getNodeType()) {
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
            case Read:
            case Literal:
                break;
            case ArrayAccess: {
                auto arrAcc = dynamic_cast<arrayAccessNode*>(expr);

                if (arrAcc->getName() == var) {
                    usages = {1, {arrAcc->getNameAsRef()}};
                } else {
                    usages.first = 0;
                }

                auto inter = find_usages_in_expression(arrAcc->getAccessor(), var);
                for (auto &i : inter.second) usages.second.push_back(i);
                usages.first += inter.first;
                break;
            } case ArrayLiteral:
                usages.first = 0;
                for (const auto &t : dynamic_cast<const arrayLiteralNode*>(expr)->getArrLit()) {
                    auto inter = find_usages_in_expression(t.get(), var);
                    usages.first += inter.first;
                    for (auto &i : inter.second) usages.second.push_back(i);
                }
                break;
            case Variable:
                if (dynamic_cast<const variableNode*>(expr)->origName == var) {
                    usages.first = 1;
                    usages.second.push_back(&dynamic_cast<variableNode*>(expr)->name);
                }
                break;
            case BinaryExpression: {
                auto binexpr = dynamic_cast<const binaryExpressionNode*>(expr);
                usages = find_usages_in_expression(binexpr->getLeft(), var);
                auto inter = find_usages_in_expression(binexpr->getRight(), var);
                for (auto &i : inter.second) usages.second.push_back(i);
                usages.first += inter.first;
                break;
            } case UnaryExpression:
                usages = find_usages_in_expression(dynamic_cast<const unaryExpressionNode*>(expr)->getExpr(), var);
                break;
        }
        return usages;
    }

    static std::tuple<bool, size_t, std::vector<std::string*>> num_usages(const std::shared_ptr<statementNode> &stmt, const std::string &var) {
        std::pair<size_t, std::vector<std::string*>> usages;
        bool event = false;
        switch (stmt->getNodeType()) {
            case Assign: {
                usages = find_usages_in_expression(dynamic_cast<assignNode*>(stmt.get())->getExpr(), var);
                break;
            } case AssignArrField: {
                auto assignmentArrayF = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                auto inter = find_usages_in_expression(assignmentArrayF->getExpr(), var);
                usages = find_usages_in_expression(assignmentArrayF->getField(), var);
                usages.first += inter.first;
                for (auto &i : inter.second) usages.second.push_back(i);
                break;
            } case While: {
                usages = find_usages_in_expression(dynamic_cast<whileNode*>(stmt.get())->getCondition(), var);
                break;
            } case If: {
                usages = find_usages_in_expression(dynamic_cast<ifElseNode*>(stmt.get())->getCondition(), var);
                break;
            } case Write: {
                usages = find_usages_in_expression(dynamic_cast<writeNode*>(stmt.get())->getExpr(), var);
                break;
            } case Event: {
                event = true;
                usages = find_usages_in_expression(dynamic_cast<eventNode*>(stmt.get())->getCondition(), var);
                break;
            } case Phi: {
                auto phi = dynamic_cast<phiNode*>(stmt.get());
                std::vector<std::string*> vec;
                vec.reserve(phi->get_variables()->size());
                for (auto & i : *phi->get_variables()) {
                    vec.push_back(&i.first);
                }
                usages = {phi->get_variables()->size(), vec};
                break;
            }
            case Pi:
            case Variable:
            case BinaryExpression:
            case UnaryExpression:
            case Skip:
            case Read:
            case Literal:
            case ArrayAccess:
            case ArrayLiteral:
            case EndFi:
            case Concurrent:
            case EndConcurrent:
            case Sequential:
            case BasicBlock:
                break;
        }
        return {event, usages.first, std::move(usages.second)};
    }

    static std::string findboolname(const std::shared_ptr<basicblock>& blk, const std::string& varname) {
        for (const auto &stmt : blk->statements) {
            if (auto ass = dynamic_cast<assignNode*>(stmt.get())) {
                if (ass->getName() == varname) {
                    return stmt->get_boolname();
                }
            } else if (auto assArrF = dynamic_cast<arrayFieldAssignNode*>(stmt.get())) {
                if (assArrF->getName() == varname) {
                    return stmt->get_boolname();
                }
            } else if (auto phiN = dynamic_cast<phiNode*>(stmt.get())) {
                if (phiN->getName() == varname) {
                    return stmt->get_boolname();
                }
            } else if (auto piN = dynamic_cast<piNode*>(stmt.get())) {
                if (piN->getName() == varname) {
                    return stmt->get_boolname();
                }
            }
        }
        assert(false);
    }

    void place_phi_functions() {
        for (const auto &pair : ccfg->conflict_edges_from) {
            std::vector<std::shared_ptr<edge>> edges = pair.second;
            std::shared_ptr<basicblock> a = pair.first;
            for (const auto &ed : edges) { //Foreach conflict-edge (a, b) do
                std::shared_ptr<basicblock> b = ed->to();
                for (const auto &v : a->defines) { //v variable defined in a
                    std::string varname = v.first;
                    if (b->uses.find(varname) != b->uses.end()) {
                        bool hasPiFunction = false;
                        for (const auto &stmt : b->statements) {
                            if (stmt->getNodeType() == Pi && dynamic_cast<piNode *>(stmt.get())->getVar() == varname) {
                                hasPiFunction = true;
                                break;
                            }
                        }
                        if (!hasPiFunction) { //if b does not have a pi function
                            std::tuple<bool, size_t, std::vector<std::string *>> result = num_usages(b->statements.back(), varname);
                            const bool event = std::get<0>(result);
                            const size_t numUsages = std::get<1>(result);
                            const std::vector<std::string *> varsToRename = std::move(std::get<2>(result));
                            std::vector<std::shared_ptr<statementNode>> vec;
                            vec.reserve(numUsages + b->statements.size());
                            Type t = symboltable->find(varname)->second->getType();

                            size_t iterations = event ? 1 : numUsages;
                            for (size_t i = 0; i < iterations; ++i) {
                                std::string argname = *(b->uses.find(varname)->second.begin());
                                std::string boolname = findboolname(ccfg->defs[argname], argname);

                                pinodes.emplace_back(std::make_shared<piNode>(piNode
                                    ( t
                                    , varname
                                    , counter[varname]++
                                    , std::vector<std::pair<std::string, std::string>>{{argname, boolname}}
                                    )
                                ));
                                vec.push_back(pinodes.back());
                                ccfg->defs.insert({pinodes.back()->getName(), b});
                                pinodeblocks.push_back(b);
                                pinodes.back()->set_boolname("-b_" + std::to_string(boolname_counter++));
                                origvar_for_pis.insert({pinodes.back()->getName(), argname});
                                ccfg->boolnameStatements.insert({vec.back()->get_boolname(), vec.back()});
                            }

                            if (!event) {
                                size_t i = numUsages;
                                for (auto item : varsToRename) {
                                    *item = "-T_" + varname + "_" + std::to_string(counter[varname] - (i--));
                                }
                            } else {
                                for (auto item : varsToRename) {
                                    *item = "-T_" + varname + "_" + std::to_string(counter[varname] - 1);
                                }
                            }

                            for (const auto &stmt: b->statements) {
                                vec.push_back(stmt);
                            }
                            b->statements = vec;
                        }
                        // if (n not in prec(s) incomplete, need prec(s) function
                        //if (ccfg->prec.find(a)->second.find(b) != ccfg->prec.find(a)->second.end())
                        if (!b->statements.empty()) {
                            for (const auto &stmt : b->statements) {
                                if (auto pi = dynamic_cast<piNode *>(stmt.get())) {
                                    std::string var = *(a->defines.find(varname)->second.begin());
                                    if (pi->getVar() == varname && !pi->contains(var)) {
                                        std::string boolname = findboolname(ccfg->defs[var], var);
                                        pi->addVariable({var, boolname});
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    static void get_thread_and_depth_level(std::set<std::shared_ptr<basicblock>> *visited, const std::shared_ptr<basicblock>& blk, size_t depth) {
        if (blk->depth < depth) blk->depth = depth;

        if (blk->statements.front()->getNodeType() == Pi) {
            visited->insert(blk);
        }
        for (const auto &nxt : blk->nexts) {
            get_thread_and_depth_level(visited, nxt, depth+1);
        }
    }


    void updatePiStatements(std::shared_ptr<basicblock> node, std::map<std::string, std::string> *var_to_SSAvar) {
        std::set<std::shared_ptr<basicblock>> blks;
        updatePiStatementsHelper(node, nullptr, &blks, var_to_SSAvar);
    }

    void updatePiStatementsHelper(std::shared_ptr<basicblock> node, std::shared_ptr<basicblock> parent, std::set<std::shared_ptr<basicblock>> *blks, std::map<std::string, std::string> *vars_to_ssa) {
        if (blks->insert(node).second || node->type == joinNode) {
            for (const auto &stmt : node->statements) {
                switch (stmt->getNodeType()) {
                    case Assign: {
                        auto assStmt = dynamic_cast<assignNode *>(stmt.get());
                        vars_to_ssa->find(assStmt->getOriginalName())->second = assStmt->getName();
                        break;
                    }
                    case AssignArrField: {
                        auto assArrF = dynamic_cast<arrayFieldAssignNode *>(stmt.get());
                        vars_to_ssa->find(assArrF->getOriginalName())->second = assArrF->getName();
                        break;
                    }
                    case Concurrent: {
                        for (size_t i = 1; i < node->nexts.size(); ++i) {
                            auto newmap = *vars_to_ssa;
                            updatePiStatementsHelper(node->nexts[i], node, blks, &newmap);
                        }
                        break;
                    } case EndConcurrent:
                        break;
                    case Sequential:
                        break;
                    case While:
                    case If: {
                        auto newmap = *vars_to_ssa;
                        updatePiStatementsHelper(node->nexts[1], node, blks, &newmap);
                        break;
                    }
                    case EndFi:
                        break;
                    case Write:
                        break;
                    case Read:
                        break;
                    case Literal:
                        break;
                    case ArrayAccess:
                        break;
                    case ArrayLiteral:
                        break;
                    case Event:
                        break;
                    case Variable:
                        break;
                    case BinaryExpression:
                        break;
                    case UnaryExpression:
                        break;
                    case Skip:
                        break;
                    case BasicBlock:
                        break;
                    case Phi: {
                        auto phiN = dynamic_cast<phiNode *>(stmt.get());
                        for (size_t i = 0; i < node->parents.size(); ++i) {
                            if (node->parents[i].lock() == parent) {
                                std::string varname = vars_to_ssa->find(phiN->getOriginalName())->second;
                                if (phiN->get_variables()->at(i).first != varname) {
                                    phiN->update_variableindex(i, {varname, findboolname(ccfg->defs[varname], varname)});
                                }
                                break;
                            }
                        }
                        vars_to_ssa->find(phiN->getOriginalName())->second = phiN->getName();
                        break;
                    }
                    case Pi: {
                        auto piN = dynamic_cast<piNode *>(stmt.get());
                        if (node->type != Coend) {
                            for (size_t i = 0; i < piN->get_variables()->size(); ++i) {
                                std::string varname = piN->get_variables()->at(i).first;
                                if (varname == origvar_for_pis[piN->getName()]) {
                                    auto it = vars_to_ssa->find(piN->getVar());
                                    piN->updateVariablesAtIndex
                                      ( i
                                      , {it->second, findboolname(ccfg->defs[it->second], it->second)}
                                      );
                                    it->second = piN->getName();
                                    break;
                                }
                            }
                        } else {
                            vars_to_ssa->find(piN->getVar())->second = piN->getName();
                        }
                        break;
                    }
                    case Assert:
                        break;
                }
            }
            if (node != ccfg->exitNode) updatePiStatementsHelper(node->nexts[0], node, blks, vars_to_ssa);
        }
    }

    void update_if_statements_boolname_branches() {
        for (const auto &n : ccfg->fiNodes) {
            auto parents = dynamic_cast<fiNode*>(n->statements.back().get())->get_parents();
            for (const auto &p : *parents) {
                auto ifN = dynamic_cast<ifElseNode*>(p->statements.back().get());
                update_if_statement_boolname_branches(ifN, p, n);
            }
        }
    }

    static void update_if_statement_boolname_branches(ifElseNode *ifstatement, const std::shared_ptr<basicblock>& start, std::shared_ptr<basicblock> goal) {
        update_if_statement_boolname_branches_helper(ifstatement, start->nexts[0], goal, true);
        update_if_statement_boolname_branches_helper(ifstatement, start->nexts[1], goal, false);
    }

    static void update_if_statement_boolname_branches_helper(ifElseNode *ifstatement, const std::shared_ptr<basicblock>& node, const std::shared_ptr<basicblock>& goal, bool branch) {
        if (node == goal) return;

        for (const auto &stmt : node->statements) {
            branch
              ? ifstatement->boolnamesForTrueBranch.push_back(stmt->get_boolname())
              : ifstatement->boolnamesForFalseBranch.push_back(stmt->get_boolname())
              ;
        }

        for (const auto &nxt : node->nexts) {
            update_if_statement_boolname_branches_helper(ifstatement, nxt, goal, branch);
        }
    }

    void update_conflict_edges_names() {
        for (auto &pair : ccfg->conflict_edges_from) {
            std::string num_conflicts = std::to_string(pair.second.size());
            std::string dcl = dynamic_cast<assignNode*>(pair.first->statements.back().get())->getName() + "-";
            for (std::shared_ptr<edge> &ed : pair.second) {
                std::string blkThreadName = ed->to()->concurrentBlock.first->get_name() + "_" + std::to_string(ed->to()->concurrentBlock.second) + "-";
                ed->name.assign(blkThreadName + dcl + num_conflicts);
            }
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
