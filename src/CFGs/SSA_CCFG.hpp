//
// Created by hu on 14/05/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
#define ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP

#include "CCFG.hpp"

struct SSA_CCFG : public CCFG {
    std::map<std::string, std::shared_ptr<basicblock>> defs; //When the CFG is on SSA-form, find the block where a variable is defined


    SSA_CCFG(CCFG a) : CCFG{std::move(a)} {

    }

    SSA_CCFG(const SSA_CCFG& a) {
        copy_edges(SSA_CCFG::copy_tree(a), a);
    }

    SSA_CCFG(SSA_CCFG&& o) noexcept
    : CCFG{std::move(o)}, defs{std::move(o.defs)} {}

    SSA_CCFG& operator=(const SSA_CCFG& a) {
        copy_edges(SSA_CCFG::copy_tree(a), a);
        return *this;
    }

    SSA_CCFG& operator=(SSA_CCFG&& other) noexcept {
        nodes = std::move(other.nodes);
        edges = std::move(other.edges);
        startNode = std::move(other.startNode);
        exitNode = std::move(other.exitNode);
        concurrent_events = std::move(other.concurrent_events);
        readcount = other.readcount;
        prec = std::move(other.prec);
        fiNodes = std::move(other.fiNodes);
        endconcNodes = std::move(other.endconcNodes);
        boolnameBlocks = std::move(other.boolnameBlocks);
        defs = std::move(other.defs);
        return *this;
    }

    void update_defs() {
        for (const auto &blk : nodes) {
            for (const auto &def : blk->defines) {
                std::string largest = *def.second.begin();
                int num = get_num(largest);
                for (const auto &item : def.second) {
                    if (num < get_num(item)) {
                        num = get_num(item);
                        largest = item;
                    }
                }
                defs.emplace(largest, blk);
            }
        }
    }

private:
    static int get_num(std::string var) {
        int pos = 1;
        int res = 0;
        for (auto it = var.rbegin(); it != var.rend(); ++it) {
            if (*it == '_') {
                return res;
            } else {
                res += (pos * (*it - '0'));
            }
        }
        return 0;
    }

protected:
    SSA_CCFG() {}

    std::map<basicblock*, std::shared_ptr<basicblock>> copy_tree(const SSA_CCFG& a) {
        auto oldMapsTo = CCFG::copy_tree(static_cast<const CCFG&>(a));
        if (!a.defs.empty()) {
            for (const auto &def : a.defs) {
                defs.emplace(def.first, oldMapsTo[def.second.get()]);
            }
        }
        return oldMapsTo;
    }
};

#endif //ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
