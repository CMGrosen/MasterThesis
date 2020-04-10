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
