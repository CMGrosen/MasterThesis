//
// Created by hu on 23/03/2020.
//

#include <DST.hpp>
#include "interpreter.hpp"

interpreter::interpreter(symEngine e) : engine{std::move(e)} {
    if (engine.execute()) {
        std::vector<std::pair<std::shared_ptr<basicblock>, std::string>> blks_and_names;
        refresh();
        for (const auto &dif : differences) {
            blks_and_names.push_back({engine.ccfg->defs.find(dif.first)->second, dif.first});
        }
        std::sort(blks_and_names.begin(), blks_and_names.end()
                , [&](const auto& a, const auto& b) {return a.first->depth < b.first->depth;});
        reach_potential_raceConditions(blks_and_names);
        std::cout << "here";
    }
}

bool interpreter::run() {
    return false;
}

void interpreter::update() {
    valuesFromModel = engine.getModel();


    for (const auto &val : valuesFromModel) {
        if (val.first[0] == '-' && val.first[1] == 'r') { //this is a -readVal
            auto res = variableValues.insert({val.second.first, {{val.first}, val.second.second}});
            if (!res.second) res.first->second.first.insert(val.first);
        } else {
            size_t len = val.first.size() - 5;
            std::string var = val.first.substr(0, len);

            if (*val.first.rbegin() == '-' && *(val.first.rbegin()+1) == '1') { //this value ends with run1-
                auto res = valuesFromModel.find(var + _run2);
                if (res == valuesFromModel.end()) {
                    differences.insert({var, {val.second.first, "undef", val.second.second}});
                } else if (val.second.first != res->second.first) {
                    differences.insert({var, {val.second.first, res->second.first, val.second.second}});
                }
                auto inserted = variableValues.insert({val.second.first, {{var}, val.second.second}});
                if (!inserted.second) inserted.first->second.first.insert(var);
            } else {
                auto res = valuesFromModel.find(var + _run1);
                if (res == valuesFromModel.end())
                    differences.insert({var, {"undef", val.second.first, val.second.second}});
            }
        }
    }

    for (const auto &var : engine.symboltable) {
        current_values.insert({var.first, {"undef", var.second->getType()}});
    }
}

void interpreter::refresh() {
    variableValues.clear();
    valuesFromModel.clear();
    differences.clear();
    current_values.clear();
    update();
}

bool interpreter::reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>& blks) {
    for (const auto &blk : blks) {

        if (reachable(blk, blk.first->defmapping.find(blk.second)->second)) return true;
    }
    return false;
}

std::pair<bool, std::vector<edge>> edges_to_take(std::shared_ptr<basicblock> current, std::shared_ptr<basicblock> conflict, const std::string& origname) {
    std::vector<edge> edges;
    if (current->depth > conflict->depth) {
        current.swap(conflict);
    }
    std::pair<bool, std::vector<edge>> res;
    for (const auto& blk : conflict->parents) {
        if (blk.lock() == current) return {true, {edge(blk.lock(), conflict)}};
        auto defs = blk.lock()->defines.find(origname);
        if (defs != blk.lock()->defines.end()) {
            for (const auto &def : defs->second) {
                if (def.front() != '-') { //overwriting this conflict. skipping.
                    return {false, {}};
                }
            }
        }
        auto inter = edges_to_take(current, blk.lock(), origname);
        if (inter.first) {
            inter.second.emplace_back(blk.lock(), conflict);
            if (edges.empty() || inter.second.size() < edges.size()) {
                edges = std::move(inter.second);
            }
        }
    }
    if (edges.empty()) return {false, {}};
    else return {true, edges};
}

std::string interpreter::compute_operator(const std::string &left, const std::string &right, op _operator) {
    std::string val;
    switch (_operator) {
        case PLUS:
            val = std::to_string(std::stoi(left) + std::stoi(right));
            break;
        case MINUS:
            val = std::to_string(std::stoi(left) - std::stoi(right));
            break;
        case MULT:
            val = std::to_string(std::stoi(left) * std::stoi(right));
            break;
        case DIV:
            val = std::to_string(std::stoi(left) / std::stoi(right));
            break;
        case MOD:
            val = std::to_string(std::stoi(left) % std::stoi(right));
            break;
        case NOT:
            val = std::to_string(left == "false");
            break;
        case AND:
            val = std::to_string(left == "true" && right == "true");
            break;
        case OR:
            val = std::to_string(left == "true" && right == "true");
            break;
        case LE:
            val = std::to_string(std::stoi(left) < std::stoi(right));
            break;
        case LEQ:
            val = std::to_string(std::stoi(left) <= std::stoi(right));
            break;
        case GE:
            val = std::to_string(std::stoi(left) > std::stoi(right));
            break;
        case GEQ:
            val = std::to_string(std::stoi(left) >= std::stoi(right));
            break;
        case EQ:
            val = std::to_string(left == right);
            break;
        case NEQ:
            val = std::to_string(left != right);
            break;
        case NEG:
            val = std::to_string(-std::stoi(left));
            break;
        case NOTUSED:
            break;
    }
    return val;
}

