//
// Created by hu on 23/03/2020.
//

#include "interpreter.hpp"

interpreter::interpreter(symEngine e) : engine{std::move(e)} {
    if (engine.execute()) {
        std::vector<std::pair<std::shared_ptr<basicblock>, std::string>> blks_and_names;
        refresh();
        for (const auto &dif : differences) {
            blks_and_names.push_back({engine.ccfg->defs.find(dif.first)->second, dif.first});
        }
        std::sort(blks_and_names.begin(), blks_and_names.end()
                , [&](const auto& a, const auto& b) {return a.first->depth < b.first->depth;});
        reach_potential_raceConditions(blks_and_names);
        std::cout << "here";
    }
}

bool interpreter::run() {
    return false;
}

void interpreter::update() {
    valuesFromModel = engine.getModel();


    for (const auto &val : valuesFromModel) {
        if (val.first[0] == '-' && val.first[1] == 'r') { //this is a -readVal
            auto res = variableValues.insert({val.second.first, {{val.first}, val.second.second}});
            if (!res.second) res.first->second.first.insert(val.first);
        } else {
            size_t len = val.first.size() - 5;
            std::string var = val.first.substr(0, len);

            if (*val.first.rbegin() == '-' && *(val.first.rbegin()+1) == '1') { //this value ends with run1-
                auto res = valuesFromModel.find(var + _run2);
                if (res == valuesFromModel.end()) {
                    differences.insert({var, {val.second.first, "undef", val.second.second}});
                } else if (val.second.first != res->second.first) {
                    differences.insert({var, {val.second.first, res->second.first, val.second.second}});
                }
                auto inserted = variableValues.insert({val.second.first, {{var}, val.second.second}});
                if (!inserted.second) inserted.first->second.first.insert(var);
            } else {
                auto res = valuesFromModel.find(var + _run1);
                if (res == valuesFromModel.end())
                    differences.insert({var, {"undef", val.second.first, val.second.second}});
            }
        }
    }
}

void interpreter::refresh() {
    variableValues.clear();
    valuesFromModel.clear();
    differences.clear();
    update();
}

bool interpreter::reach_potential_raceConditions(const std::vector<std::pair<std::shared_ptr<basicblock>, std::string>>& blks) {
    for (const auto &blk : blks) {

        if (reachable(blk, blk.first->defmapping.find(blk.second)->second)) return true;
    }
    return false;
}

std::pair<bool, std::vector<edge>> edges_to_take(std::shared_ptr<basicblock> current, std::shared_ptr<basicblock> conflict, const std::string& origname) {
    std::vector<edge> edges;
    if (current->depth > conflict->depth) {
        current.swap(conflict);
    }
    std::pair<bool, std::vector<edge>> res;
    for (const auto& blk : conflict->parents) {
        if (blk.lock() == current) return {true, {edge(blk.lock(), conflict)}};
        auto defs = blk.lock()->defines.find(origname);
        if (defs != blk.lock()->defines.end()) {
            for (const auto &def : defs->second) {
                if (def.front() != '-') { //overwriting this conflict. skipping.
                    return {false, {}};
                }
            }
        }
        auto inter = edges_to_take(current, blk.lock(), origname);
        if (inter.first) {
            inter.second.emplace_back(blk.lock(), conflict);
            if (edges.empty() || inter.second.size() < edges.size()) {
                edges = std::move(inter.second);
            }
        }
    }
    if (edges.empty()) return {false, {}};
    else return {true, edges};
    /*while (blk) {
        edges.push_back(edge)
    }*/
}

bool interpreter::reachable(const std::pair<std::shared_ptr<basicblock>, std::string> &blk, const std::string& origname) {
    std::set<std::shared_ptr<basicblock>> conflictsForRun1;
    std::set<std::shared_ptr<basicblock>> conflictsForRun2;
    std::string valForRun1 = std::get<0>(differences.find(blk.second)->second);
    std::string valForRun2 = std::get<1>(differences.find(blk.second)->second);

    for (const auto &value : variableValues.find(valForRun1)->second.first) {
        conflictsForRun1.insert(engine.ccfg->defs.find(value)->second);
    }
    for (const auto &value : variableValues.find(valForRun2)->second.first) {
        conflictsForRun2.insert(engine.ccfg->defs.find(value)->second);
    }

    std::shared_ptr<basicblock> current = engine.ccfg->startNode;

    while (current) {
        if (conflictsForRun1.find(current) != conflictsForRun1.end()) {

        } else if (conflictsForRun2.find(current) != conflictsForRun2.end()) {
            auto path = edges_to_take(current, blk.first, origname);
            std::cout << "here";
        } else {

        }
        current = nullptr;
    }
    std::cout << "here";
}
