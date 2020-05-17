//
// Created by hu on 10/04/2020.
//

#include "statesHandler.hpp"

std::set<std::shared_ptr<basicblock>> statesHandler::conflictsForRun1 = {};
std::set<std::shared_ptr<basicblock>> statesHandler::conflictsForRun2 = {};
std::shared_ptr<basicblock> statesHandler::conflictNode = {};
std::set<std::string> statesHandler::valsForRun1 = {};
std::set<std::string> statesHandler::valsForRun2 = {};
std::string statesHandler::conflictvar = {};
std::string statesHandler::origname = {};
bool statesHandler::conflictIsCoend = false;
interpreterData* statesHandler::interdata = nullptr;
std::pair<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> statesHandler::conflicts = {nullptr, nullptr};

bool statesHandler::update_conflict(bool first, const std::shared_ptr<statementNode>& stmt) {
    if (first && !conflicts.first) {
        conflicts.first = stmt;
        return true;
    } else if (!first && !conflicts.second) {
        conflicts.second = stmt;
        return true;
    }
    return false;
}

std::string statesHandler::report_racecondition(const std::shared_ptr<statementNode>& def1, const std::shared_ptr<statementNode>& def2) {
    std::shared_ptr<statementNode> stmtUsage = conflictNode->statements.back();
    std::string raceconditionStr =
            "Usages of variable '" + origname + "' following fork statement on line: " + std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
            + *valsForRun1.begin() + " defined in statement: '" + def1->strOnSourceForm() + "' on line " + std::to_string(def1->get_linenum()) + "\n"
                                                                                                                                       "and\n"
            + *valsForRun2.begin() + " defined in statement: '" + def2->strOnSourceForm() + "' on line " + std::to_string(def2->get_linenum()) + "\n";

    return raceconditionStr;
}

void statesHandler::clear() {
    conflictsForRun1.clear();
    conflictsForRun2.clear();
    conflictNode = nullptr;
    valsForRun1.clear();
    valsForRun2.clear();
    conflictvar = origname = "";
    conflicts = {};
}
