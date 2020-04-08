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

bool state::onconflict(const std::shared_ptr<basicblock>& blk) {
    if (conflictsForRun1.find(blk) != conflictsForRun1.end()) {
        conflictingDefs.first = blk;
        //path = edges_to_take(current, blk.first, origname);
        conflict1 = true;
        onconflictnode = true;
        return true;
    } else if (conflictsForRun2.find(blk) != conflictsForRun2.end()) {
        conflictingDefs.second = blk;
        //path = edges_to_take(current, blk.first, origname);
        conflict2 = true;
        onconflictnode = true;
        return true;
    }
    return false;
}

std::set<std::shared_ptr<basicblock>> state::conflictsForRun1 = {};
std::set<std::shared_ptr<basicblock>> state::conflictsForRun2 = {};
std::shared_ptr<basicblock> state::conflictNode = {};
std::string state::valForRun1 = {};
std::string state::valForRun2 = {};
std::string state::origname = {};