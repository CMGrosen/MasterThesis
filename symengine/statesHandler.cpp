//
// Created by hu on 10/04/2020.
//

#include "statesHandler.hpp"

std::set<std::shared_ptr<basicblock>> statesHandler::conflictsForRun1 = {};
std::set<std::shared_ptr<basicblock>> statesHandler::conflictsForRun2 = {};
std::shared_ptr<basicblock> statesHandler::conflictNode = {};
std::string statesHandler::valForRun1 = {};
std::string statesHandler::valForRun2 = {};
std::string statesHandler::conflictvar = {};
std::string statesHandler::origname = {};
bool statesHandler::conflictIsCoend = false;
CCFG* statesHandler::ccfg = nullptr;
std::pair<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> statesHandler::conflicts = {nullptr, nullptr};

void statesHandler::update_conflict(bool first, const std::shared_ptr<statementNode>& stmt) {
    if (first && !conflicts.first) conflicts.first = stmt;
    else if (!first && !conflicts.second) conflicts.second = stmt;
}

std::string statesHandler::report_racecondition(const std::shared_ptr<statementNode>& def1, const std::shared_ptr<statementNode>& def2) {
    std::shared_ptr<statementNode> stmtUsage = conflictNode->statements.back();
    std::string raceconditionStr =
            "Usages of variable '" + origname + "' following fork statement on line: " + std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
            + valForRun1 + " defined in statement: '" + def1->strOnSourceForm() + "' on line " + std::to_string(def1->get_linenum()) + "\n"
                                                                                                                                       "and\n"
            + valForRun2 + " defined in statement: '" + def2->strOnSourceForm() + "' on line " + std::to_string(def2->get_linenum()) + "\n";

    return raceconditionStr;
}