//
// Created by CMG on 12/11/2019.
//
#ifndef ANTLR_CPP_TUTORIAL_STATE_HPP
#define ANTLR_CPP_TUTORIAL_STATE_HPP

#include <set>
#include <map>
#include <nodes/basicblock.hpp>
#include "statesHandler.hpp"
#include "value.hpp"

struct state : public statesHandler {
    std::pair<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> conflictingDefs;
    std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> threadsToFinish;
    std::map<std::string, Value> current_values;
    bool conflict1, conflict2, onconflictnode, found;
    std::set<std::shared_ptr<basicblock>> currents;
    std::set<std::shared_ptr<basicblock>> visited;

    state();
    state(std::set<std::shared_ptr<basicblock>> cr1, std::set<std::shared_ptr<basicblock>> cr2,
          std::shared_ptr<basicblock> cn,
          std::map<std::shared_ptr<basicblock>, std::set<basicblock*>> _threadsToFinish,
          std::map<std::string, Value> cv,
          std::string v1, std::string v2, interpreterData* _interdata);

    bool updateConflict(const std::shared_ptr<basicblock>&);
    bool isConflicting(const std::shared_ptr<basicblock>&);
    void updateVisited(const std::shared_ptr<basicblock>&, const std::vector<std::shared_ptr<basicblock>>&);
    bool updateVal(const std::shared_ptr<basicblock>&);
    std::string report_racecondition();

private:
    void findassignedconflicts(const std::string &, const std::pair<std::shared_ptr<basicblock>, std::shared_ptr<statementNode>> &, std::set<std::pair<std::shared_ptr<basicblock>, std::shared_ptr<statementNode>>> *);
};



#endif //ANTLR_CPP_TUTORIAL_STATE_HPP
