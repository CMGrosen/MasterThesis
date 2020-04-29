//
// Created by hu on 08/04/2020.
//

#include "state.hpp"
#include "symEngine.hpp"

state::state() {
    conflict1 = conflict2 = onconflictnode = found = false;
}
state::state(std::set<std::shared_ptr<basicblock>> cr1, std::set<std::shared_ptr<basicblock>> cr2,
        std::shared_ptr<basicblock> cn,
        std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> _threadsToFinish,
        std::map<std::string, Value> cv,
        std::string v1, std::string v2, interpreterData* _interdata)
        : conflictingDefs{}, threadsToFinish{std::move(_threadsToFinish)}, current_values{std::move(cv)}, currents{}
        { conflictsForRun1 = std::move(cr1);
          conflictsForRun2 = std::move(cr2);
          conflictNode = std::move(cn);
          conflictingDefs = {};
          valForRun1 = std::move(v1);
          valForRun2 = std::move(v2);

          conflict1 = conflict2 = onconflictnode = found = false;
          conflictIsCoend = conflictNode->type == Coend;
          conflicts = {nullptr, nullptr};
          interdata = _interdata;
        }

void state::findassignedconflicts(const std::string &val, const std::pair<std::shared_ptr<basicblock>, std::shared_ptr<statementNode>> &def, std::set<std::pair<std::shared_ptr<basicblock>, std::shared_ptr<statementNode>>> *conflicts) {
    std::vector<option> *possibilities;
    if (def.second->getNodeType() == Pi) {
        possibilities = dynamic_cast<piNode*>(def.second.get())->get_variables();

    } else if (def.second->getNodeType() == Phi) {
        possibilities = dynamic_cast<phiNode*>(def.second.get())->get_variables();

    } else {
        conflicts->insert(def);
        return;
    }
    for (const auto &v : *possibilities) {
        if (interdata->statementsExecuted.find(v.var_boolname)->second) { //this statement was executed
            if (interdata->valuesFromModel.find(v.var + _run1)->second->value == val
                || interdata->valuesFromModel.find(v.var + _run2)->second->value == val)
            {
                if (visited.find(interdata->ccfg->defs[v.var]) != visited.end()) { //This block has been visited
                    findassignedconflicts(val, {interdata->ccfg->defs[v.var], interdata->ccfg->boolnameStatements[v.var_boolname]}, conflicts);
                }
            }
        }
    }

}

bool state::updateConflict(const std::shared_ptr<basicblock> &b) {
    auto res = current_values.find(conflictvar);


    //if (dynamic_cast<piNode*>(b->defsite[con])


    if (res != current_values.end()) {
        std::set<std::pair<std::shared_ptr<basicblock>, std::shared_ptr<statementNode>>> conflicts;
        auto defsite = b->defsite.find(conflictvar);
        if (defsite != b->defsite.end()) {
            findassignedconflicts(res->second.val, {b, defsite->second}, &conflicts);
        }

        if (res->second.val == valForRun1) {
            if (conflictIsCoend) {
                update_conflict(true, conflicts.begin()->second);
            }
            conflictingDefs.first = res->second.statement;
            onconflictnode = conflict1 = true;
            return true;
        } else if (res->second.val == valForRun2) {
            if (conflictIsCoend) {
                update_conflict(false, conflicts.begin()->second);
            }
            conflictingDefs.second = res->second.statement;
            onconflictnode = conflict2 = true;
            return true;
        }

        /*
        if (res->second.first == valForRun1) {
            for (const auto &blk : conflictsForRun1) {
                if (visited.find(blk) != visited.end() && blk->defsite[*blk->defines[origname].rbegin()]->getNodeType() != Pi) {
                    if (conflictIsCoend) {
                        update_conflict(true, blk->defsite[*blk->defines[origname].begin()]);
                    }
                    conflictingDefs.first = blk;
                    onconflictnode = conflict1 = true;
                    return true;
                }
            }
        } else if (res->second.first == valForRun2) {
            for (const auto &blk : conflictsForRun2) {
                if (visited.find(blk) != visited.end() && blk->defsite[*blk->defines[origname].rbegin()]->getNodeType() != Pi) {
                    if (conflictIsCoend) {
                        update_conflict(false, blk->defsite[*blk->defines[origname].begin()]);
                    }
                    conflictingDefs.second = blk;
                    onconflictnode = conflict2 = true;
                    return true;
                }
            }
        }*/
    }
    return false;
}

bool state::isConflicting(const std::shared_ptr<basicblock> &blk) {
    if (conflict1 && conflictsForRun2.find(blk) != conflictsForRun2.end()) {
        return true;
    } else if (conflict2 && conflictsForRun1.find(blk) != conflictsForRun1.end()) {
        return true;
    }
    return false;
}

std::string state::report_racecondition() {
    std::shared_ptr<statementNode> stmtUsage = conflictNode->statements.back();
    std::shared_ptr<statementNode> def1;
    std::shared_ptr<statementNode> def2;
    if (conflictIsCoend) {
        def1 = conflicts.first;
        def2 = conflicts.second;
    } else {
        def1 = conflictingDefs.first;
        def2 = conflictingDefs.second;
    }
    if (conflictIsCoend) return statesHandler::report_racecondition(def1, def2);
    else {
        std::string raceconditionStr =
                "Use of variable '" + origname + "' in statement: '" + stmtUsage->strOnSourceForm() + "' line " +
                std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
                + valForRun1 + " defined in statement: '" + def1->strOnSourceForm() + "' on line " +
                std::to_string(def1->get_linenum()) + "\n"
                "and\n"
                + valForRun2 + " defined in statement: '" + def2->strOnSourceForm() + "' on line " +
                std::to_string(def2->get_linenum()) + "\n";

        return raceconditionStr;
    }
}

void state::updateVisited(const std::shared_ptr<basicblock> &blk, const std::vector<std::shared_ptr<basicblock>> &blks) {
  currents.erase(blk);
  visited.insert(blk);
  for (const auto &nxt : blks) currents.insert(nxt);
}

bool state::updateVal(const std::shared_ptr<basicblock> &blk) {
    auto value = current_values.find(origname)->second;
    if (value.val == "0") {
        //std::cout << "here";
    }
    if (!conflictIsCoend) {
        std::set<std::pair<std::shared_ptr<basicblock>, std::shared_ptr<statementNode>>> conflicts;

        if (conflict1 && valForRun1 != value.val) {
            conflictingDefs.second = value.statement;
            valForRun2 = value.val;
        } else if (conflict2 && valForRun2 != value.val) {
            conflictingDefs.first = value.statement;
            valForRun1 = value.val;
        } else {
            return false;
        }
    } else { //This is a coend block, so blk == conflictNode. Another visit here is saved on statesHandler
        updateConflict(blk);
        return conflicts.first && conflicts.second;/* {
            return update_conflict(true, conflictingDefs.first->defsite[*conflictingDefs.first->defines[origname].begin()]);
        } else if (conflict2 && valForRun1 != val) {
            return update_conflict(false, conflictingDefs.second->defsite[*conflictingDefs.second->defines[origname].begin()]);
        } else {
            return false;
        }*/
    }
    return true;
}
