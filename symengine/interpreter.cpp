//
// Created by hu on 23/03/2020.
//

#include <DST.hpp>
#include "interpreter.hpp"

interpreter::interpreter(symEngine e) : engine{std::move(e)} {

}

bool interpreter::run() {
    bool returnval = true;
    bool satisfiable = engine.execute();
    if (!satisfiable) {
        std::cout << "model unsatisfiable\n";
        returnval = false;
    }
    while (satisfiable) {
        std::vector<std::pair<std::shared_ptr<basicblock>, std::string>> blks_and_names;
        refresh();
        for (const auto &dif : differences) {
            //Don't add difference if pi-function is used in an event
            std::shared_ptr<basicblock> blk = engine.ccfg->defs[dif.first];
            if (blk->defsite[dif.first]->getNodeType() == Pi && blk->statements.back()->getNodeType() != Event) {
                blks_and_names.emplace_back(engine.ccfg->defs.find(dif.first)->second, dif.first);
            }
        }
        if (blks_and_names.empty()) {
            std::cout << "both runs identical. No potential race-conditions found\n";
            auto firstEarlyExit = valuesFromModel.find(std::string("0_early_exit_1") + _run1);
            if (firstEarlyExit == valuesFromModel.end()) {
                std::cout << "no early exits found. Program has no race-conditions\n";
            } else {
                while (firstEarlyExit != valuesFromModel.end() && firstEarlyExit->first.front() >= '0' &&
                       firstEarlyExit->first.front() <= '9') {
                    //Still have values in model. Current iterator points to an early_exit variable
                    //This is a map, so keys are sorted lexiographically. Only early exists starts with a number
                    //So when a key that doesn't start with a number is located, there are no early exit variables remaining in the model
                    if (firstEarlyExit->second->value == "true") {//We exited the program early
                        std::cout << firstEarlyExit->first << " = " << firstEarlyExit->second->value << "\n";
                        returnval = false;
                    }
                    ++firstEarlyExit;
                }
            }
            return returnval;
        } else {
            std::vector<std::string> races;
            races.reserve(differences.size());
            std::sort(blks_and_names.begin(), blks_and_names.end(),
                      [&](const auto &a, const auto &b) { return a.first->depth < b.first->depth; });
            returnval = reach_potential_raceConditions(blks_and_names, &races);
            if (!returnval) {
                std::cout << "didn't find race-condition: updating constraints: ...\n";
                auto conflict = blks_and_names.front();
                Type t = engine.symboltable[conflict.first->defmapping[conflict.second]]->getType();
                satisfiable = engine.updateModel({{conflict.second, t}}); //Tell engine to make this variable equal between runs, and get new model
            } else {
                //Still probably satisfiable. Update model to not include found race-conditions
                std::vector<std::pair<std::string, Type>> varstoupdate;
                for (const auto &conflict : races) {
                    varstoupdate.emplace_back(conflict, engine.symboltable[engine.ccfg->defs[conflict]->defmapping[conflict]]->getType());
                }
                satisfiable = engine.updateModel(varstoupdate);
            }
        }
    }
    return returnval;
}

void interpreter::update() {
    auto res = engine.getModel();
    valuesFromModel = std::move(res.first);
    statementsExecuted = std::move(res.second);

    std::shared_ptr<VariableValue> undef = std::make_shared<VariableValue>(VariableValue
            ( errorType
            , "undef"
            , "undef"
            , "undef"
            ));

    for (const auto &val : valuesFromModel) {
        if (val.first[0] == '-' && val.first[1] == 'r') { //this is a -readVal
            auto res = variableValues.insert({val.second->value, {val.second}});
            if (!res.second) res.first->second.insert(val.second);
        } else {
            size_t len = val.first.size() - 5;
            std::string var = val.first.substr(0, len);

            if (*val.first.rbegin() == '-' && *(val.first.rbegin()+1) == '1') { //this value ends with run1-
                auto res = valuesFromModel.find(var + _run2);
                if (res == valuesFromModel.end()) {
                    differences.insert({var, Difference(val.second, undef)});
                } else if (val.second->value != res->second->value) {
                    differences.insert({var, Difference(val.second, res->second)});
                }
                auto inserted = variableValues.insert({val.second->value, {val.second}});
                if (!inserted.second) inserted.first->second.insert(val.second);
            } else {
                auto res = valuesFromModel.find(var + _run1);
                if (res == valuesFromModel.end())
                    differences.insert({var, Difference(undef, val.second)});
                //no need for else clause.
                // Else-clause will either have reached the else-if branch of the first if-statement
                // or will later through the iteration
            }
        }
    }

    for (const auto &var : engine.symboltable) {
        current_values.insert({var.first, {"undef", var.second->getType()}});
    }
    refreshConcs();
}

