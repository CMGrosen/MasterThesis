//
// Created by hu on 10/04/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_STATESHANDLER_HPP
#define ANTLR_CPP_TUTORIAL_STATESHANDLER_HPP

#include <set>
#include <memory>
#include <string>
#include <nodes/basicblock.hpp>
#include <basicblockTreeConstructor.hpp>

struct statesHandler {
    static CCFG* ccfg;
    static std::set<std::shared_ptr<basicblock>> conflictsForRun1, conflictsForRun2;
    static std::shared_ptr<basicblock> conflictNode;
    static bool conflictIsCoend;
    static std::string valForRun1;
    static std::string valForRun2;
    static std::string conflictvar;
    static std::string origname;
    static std::pair<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> conflicts;

    void static update_conflict(bool first, const std::shared_ptr<statementNode>&);

    std::string static report_racecondition(const std::shared_ptr<statementNode>& def1, const std::shared_ptr<statementNode>& def2) {
        std::shared_ptr<statementNode> stmtUsage = conflictNode->statements.back();
        std::string raceconditionStr =
                "Use of variable '" + origname + "' in statement: '" + stmtUsage->strOnSourceForm() + "' line " + std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
                + valForRun1 + " defined in statement: '" + def1->strOnSourceForm() + "' on line " + std::to_string(def1->get_linenum()) + "\n"
                                                                                                                                           "and\n"
                + valForRun2 + " defined in statement: '" + def2->strOnSourceForm() + "' on line " + std::to_string(def2->get_linenum()) + "\n";

        return raceconditionStr;
    }
};


#endif //ANTLR_CPP_TUTORIAL_STATESHANDLER_HPP
