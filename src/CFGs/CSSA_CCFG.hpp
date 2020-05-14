//
// Created by hu on 14/05/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CSSA_CCFG_HPP
#define ANTLR_CPP_TUTORIAL_CSSA_CCFG_HPP

#include "SSA_CCFG.hpp"

struct CSSA_CCFG : public SSA_CCFG {
    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<edge>>> conflict_edges_from; //only the conflict edges. The usage maps to all definitions
    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<edge>>> conflict_edges_to;

    std::vector<std::pair<std::shared_ptr<basicblock>, size_t>> pis_and_depth; //all pi-functions and the depth. Sorted by depth

    void updateConflictEdges() { add_conflict_edges(); };

    CSSA_CCFG(SSA_CCFG a) : SSA_CCFG{std::move(a)} {

    };

    CSSA_CCFG(const CSSA_CCFG& a) {
        CSSA_CCFG::copy_tree(a);
    }

    CSSA_CCFG(CSSA_CCFG&& o) noexcept
    : SSA_CCFG{std::move(o)},
      conflict_edges_from{std::move(o.conflict_edges_from)}, conflict_edges_to{std::move(o.conflict_edges_to)},
      pis_and_depth{std::move(o.pis_and_depth)} {}

    CSSA_CCFG& operator=(const CSSA_CCFG& a) {
        CSSA_CCFG::copy_tree(a);
        return *this;
    }

    CSSA_CCFG& operator=(CSSA_CCFG&& other) noexcept {
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
        conflict_edges_from = std::move(other.conflict_edges_from);
        conflict_edges_to = std::move(other.conflict_edges_to);
        pis_and_depth = std::move(other.pis_and_depth);
        return *this;
    }

    static bool concurrent(const std::shared_ptr<basicblock> &a, const std::shared_ptr<basicblock> &b) {
        if (a == b) return false; //same nodes aren't concurrent
        if (!a->concurrentBlock.first || !b->concurrentBlock.first) return false; //one of them aren't concurrent, thus they aren't concurrent
        if (a->type == Coend || b->type == Coend) return false; //these aren't variable assignments, so don't want to consider them as concurrent ever
        if (a->concurrentBlock.first == b->concurrentBlock.first && a->concurrentBlock.second == b->concurrentBlock.second) {
            //If they're in the same thread, they're not concurrent
            return false;
        }
        std::unordered_map<basicblock*, int> concurrentNodesForA;
        auto concblock = a->concurrentBlock;
        do {
            concurrentNodesForA.insert(concblock);
            concblock = concblock.first->concurrentBlock;
        } while(concblock.first);

        concblock = b->concurrentBlock;
        while (concblock.first && concurrentNodesForA.find(concblock.first) == concurrentNodesForA.end()) {
            concblock = concblock.first->concurrentBlock;
        }
        if (concblock.first) {
            if (concurrentNodesForA.find(concblock.first)->second != concblock.second) {
                //We have found a common ancestor fork node, and the immediate fork statements for block a and b
                // are not within the same thread
                return true;
            }
        }
        return false; //if we get here, no conditions were met, thus not concurrent
    }

private:
    void add_conflict_edges() {
        for (auto blk : nodes) {
            if (blk->type != joinNode)
                for (auto cmp : nodes) {
                    if (cmp->type != joinNode && concurrent(blk, cmp)) {
                        for (const auto &var : blk->defines) {
                            if ((cmp->uses.find(var.first)) != cmp->uses.end()) {//|| cmp->defines.find(var.first) != cmp->defines.end()) {
                                //blocks are concurrent
                                // and have a variable in common,
                                // thus there's a conflict
                                std::shared_ptr<edge> e = std::make_shared<edge>(edge(conflict, blk, cmp));
                                edges.insert(e);
                                auto res = conflict_edges_from.insert({blk, {e}});
                                if (!res.second) res.first->second.push_back(e);
                                res = conflict_edges_to.insert({cmp, {e}});
                                if (!res.second) res.first->second.push_back(e);
                                break;
                            }
                        }
                        if (blk->statements.back()->getNodeType() == Event && cmp->statements.back()->getNodeType() == Event) {
                            auto res = concurrent_events.insert({blk.get(), {cmp.get()}});
                            if (!res.second) {
                                res.first->second.insert(cmp.get());
                            }
                        }
                    }
                }
        }
    }

protected:
    std::map<basicblock*, std::shared_ptr<basicblock>> copy_tree(const CSSA_CCFG& a) {
        auto oldMapsTo = SSA_CCFG::copy_tree(static_cast<const SSA_CCFG&>(a));
        for (const auto &ed : a.edges) {
            std::shared_ptr<edge> e = std::make_shared<edge>(edge(ed->type, oldMapsTo[ed->from().get()], oldMapsTo[ed->to().get()]));
            edges.insert(e);
            if (ed->type == conflict) {
                auto res = conflict_edges_from.insert({e->from(), {e}});
                if (!res.second) res.first->second.push_back(e);
                res = conflict_edges_to.insert({e->to(), {e}});
                if (!res.second) res.first->second.push_back(e);
            }
        }
        for (const auto &pair : a.pis_and_depth) {
            pis_and_depth.emplace_back(oldMapsTo[pair.first.get()], pair.second);
        }
        copy_edges(oldMapsTo, a);
        return oldMapsTo;
    }

    void copy_edges(std::map<basicblock*, std::shared_ptr<basicblock>> oldMapsTo, const CCFG& a) override {
        for (const auto &ed : a.edges) {
            std::shared_ptr<edge> e = std::make_shared<edge>(edge(ed->type, oldMapsTo[ed->from().get()], oldMapsTo[ed->to().get()]));
            edges.insert(e);
            if (ed->type == conflict) {
                auto res = conflict_edges_from.insert({e->from(), {e}});
                if (!res.second) res.first->second.push_back(e);
                res = conflict_edges_to.insert({e->to(), {e}});
                if (!res.second) res.first->second.push_back(e);
            }
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_CSSA_CCFG_HPP
