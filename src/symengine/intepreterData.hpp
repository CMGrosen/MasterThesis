//
// Created by hu on 15/04/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_INTEPRETERDATA_HPP
#define ANTLR_CPP_TUTORIAL_INTEPRETERDATA_HPP

#include <set>
#include <src/CFGs/CSSA_CCFG.hpp>

struct runInformation {
    std::unique_ptr<CSSA_CCFG> ccfg;
    std::set<std::string> interleavingsTaken;
    //std::map<std::string, std::vector<std::shared_ptr<edge>>> interleavingsTakenFrom;
    std::set<std::string> blocksVisited;
    std::map<std::string, int16_t> symbolicInputs;
    std::map<std::string, literalNode> variableValues;
    std::map<std::string, std::string> varValuesDuringExecution;
    std::map<std::string, std::shared_ptr<statementNode>> lastAssignmentToKey;
    std::map<std::shared_ptr<basicblock>, std::unordered_set<basicblock*>> threads_to_finish;
    std::unordered_set<std::shared_ptr<basicblock>> frontier;
};

struct interpreterData {
    runInformation run1;
    runInformation run2;

    void clear() {
        clear(run1);
        clear(run2);
    }

private:
    void clear(runInformation &run) {
        run.variableValues.clear();
        run.varValuesDuringExecution.clear();
        run.symbolicInputs.clear();
        run.blocksVisited.clear();
        run.interleavingsTaken.clear();
        run.lastAssignmentToKey.clear();
        run.threads_to_finish.clear();
        run.frontier.clear();
        run.ccfg = nullptr;
    }
};

#endif //ANTLR_CPP_TUTORIAL_INTEPRETERDATA_HPP
