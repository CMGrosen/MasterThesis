//
// Created by hu on 23/03/2020.
//

#include <DST.hpp>
#include "interpreter.hpp"

interpreter::interpreter(symEngine e) : engine{std::move(e)} {
    data.ccfg = engine.ccfg.get();
}

bool interpreter::run() {
    bool returnval = true;
    std::set<std::string> checklater;
    std::set<std::string> races;
    bool satisfiable = engine.execute();
    if (!satisfiable) {
        std::cout << "model unsatisfiable\n";
        returnval = false;
    }
    while (satisfiable) {
        std::vector<std::pair<std::shared_ptr<basicblock>, std::string>> blks_and_names;
        bool modified = false;
        refresh();
        for (const auto &dif : data.differences) {
            //Don't add if not visited in both runs
            if (!dif.second.executedRun1 || !dif.second.executedRun2) {
                if (*dif.first.begin() == '-') {//Pi
                    checklater.insert(dif.first);
                    modified = true;
                }
                continue;
            }

            //Don't add difference if pi-function is used in an event
            //Also only include those pi-functions which aren't on Coend blocks
            std::shared_ptr<basicblock> blk = engine.ccfg->defs[dif.first];
            std::shared_ptr<statementNode> def = blk->defsite[dif.first];
            if (def->getNodeType() == Pi && blk->statements.back()->getNodeType() != Event && dynamic_cast<piNode*>(def.get())->getName().front() == '-') {
                blks_and_names.emplace_back(engine.ccfg->defs.find(dif.first)->second, dif.first);
            }
        }
        if (blks_and_names.empty()) {
            std::cout << "both runs identical. No potential race-conditions found\n";
            auto firstEarlyExit = data.valuesFromModel.find(std::string("0_early_exit_1") + _run1);
            if (firstEarlyExit == data.valuesFromModel.end()) {
                std::cout << "no early exits found. Program has no race-conditions\n";
            } else {
                while (firstEarlyExit != data.valuesFromModel.end() && firstEarlyExit->first.front() >= '0' &&
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
            std::vector<std::string> foundraces;
            foundraces.reserve(blks_and_names.size());
            std::sort(blks_and_names.begin(), blks_and_names.end(),
                      [&](const auto &a, const auto &b) { return a.first->depth < b.first->depth; });
            if (modified) returnval = reach_potential_raceConditions({blks_and_names.front()}, &foundraces);
            else returnval = reach_potential_raceConditions(blks_and_names, &foundraces);
            std::vector<std::pair<std::string, Type>> varstoupdate;
            if (!returnval) {
                std::cout << "didn't find race-condition: updating constraints: ...\n";
                races.insert(blks_and_names.front().second);
            } else {
                //Still probably satisfiable. Update model to not include found race-conditions
                for (const auto &race : foundraces) {
                    races.insert(race);
                }
            }
            for (const auto &conflict : races) {
                varstoupdate.emplace_back(conflict, engine.symboltable[engine.ccfg->defs[conflict]->defmapping[conflict]]->getType());
            }
            std::vector<std::string> bools;
            bools.reserve(1);
            if (!checklater.empty()) {
                auto first = checklater.begin();
                bools.push_back(data.ccfg->defs[*first]->defsite[*first]->get_boolname());
                checklater.erase(first);
            }

            satisfiable = engine.updateModel(varstoupdate, bools);
        }
    }
    return returnval;
}

void interpreter::update() {
    auto res = engine.getModel();
    data.valuesFromModel = std::move(res.first);
    data.statementsExecuted = std::move(res.second);

    std::shared_ptr<VariableValue> undef = std::make_shared<VariableValue>(VariableValue
            ( errorType
            , "undef"
            , "undef"
            , "undef"
            , false
            ));

    for (const auto &val : data.valuesFromModel) {
        auto inserted = data.variableValues.insert({val.second->value, {val.second}});
        if (!inserted.second) inserted.first->second.insert(val.second);
        if (val.first[0] == '-' && val.first[1] == 'r') { //this is a -readVal

        } else {
            size_t len = val.first.size() - 5;
            std::string var = val.first.substr(0, len);

            if (*val.first.rbegin() == '-' && *(val.first.rbegin()+1) == '1') { //this value ends with run1-
                auto res = data.valuesFromModel.find(var + _run2);
                if (res == data.valuesFromModel.end()) {
                    data.differences.insert({var, Difference(val.second, undef, val.second->defined, false)});
                } else if (val.second->value != res->second->value) {
                    data.differences.insert({var, Difference(val.second, res->second, val.second->defined, res->second->defined)});
                }
            } else {
                auto res = data.valuesFromModel.find(var + _run1);
                if (res == data.valuesFromModel.end())
                    data.differences.insert({var, Difference(undef, val.second, false, val.second->defined)});
                //no need for else clause.
                // Else-clause will either have reached the else-if branch of the first if-statement
                // or will later through the iteration
            }
        }
    }
}

void interpreter::refresh() {
    data.variableValues.clear();
    data.valuesFromModel.clear();
    data.differences.clear();
    update();
}

static std::set<basicblock*> vecToSet(std::vector<std::weak_ptr<basicblock>> *vec) {
    std::set<basicblock*> res;
    for (const auto &ele : *vec) {
        res.insert(ele.lock().get());
    }
    return res;
}

std::map<std::string, Value> interpreter::get_current_values() {
    std::map<std::string, Value> current_values;
    for (const auto &var : engine.symboltable) {
        current_values.insert({var.first, Value{"undef", nullptr, var.second->getType()}});
    }
    return current_values;
}

std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> interpreter::get_threads_to_finish() {
    std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> threadsToFinish;
    for (const auto &conc : engine.ccfg->endconcNodes) {
        threadsToFinish.insert({conc, vecToSet(&conc->parents)});
    }
    return threadsToFinish;
}

bool interpreter::reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>& blks, std::vector<std::string> *foundConditions) {
    bool foundrace = false;
    for (const auto &blk : blks) {
        std::string origname = blk.first->defmapping.find(blk.second)->second;
        std::string conflictvar = blk.first->defsite.find(blk.second)->second->getNodeType() == Pi ? blk.second : origname;
        if (reachable(blk, origname, conflictvar)) {
            foundConditions->push_back(blk.second);
            foundrace = true;
        }
    }
    return foundrace;
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

std::string interpreter::exec_expr(expressionNode* expr, const std::map<std::string, Value> *current_values) {
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
            val = data.valuesFromModel.find(dynamic_cast<readNode*>(expr)->getName())->second->value;
            break;
        case Literal:
            val = dynamic_cast<literalNode*>(expr)->value;
            break;
        case ArrayAccess: {
            auto arracc = dynamic_cast<arrayAccessNode*>(expr);
            val = current_values->find(arracc->getVar()->origName + "[" + exec_expr(arracc->getAccessor(), current_values) + "]")->second.val;
            break;
        } case ArrayLiteral:
            //don't handle array, this is good enough for now
            break;
        case Variable: {
            auto var = dynamic_cast<variableNode*>(expr);
            if (engine.ccfg->defs.find(var->name)->second->defsite.find(var->name)->second->getNodeType() == Pi) {//pi-node
                val = current_values->find(var->name)->second.val;
            } else {
                val = current_values->find(var->origName)->second.val;
            }
            break;
        } case BinaryExpression: {
            auto binexpr = dynamic_cast<binaryExpressionNode*>(expr);
            val = compute_operator(exec_expr(binexpr->getLeft(), current_values), exec_expr(binexpr->getRight(), current_values), binexpr->getOperator());
            break;
        } case UnaryExpression:
            auto unexpr = dynamic_cast<unaryExpressionNode*>(expr);
            std::string inter = exec_expr(unexpr->getExpr(), current_values);
            val = compute_operator(inter, inter, unexpr->getOperator());
            break;
    }
    return val;
}

std::pair<bool, bool> interpreter::exec_stmt(const std::shared_ptr<statementNode> &stmt, std::map<std::string, Value> *current_values, bool conflictIsCoend) {
    switch (stmt->getNodeType()) {
        case Assign: {
            auto assNode = dynamic_cast<assignNode *>(stmt.get());
            std::string value = exec_expr(assNode->getExpr(), current_values);
            auto val = current_values->find(assNode->getOriginalName());
            val->second.val = value;
            val->second.statement = stmt;
            break;
        }
        case AssignArrField: {
            auto assArrF = dynamic_cast<arrayFieldAssignNode *>(stmt.get());
            std::string name = exec_expr(assArrF->getField(), current_values);
            std::string value = exec_expr(assArrF->getExpr(), current_values);
            auto val = current_values->find(assArrF->getOriginalName() + "[" + name + "]");
            val->second.val = value;
            val->second.statement = stmt;
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
            if (exec_expr(ifnode->getCondition(), current_values) == "false") return {false, true};
            break;
        } case EndFi: {
            break;
        } case Write: {
            break;
        } case Event: {
            auto event = dynamic_cast<eventNode *>(stmt.get());
            if (exec_expr(event->getCondition(), current_values) == "false") return {false, true};
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
            std::string val = current_values->find(pi->getVar())->second.val;
            auto res = current_values->find(pi->getName());
            if (res != current_values->end()) return {true, true};

            if (conflictIsCoend) {
                current_values->insert({pi->getName(), current_values->find(pi->getVar())->second});
            } else {
                if (data.valuesFromModel.find(pi->getName() + _run1)->second->value == val ||
                    data.valuesFromModel.find(pi->getName() + _run2)->second->value == val) {
                    current_values->insert({pi->getName(), current_values->find(pi->getVar())->second});
                } else {
                    //std::cout << "unexpected value for pi, adding anyway\n";
                    return {true, false};
                }
            }
            break;
        } case Assert: {
            auto assertstmt = dynamic_cast<assertNode*>(stmt.get());
            auto res = exec_expr(assertstmt->getCondition(), current_values);
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

bool interpreter::execute(const std::shared_ptr<basicblock>& blk, state *s) {
    std::vector<std::shared_ptr<basicblock>> blksToInsert;
    bool piSuccessfullyExecuted = false;
    for (const auto &stmt : blk->statements) {
        std::pair<bool, bool> res = exec_stmt(stmt, &s->current_values, s->conflictIsCoend);
        if (stmt->getNodeType() == Pi && res.second) {
            piSuccessfullyExecuted = true;
        }
        if (!res.second) {
            return piSuccessfullyExecuted;
        }
        if (stmt->getNodeType() == Event && !res.first) {
            std::cout << "event is false, cannot continue to next block\n";
            if (data.statementsExecuted.find(blk->nexts[0]->statements.front()->get_boolname())->second) {
                std::cout << "This event should have been true!\n";
            }
            return false;
        }
        else if (stmt->getNodeType() == If) {
            if (res.first && data.statementsExecuted.find(blk->nexts[1]->statements.front()->get_boolname())->second) {
                std::cout << "error occured. If statement should have been false\n";
            } else if (!res.first && data.statementsExecuted.find(blk->nexts[0]->statements.front()->get_boolname())->second) {
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
            if (!s->threadsToFinish.find(blk->nexts[0])->second.empty()) {
                s->threadsToFinish.find(blk->nexts[0])->second.erase(blk.get());
                if (s->threadsToFinish.find(blk->nexts[0])->second.empty()) {
                    blksToInsert.push_back(blk->nexts[0]);
                }
            }
        } else {
            blksToInsert.push_back(blk->nexts[0]);
        }
    }

    s->updateVisited(blk, std::move(blksToInsert));
    return true;
}

bool interpreter::recursive_read(const std::shared_ptr<basicblock>& current, state s) {
    if (current == s.conflictNode && !s.conflicts.first && !s.conflicts.second) { //if we're on the block with the conflicting values
        if(!execute(current, &s)) { //This couldn't be executed. Values aren't the expected, so another attempted execution should run
            //std::cout << "something went wrong\n";
            return false;
        } else {
            for (const auto &nxt : current->nexts) s.currents.erase(nxt); //remove nexts, as we don't want to visit coming blocks
            if(!s.updateConflict(current)) {
                //std::cout << "value of conflict node is not from either run\n";
                return false;
            }/* else {
                s.updateVisited(current, {});
            }*/
        }
        // We previously encountered the conflicting node.
        // Check if current is a possible other value for the conflict
    } else if ((!s.conflictIsCoend && s.onconflictnode && s.isConflicting(current)) || (s.conflictIsCoend && current == s.conflictNode)) {
        if (!execute(current, &s)) { //This couldn't be executed. Values aren't the expected, so another attempted execution should run
            //std::cout << "something went wrong\n";
            return false;
        } else { //Everything fine. Report found race-condition
            if (s.updateVal(current)) {
                std::cout << s.report_racecondition() << std::endl;
                return true;
            } else {
                //std::cout << "values are identical. Disregard\n";
                return false;
            }
        }
    } else if (!execute(current, &s)) { //Pi statement didn't get an expected value according to model. Stop this execution
        return false;
    }
    //execution went well. Continue execution of all possible future statements
    if (s.currents.size() == 1) {
        std::shared_ptr<basicblock> nCurrent = *s.currents.begin();
        return recursive_read(nCurrent, std::move(s));
    } else if (!s.currents.empty()) {
        for (const auto &nxt : s.currents) {
            if (recursive_read(nxt, state(s))) {
                return true;
            }
        }
    }
    //If we got here, we cannot execute
    return false;
}

bool interpreter::reachable(const std::pair<std::shared_ptr<basicblock>, std::string> &blk, const std::string& origname, const std::string& conflictvar) {
    std::set<std::shared_ptr<basicblock>> conflictsForRun1;
    std::set<std::shared_ptr<basicblock>> conflictsForRun2;
    std::shared_ptr<basicblock> conflictNode = blk.first;
    std::pair<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> conflictingDefs;
    std::string valForRun1 = data.differences.find(blk.second)->second.run1;
    std::string valForRun2 = data.differences.find(blk.second)->second.run2;

    for (const auto &value : data.variableValues.find(valForRun1)->second) {
        if (origname == value->origName) {
            conflictsForRun1.insert(engine.ccfg->defs.find(value->name)->second);
        }
    }
    for (const auto &value : data.variableValues.find(valForRun2)->second) {
        if (origname == value->origName) {
            conflictsForRun2.insert(engine.ccfg->defs.find(value->name)->second);
        }
    }

    std::set<std::shared_ptr<basicblock>> currents = {engine.ccfg->startNode};

    state s = state
            ( std::move(conflictsForRun1)
            , std::move(conflictsForRun2)
            , std::move(conflictNode)
            , get_threads_to_finish()
            , get_current_values()
            , valForRun1
            , valForRun2
            , &data
            );
    s.origname = origname;
    s.conflictvar = conflictvar;
    s.currents = {engine.ccfg->startNode};
    return recursive_read(engine.ccfg->startNode, std::move(s));
}

