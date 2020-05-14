//
// Created by hu on 10/04/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_STATESHANDLER_HPP
#define ANTLR_CPP_TUTORIAL_STATESHANDLER_HPP

#include <set>
#include <memory>
#include <string>
#include <src/nodes/basicblock.hpp>
#include "intepreterData.hpp"

struct statesHandler {
    static interpreterData* interdata;
    static std::set<std::shared_ptr<basicblock>> conflictsForRun1, conflictsForRun2;
    static std::shared_ptr<basicblock> conflictNode;
    static bool conflictIsCoend;
    static std::set<std::string> valsForRun1;
    static std::set<std::string> valsForRun2;
    static std::string conflictvar;
    static std::string origname;
    static std::pair<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> conflicts;

    bool static update_conflict(bool first, const std::shared_ptr<statementNode>&);

    std::string static report_racecondition(const std::shared_ptr<statementNode>& def1, const std::shared_ptr<statementNode>& def2);
};


#endif //ANTLR_CPP_TUTORIAL_STATESHANDLER_HPP
