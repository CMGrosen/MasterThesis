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
        std::string v1, std::string v2)
        : conflictingDefs{}, threadsToFinish{std::move(_threadsToFinish)}, current_values{std::move(cv)}, currents{}
        { conflictsForRun1 = std::move(cr1);
          conflictsForRun2 = std::move(cr2);
          conflictNode = std::move(cn);
          conflictingDefs = {};
          valForRun1 = std::move(v1);
          valForRun2 = std::move(v2);

          conflict1 = conflict2 = onconflictnode = found = false;
        }

std::set<std::shared_ptr<basicblock>> state::conflictsForRun1 = {};
std::set<std::shared_ptr<basicblock>> state::conflictsForRun2 = {};
std::shared_ptr<basicblock> state::conflictNode = {};
std::string state::valForRun1 = {};
std::string state::valForRun2 = {};
std::string state::conflictvar = {};
std::string state::origname = {};

bool state::updateConflict() {
    auto res = current_values.find(conflictvar);
    if (res != current_values.end()) {
        if (res->second.first == valForRun1) {
            for (const auto &blk : conflictsForRun1) {
                if (visited.find(blk) != visited.end() && blk->defsite[*blk->defines[origname].rbegin()]->getNodeType() != Pi) {
                    conflictingDefs.first = blk;
                    onconflictnode = conflict1 = true;
                    return true;
                }
            }
        } else if (res->second.first == valForRun2) {
            for (const auto &blk : conflictsForRun2) {
                if (visited.find(blk) != visited.end() && blk->defsite[*blk->defines[origname].rbegin()]->getNodeType() != Pi) {
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
    std::shared_ptr<statementNode> def1 = conflictingDefs.first->defsite[*conflictingDefs.first->defines[origname].begin()];
    std::shared_ptr<statementNode> def2 = conflictingDefs.second->defsite[*conflictingDefs.second->defines[origname].begin()];
    std::string raceconditionStr =
"Use of variable '" + origname + "' in statement: '" + stmtUsage->strOnSourceForm() + "' line " + std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
+ valForRun1 + " defined in statement: '" + def1->strOnSourceForm() + "' on line " + std::to_string(def1->get_linenum()) + "\n"
"and\n"
+ valForRun2 + " defined in statement: '" + def2->strOnSourceForm() + "' on line " + std::to_string(def2->get_linenum()) + "\n";

return raceconditionStr;
}

void state::updateVisited(const std::shared_ptr<basicblock> &blk, const std::vector<std::shared_ptr<basicblock>> &blks) {
  currents.erase(blk);
  visited.insert(blk);
  for (const auto &nxt : blks) currents.insert(nxt);
}

bool state::updateVal(const std::shared_ptr<basicblock> &blk) {
    std::string val = current_values[origname].first;
    if (conflict1 && valForRun1 != val) {
        conflictingDefs.second = blk;
        valForRun2 = val;
    } else if (conflict2 && valForRun2 != val) {
        conflictingDefs.first = blk;
        valForRun1 = val;
    } else {
        return false;
    }
    return true;
}