std::string interpreter::exec_expr(expressionNode* expr) {
    std::string val;
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
            std::cout << "expression is apparently a statement. error\n";
            assert(false);
            break;

        case Read:
            val = valuesFromModel[dynamic_cast<readNode*>(expr)->getName()].first;
            break;
        case Literal:
            val = dynamic_cast<literalNode*>(expr)->value;
            break;
        case ArrayAccess: {
            auto arracc = dynamic_cast<arrayAccessNode*>(expr);
            val = current_values[arracc->getVar()->origName + "[" + exec_expr(arracc->getAccessor()) + "]"].first;
            break;
        } case ArrayLiteral:
            //don't handle array, this is good enough for now
            break;
        case Variable:
            val = current_values[dynamic_cast<variableNode*>(expr)->origName].first;
            break;
        case BinaryExpression: {
            auto binexpr = dynamic_cast<binaryExpressionNode*>(expr);
            val = compute_operator(exec_expr(binexpr->getLeft()), exec_expr(binexpr->getRight()), binexpr->getOperator());
            break;
        } case UnaryExpression:
            auto unexpr = dynamic_cast<unaryExpressionNode*>(expr);
            std::string inter = exec_expr(unexpr->getExpr());
            val = compute_operator(inter, inter, unexpr->getOperator());
            break;
    }
    return val;
}

bool interpreter::exec_stmt(const std::shared_ptr<statementNode> &stmt) {
    switch (stmt->getNodeType()) {
        case Assign: {
            auto assNode = dynamic_cast<assignNode *>(stmt.get());
            std::string value = exec_expr(assNode->getExpr());
            current_values[assNode->getOriginalName()].first = value;
            break;
        }
        case AssignArrField: {
            auto assArrF = dynamic_cast<arrayFieldAssignNode *>(stmt.get());
            std::string name = exec_expr(assArrF->getField());
            std::string value = exec_expr(assArrF->getExpr());
            current_values[assArrF->getOriginalName() + "[" + name + "]"].first = value;
            break;
        } case Concurrent: {
            break;
        } case EndConcurrent: {
            break;
        } case Sequential: {
            break;
        } case While: {
            break;
        } case If: {
            auto ifnode = dynamic_cast<ifElseNode*>(stmt.get());
            if (exec_expr(ifnode->getCondition()) == "false") return false;
            break;
        } case EndFi: {
            break;
        } case Write: {
            break;
        } case Event: {
            auto event = dynamic_cast<eventNode *>(stmt.get());
            if (exec_expr(event->getCondition()) == "false") return false;
            break;
        } case Skip: {
            break;
        } case Phi: {
            //value already assigned, since the value that should be assigned, is the one already stored
            break;
        } case Pi: {
            //value already assigned, since the value that should be assigned, is the one already stored
            //maybe check to see if model value is the one already stored
            auto pi = dynamic_cast<piNode *>(stmt.get());
            if (!current_values.insert({pi->getName(), current_values[pi->getVar()]}).second) {
                std::cout << "error occured. This pi-node has been visited once already. Shouldn't be possible!\n";
            }
            break;
        }

        case Read:
        case Literal:
        case ArrayAccess:
        case ArrayLiteral:
        case Variable:
        case BinaryExpression:
        case UnaryExpression:
        case BasicBlock:
            std::cout << "statement is apparently an expression. Error occured\n";
            assert(false);
            break;
    }
    return true;
}

