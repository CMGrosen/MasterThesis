//
// Created by hu on 23/03/2020.
//

#include "interpreter.hpp"

interpreter::interpreter(symEngine e) : engine{std::move(e)} {
    if (engine.execute()) {
        std::vector<std::shared_ptr<basicblock>> blks;
        update();
        for (const auto &dif : differences) {
            blks.push_back(engine.ccfg->defs.find(dif.first)->second);
        }

        std::cout << "here";
    }
}

bool interpreter::run() {
    return false;
}

void interpreter::update() {
    valuesFromModel = engine.getModel();

    for (const auto &val : valuesFromModel) {
        if (val.first.find("-readVal") == std::string::npos) {
            size_t len = val.first.size() - 4;
            std::string var = val.first.substr(0, len);

            if (val.first.find("run1") != std::string::npos) {
                auto res = valuesFromModel.find(var + "run2");
                if (res == valuesFromModel.end()) {
                    differences.insert({var, {val.second.first, "undef", val.second.second}});
                } else if (val.second.first != res->second.first) {
                    differences.insert({var, {val.second.first, res->second.first, val.second.second}});
                }
            } else {
                auto res = valuesFromModel.find(var + "run1");
                if (res == valuesFromModel.end())
                    differences.insert({var, {"undef", val.second.first, val.second.second}});
            }
        }
    }
}