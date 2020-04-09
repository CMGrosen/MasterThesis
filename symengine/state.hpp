//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_STATE_HPP
#define ANTLR_CPP_TUTORIAL_STATE_HPP

#include <set>
#include <map>
#include <nodes/basicblock.hpp>

struct state {
    static std::set<std::shared_ptr<basicblock>> conflictsForRun1, conflictsForRun2;
    static std::shared_ptr<basicblock> conflictNode;
    std::pair<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> conflictingDefs;
    std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> threadsToFinish;
    std::map<std::string, std::pair<std::string, Type>> current_values;
    bool conflict1, conflict2, onconflictnode, found;
    static std::string valForRun1;
    static std::string valForRun2;
    std::set<std::shared_ptr<basicblock>> currents;
    std::set<std::shared_ptr<basicblock>> visited;

    state();
    state(std::set<std::shared_ptr<basicblock>> cr1, std::set<std::shared_ptr<basicblock>> cr2,
          std::shared_ptr<basicblock> cn,
          std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> _threadsToFinish,
          std::map<std::string, std::pair<std::string, Type>> cv,
          std::string v1, std::string v2);

    bool updateConflict();
    bool isConflicting(const std::shared_ptr<basicblock>&);
    static std::string conflictvar;
    static std::string origname;
    void updateVisited(const std::shared_ptr<basicblock>&, const std::vector<std::shared_ptr<basicblock>>&);
    bool updateVal(const std::shared_ptr<basicblock>&);
    std::string report_racecondition();
};



#endif //ANTLR_CPP_TUTORIAL_STATE_HPP