void interpreter::execute(bool going_to_conflictnode, edge ed, std::set<std::shared_ptr<basicblock>> *blks) {
    std::shared_ptr<basicblock> blk = ed.neighbours[0];
    for (const auto &stmt : blk->statements) {
        bool res = exec_stmt(stmt);
        if (stmt->getNodeType() == Event && !res) {
            std::cout << "event is false, cannot continue to next block\n";
            return;
        }
        else if (stmt->getNodeType() == If) {
            if (going_to_conflictnode && res && ed.neighbours[1] != blk->nexts[0]) {
                std::cout << "error occured. If statement should have been false\n";
            } else if (going_to_conflictnode && !res && ed.neighbours[1] != blk->nexts[1]) {
                std::cout << "error occured. If statement should have been true\n";
            }
            res ? blks->insert(blk->nexts[0]) : blks->insert(blk->nexts[1]);
        }
    }
    if (blk->type == Cobegin) {
        for (const auto &nxt : blk->nexts) blks->insert(nxt);
    } else if (blk->statements.back()->getNodeType() != If) {
        if (blk != engine.ccfg->exitNode) blks->insert(blk->nexts[0]);
    }
    blks->erase(blk);
}


bool interpreter::reachable(const std::pair<std::shared_ptr<basicblock>, std::string> &blk, const std::string& origname) {
    std::set<std::shared_ptr<basicblock>> conflictsForRun1;
    std::set<std::shared_ptr<basicblock>> conflictsForRun2;
    std::shared_ptr<basicblock> conflictNode;
    bool conflict1 = false, conflict2 = false;
    bool onconflictnode = false;
    std::string valForRun1 = std::get<0>(differences.find(blk.second)->second);
    std::string valForRun2 = std::get<1>(differences.find(blk.second)->second);

    for (const auto &value : variableValues.find(valForRun1)->second.first) {
        conflictsForRun1.insert(engine.ccfg->defs.find(value)->second);
    }
    for (const auto &value : variableValues.find(valForRun2)->second.first) {
        conflictsForRun2.insert(engine.ccfg->defs.find(value)->second);
    }

    std::set<std::shared_ptr<basicblock>> currents = {engine.ccfg->startNode};

    std::shared_ptr<basicblock> current = *currents.begin();
    while (current) {
        if (!onconflictnode) {
            std::pair<bool, std::vector<edge>> path;
            if (conflictsForRun1.find(current) != conflictsForRun1.end()) {
                path = edges_to_take(current, blk.first, origname);
                conflict1 = true;
                onconflictnode = true;
            } else if (conflictsForRun2.find(current) != conflictsForRun2.end()) {
                path = edges_to_take(current, blk.first, origname);
                conflict2 = true;
                onconflictnode = true;
            }
            if (onconflictnode) {
                if (!path.first) {
                    std::cout << "an error occured. Cannot reach conflict\n";
                }
                for (const auto &ed : path.second) {
                    execute(true, ed, &currents);
                }
                //we're at possible race-condition location. Don't want to continue from this node
                conflictNode = path.second.back().neighbours[1];
                currents.erase(conflictNode);
            }
        } else {
            if (conflict1 && conflictsForRun2.find(current) != conflictsForRun2.end()) {
                std::string current_val = current_values[origname].first;
                if (current_val == valForRun1) {
                    execute(false, edge(current, current->nexts.empty() ? nullptr : current->nexts[0]), &currents);
                    if (current_values[origname].first != current_val || current_values[origname].first == valForRun2) {
                        std::cout << "found both conflicts\n";
                    }
                }
                std::cout << "found both conflicts\n";
                return true;
            } else if (conflict2 && conflictsForRun1.find(current) != conflictsForRun1.end()) {
                std::string current_val = current_values[origname].first;
                if (current_val == valForRun2) {
                    execute(false, edge(current, current->nexts.empty() ? nullptr : current->nexts[0]), &currents);
                    if (current_values[origname].first != current_val || current_values[origname].first == valForRun1) {
                        std::cout << "found both conflicts\n";
                    }
                }
                std::cout << "found both conflicts\n";
                return true;
            } else {
                if (blk.first == current) {
                    //we're at node with race-condition but haven't yet met a node with a conflicting variable assignment
                    //unlikely to ever happen
                    std::cout << "at node with race-condition but haven't yet met a node with a conflicting variable assignment\n";
                }
                execute(false, edge(current, current->nexts.empty() ? nullptr : current->nexts[0]), &currents);
            }
        }
        current = *currents.begin();
    }
    std::cout << "here";
}
