//
// Created by hu on 08/04/2020.
//

#include "state.hpp"

state::state() {
    conflict1 = conflict2 = onconflictnode = found = false;
}
state::state(std::set<std::shared_ptr<basicblock>> cr1, std::set<std::shared_ptr<basicblock>> cr2,
        std::shared_ptr<basicblock> cn,
        std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> _threadsToFinish,
        std::map<std::string, std::pair<std::string, Type>> cv,
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

bool state::updateConflict(const std::shared_ptr<basicblock> &b) {
    auto res = current_values.find(conflictvar);
    /*std::set<std::shared_ptr<statementNode>> conflicts;
    auto defsite = b->defsite.find(conflictvar);
    if (defsite != b->defsite.end()) {
        if (defsite->second->getNodeType() == Pi) {
            auto pipossibilities = *dynamic_cast<piNode*>(defsite->second.get())->get_variables();
            for (const auto &v : pipossibilities) {
                if (statementsE)
            }
        } else if (defsite->second->getNodeType() == Phi) {

        } else {

        }
    }
    if (dynamic_cast<piNode*>(b->defsite[con])
    */if (res != current_values.end()) {
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
        }
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
        def1 = conflictingDefs.first->defsite[*conflictingDefs.first->defines[origname].begin()];
        def2 = conflictingDefs.second->defsite[*conflictingDefs.second->defines[origname].begin()];
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
    std::string val = current_values[origname].first;
    if (val == "0") {
        //std::cout << "here";
    }
    if (!conflictIsCoend) {
        if (conflict1 && valForRun1 != val) {
            conflictingDefs.second = blk;
            valForRun2 = val;
        } else if (conflict2 && valForRun2 != val) {
            conflictingDefs.first = blk;
            valForRun1 = val;
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