void interpreter::refresh() {
    variableValues.clear();
    valuesFromModel.clear();
    differences.clear();
    current_values.clear();
    update();
}

static std::set<basicblock*> vecToSet(std::vector<std::weak_ptr<basicblock>> *vec) {
    std::set<basicblock*> res;
    for (const auto &ele : *vec) {
        res.insert(ele.lock().get());
    }
    return res;
}

void interpreter::refreshConcs() {
    threadsToFinish.clear();
    for (const auto &conc : engine.ccfg->endconcNodes) {
        threadsToFinish.insert({conc, vecToSet(&conc->parents)});
    }
}

bool interpreter::reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>& blks, std::vector<std::string> *foundConditions) {
    bool foundrace = false;
    for (const auto &blk : blks) {
        if (reachable(blk, blk.first->defmapping.find(blk.second)->second)) {
            foundConditions->push_back(blk.second);
            foundrace = true;
        }
        refreshConcs();
        std::vector<std::string> pis;
        for (auto &p : current_values) {
            p.second.first = "undef";
            if (p.first[0] == '-' && p.first[1] == 'T') pis.push_back(p.first);
        }
        for (const auto &p : pis) current_values.erase(p);
    }
    return foundrace;
}

std::pair<bool, std::vector<edge>> interpreter::edges_to_take(std::shared_ptr<basicblock> current, std::shared_ptr<basicblock> conflict, const std::string& origname) {
    std::vector<edge> edges;
    if (current->depth > conflict->depth) {
        current.swap(conflict);
    }
    std::pair<bool, std::vector<edge>> res;
    for (const auto& blk : conflict->parents) {
        if (blk.lock() == current) return {true, {edge(blk.lock(), conflict)}};
        //if block wasn't visited during an execution, then we don't want to consider it
        if (valuesFromModel.find(blk.lock()->statements.front()->get_boolname() + _run1)->second->value == "false") {
            continue;
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

static std::string btos(bool val) {
    return val ? "true" : "false";
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
            val = btos(left == "false");
            break;
        case AND:
            val = btos(left == "true" && right == "true");
            break;
        case OR:
            val = btos(left == "true" || right == "true");
            break;
        case LE:
            val = btos(std::stoi(left) < std::stoi(right));
            break;
        case LEQ:
            val = btos(std::stoi(left) <= std::stoi(right));
            break;
        case GE:
            val = btos(std::stoi(left) > std::stoi(right));
            break;
        case GEQ:
            val = btos(std::stoi(left) >= std::stoi(right));
            break;
        case EQ:
            val = btos(left == right);
            break;
        case NEQ:
            val = btos(left != right);
            break;
        case NEG:
            val = std::to_string(-std::stoi(left));
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
        case Assert:
            std::cout << "expression is apparently a statement. error\n";
            assert(false);
            break;

        case Read:
            val = valuesFromModel.find(dynamic_cast<readNode*>(expr)->getName())->second->value;
            break;
        case Literal:
            val = dynamic_cast<literalNode*>(expr)->value;
            break;
        case ArrayAccess: {
            auto arracc = dynamic_cast<arrayAccessNode*>(expr);
            val = current_values.find(arracc->getVar()->origName + "[" + exec_expr(arracc->getAccessor()) + "]")->second.first;
            break;
        } case ArrayLiteral:
            //don't handle array, this is good enough for now
            break;
        case Variable: {
            auto var = dynamic_cast<variableNode*>(expr);
            if (var->name[0] == '-' && var->name[1] == 'T') {//pi-node
                val = current_values.find(var->name)->second.first;
            } else {
                val = current_values.find(var->origName)->second.first;
            }
            break;
        } case BinaryExpression: {
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

std::pair<bool, bool> interpreter::exec_stmt(const std::shared_ptr<statementNode> &stmt) {
    switch (stmt->getNodeType()) {
        case Assign: {
            auto assNode = dynamic_cast<assignNode *>(stmt.get());
            std::string value = exec_expr(assNode->getExpr());
            current_values.find(assNode->getOriginalName())->second.first = value;
            break;
        }
        case AssignArrField: {
            auto assArrF = dynamic_cast<arrayFieldAssignNode *>(stmt.get());
            std::string name = exec_expr(assArrF->getField());
            std::string value = exec_expr(assArrF->getExpr());
            current_values.find(assArrF->getOriginalName() + "[" + name + "]")->second.first = value;
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
            if (exec_expr(ifnode->getCondition()) == "false") return {false, true};
            break;
        } case EndFi: {
            break;
        } case Write: {
            break;
        } case Event: {
            auto event = dynamic_cast<eventNode *>(stmt.get());
            if (exec_expr(event->getCondition()) == "false") return {false, true};
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
            std::string val = current_values.find(pi->getVar())->second.first;
            auto res = current_values.find(pi->getName());
            if (res != current_values.end()) return {true, true};

            if (valuesFromModel.find(pi->getName() + _run1)->second->value == val || valuesFromModel.find(pi->getName() + _run2)->second->value == val) {
                current_values.insert({pi->getName(), current_values.find(pi->getVar())->second});
            } else {
                std::cout << "visit this block later\n";
                return {true, false};
            }
            break;
        } case Assert: {
            auto assertstmt = dynamic_cast<assertNode*>(stmt.get());
            auto res = exec_expr(assertstmt->getCondition());
            if (res != "true") {
                std::cout << "assertion in code\nExpected (" << assertstmt->getCondition()->to_string() << ") to be true\nWas " << res;
                assert(false);
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
    return {true, true};
}

bool interpreter::execute(const std::shared_ptr<basicblock>& blk, std::set<std::shared_ptr<basicblock>> *blks) {
    std::vector<std::shared_ptr<basicblock>> blksToInsert;
    for (const auto &stmt : blk->statements) {
        std::pair<bool, bool> res = exec_stmt(stmt);
        if (!res.second) {
            visitLater.insert(blk);
            blks->erase(blk);
            return false;
        }
        if (stmt->getNodeType() == Event && !res.first) {
            std::cout << "event is false, cannot continue to next block\n";
            if (statementsExecuted.find(blk->nexts[0]->statements.front()->get_boolname())->second) {
                std::cout << "This event should have been true!\n";
            }
            return false;
        }
        else if (stmt->getNodeType() == If) {
            if (res.first && statementsExecuted.find(blk->nexts[1]->statements.front()->get_boolname())->second) {
                std::cout << "error occured. If statement should have been false\n";
            } else if (!res.first && statementsExecuted.find(blk->nexts[0]->statements.front()->get_boolname())->second) {
                std::cout << "error occured. If statement should have been true\n";
            } else {
                res.first ? blksToInsert.push_back(blk->nexts[0]) : blksToInsert.push_back(blk->nexts[1]);
            }
        }
    }
    if (blk->type == Cobegin) {
        for (const auto &nxt : blk->nexts) blksToInsert.push_back(nxt);
    } else if (blk->statements.back()->getNodeType() != If && !blk->nexts.empty()) {
        if (blk->nexts[0]->type == Coend) {
            if (!threadsToFinish.find(blk->nexts[0])->second.empty()) {
                threadsToFinish.find(blk->nexts[0])->second.erase(blk.get());
            } else {
                blksToInsert.push_back(blk->nexts[0]);
            }
        } else {
            blksToInsert.push_back(blk->nexts[0]);
        }
    }

    for (const auto &nBlk : blksToInsert) {blks->insert(nBlk);}
    blks->erase(blk);
    return true;
}


bool interpreter::reachable(const std::pair<std::shared_ptr<basicblock>, std::string> &blk, const std::string& origname) {
    std::set<std::shared_ptr<basicblock>> conflictsForRun1;
    std::set<std::shared_ptr<basicblock>> conflictsForRun2;
    std::shared_ptr<basicblock> conflictNode;
    std::pair<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> conflictingDefs;
    bool conflict1 = false, conflict2 = false;
    bool onconflictnode = false;
    bool checkvisitLater = false;
    bool found = false;
    std::string valForRun1 = differences.find(blk.second)->second.run1;
    std::string valForRun2 = differences.find(blk.second)->second.run2;

    for (const auto &value : variableValues.find(valForRun1)->second) {
        conflictsForRun1.insert(engine.ccfg->defs.find(value->name)->second);
    }
    for (const auto &value : variableValues.find(valForRun2)->second) {
        conflictsForRun2.insert(engine.ccfg->defs.find(value->name)->second);
    }

    std::set<std::shared_ptr<basicblock>> currents = {engine.ccfg->startNode};

    std::shared_ptr<basicblock> current = *currents.begin();
    while (current) {
        if (!onconflictnode) {
            std::pair<bool, std::vector<edge>> path;
            if (conflictsForRun1.find(current) != conflictsForRun1.end()) {
                conflictingDefs.first = current;
                path = edges_to_take(current, blk.first, origname);
                conflict1 = true;
                onconflictnode = true;
            } else if (conflictsForRun2.find(current) != conflictsForRun2.end()) {
                conflictingDefs.second = current;
                path = edges_to_take(current, blk.first, origname);
                conflict2 = true;
                onconflictnode = true;
            }
            if (onconflictnode) {
                if (!path.first) {
                    std::cout << "an error occured. Cannot reach conflict\n";
                    currents.erase(current);
                } else {
                    for (const auto &ed : path.second) {
                        if (!execute(ed.neighbours[0], &currents)) checkvisitLater = true;
                    }
                    //we're at possible race-condition location. Don't want to continue from this node
                    conflictNode = path.second.back().neighbours[1];
                    currents.erase(conflictNode);
                }
            } else {
                if (!execute(current, &currents)) checkvisitLater = true;
            }
        } else {
            if (conflict1 && conflictsForRun2.find(current) != conflictsForRun2.end()) {
                std::string current_val = current_values[origname].first;
                conflictingDefs.second = current;
                execute(current, &currents);
                if (current_val == valForRun1) {
                    if (current_values[origname].first != current_val || current_values[origname].first == valForRun2) {
                        std::cout << "found both conflicts\n";
                    }
                }

                std::cout << report_racecondition(origname, conflictNode->statements.back(), {conflictingDefs.first, current_val}, {conflictingDefs.second, current_values[origname].first}) << "\n";
                return true;
            } else if (conflict2 && conflictsForRun1.find(current) != conflictsForRun1.end()) {
                std::string current_val = current_values[origname].first;
                conflictingDefs.first = current;
                execute(current, &currents);
                if (current_val == valForRun2) {
                    if (current_values[origname].first != current_val || current_values[origname].first == valForRun1) {
                        std::cout << "found both conflicts\n";
                    }
                }
                std::cout << report_racecondition(origname, conflictNode->statements.back(), {conflictingDefs.first, current_values[origname].first}, {conflictingDefs.second, current_val}) << "\n";
                return true;
            } else {
                if (blk.first == current) {
                    //we're at node with race-condition but haven't yet met a node with a conflicting variable assignment
                    //unlikely to ever happen
                    std::cout << "at node with race-condition but haven't yet met a node with a conflicting variable assignment\n";
                }
                if (!execute(current, &currents)) checkvisitLater = true;
            }
        }/*
        if (currents.empty() && !visitLater.empty()) {
            for (const auto &v : visitLater) {
                currents.insert(v);
            }
            checkvisitLater = false;
        }*/
        current = *currents.begin();
        /*if (checkvisitLater) {
            currents.insert(*visitLater.begin());
            visitLater.erase(visitLater.begin());
            checkvisitLater = false;
        }*/
    }
    return false;
}

std::string interpreter::report_racecondition
  ( std::string name
  , std::shared_ptr<statementNode> stmtUsage
  , std::pair<std::shared_ptr<basicblock>, std::string> defVal1
  , std::pair<std::shared_ptr<basicblock>, std::string> defVal2
  ) {
    std::shared_ptr<statementNode> def1 = defVal1.first->defsite[*defVal1.first->defines[name].begin()];
    std::shared_ptr<statementNode> def2 = defVal2.first->defsite[*defVal2.first->defines[name].begin()];
    std::string raceconditionStr =
    "Use of variable '" + name + "' in statement: '" + stmtUsage->strOnSourceForm() + "' line " + std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
    + defVal1.second + " defined in statement: '" + def1->strOnSourceForm() + "' on line " + std::to_string(def1->get_linenum()) + "\n"
    "and\n"
    + defVal2.second + " defined in statement: '" + def2->strOnSourceForm() + "' on line " + std::to_string(def2->get_linenum()) + "\n";


    return raceconditionStr;
}
