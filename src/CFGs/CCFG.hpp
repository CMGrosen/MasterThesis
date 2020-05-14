//
// Created by hu on 14/05/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFG_HPP
#define ANTLR_CPP_TUTORIAL_CCFG_HPP

#include <src/nodes/basicblock.hpp>
#include <src/nodes/statements/statementNodes.hpp>
#include <memory>
#include <map>
#include <iostream>

struct CCFG {
    std::set<std::shared_ptr<basicblock>> nodes; //All basicblocks in the graph
    std::set<std::shared_ptr<edge>> edges; //All edges in the graph, including conflict edges after updating
    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<edge>>> conflict_edges_from; //only the conflict edges. The usage maps to all definitions
    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<edge>>> conflict_edges_to;
    std::shared_ptr<basicblock> startNode;
    std::shared_ptr<basicblock> exitNode;
    std::map<std::string, std::shared_ptr<basicblock>> defs; //When the CFG is on SSA-form, find the block where a variable is defined
    std::map<basicblock *, std::set<basicblock *>> concurrent_events; //unused
    size_t readcount; //how many reads we have. Used in symengine to build constraints for all reads
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::shared_ptr<basicblock>>> prec; //unused
    std::vector<std::pair<std::shared_ptr<basicblock>, size_t>> pis_and_depth; //all pi-functions and the depth. Sorted by depth
    std::vector<std::shared_ptr<basicblock>> fiNodes; //all blocks with endFi statements
    std::vector<std::shared_ptr<basicblock>> endconcNodes; //all blocks with endConc statements
    std::map<std::string, std::shared_ptr<basicblock>> boolnameBlocks; //find the statement with the given bool tracking constant


    void updateConflictEdges() { add_conflict_edges(); };

    CCFG(std::set<std::shared_ptr<basicblock>> _nodes, std::unordered_set<edge> _edges,
         std::shared_ptr<basicblock> _start, std::shared_ptr<basicblock> _exit)
            : nodes{std::move(_nodes)}, startNode{std::move(_start)},
              exitNode{std::move(_exit)} {
        assign_parents();
        if (exitNode->parents.size() == 1 && exitNode->parents[0].lock()->statements[0]->getNodeType() != While) {
            nodes.erase(exitNode);
            _edges.erase(edge(flow, exitNode->parents[0].lock(), exitNode));
            std::vector<std::shared_ptr<basicblock>> _nexts;
            exitNode = exitNode->parents[0].lock();
            exitNode->nexts.clear();
        }

        for (auto &n : nodes) {
            if (!n->statements.empty()) {
                switch (n->statements.back()->getNodeType()) {
                    case Assign:
                    case AssignArrField:
                    case Write:
                    case Skip:
                    case Phi:
                    case Pi:
                    case Assert:
                        n->type = Compute;
                        break;
                    case Concurrent:
                        n->type = Cobegin;
                        break;
                    case EndConcurrent:
                        n->type = Coend;
                        break;
                    case Sequential:
                        break;
                    case While:
                        n->type = Loop;
                        break;
                    case If:
                    case Event:
                        n->type = Condition;
                        break;
                    default:
                        break;
                }
            }
            if (n->type == Coend) endconcNodes.push_back(n);
            else if (n->type == joinNode) fiNodes.push_back(n);
        }
        for (auto &e : _edges) {
            edges.insert(std::make_shared<edge>(e));
        }
        assign_blocknames(startNode, this);
    }

    CCFG(const CCFG& a) {
        copy_tree(a);
    }

    CCFG(CCFG&& o) noexcept
            : nodes{std::move(o.nodes)}, edges{std::move(o.edges)},
              conflict_edges_from{std::move(o.conflict_edges_from)}, conflict_edges_to{std::move(o.conflict_edges_to)},
              startNode{std::move(o.startNode)}, exitNode{std::move(o.exitNode)},
              defs{std::move(o.defs)}, concurrent_events{std::move(o.concurrent_events)}, readcount{o.readcount},
              prec{std::move(o.prec)}, pis_and_depth(std::move(o.pis_and_depth)),
              fiNodes{std::move(o.fiNodes)}, endconcNodes{std::move(o.endconcNodes)}, boolnameBlocks{std::move(o.boolnameBlocks)}
    {}

    CCFG& operator=(const CCFG& a) {
        copy_tree(a);
        return *this;
    }

    CCFG& operator=(CCFG&& other) noexcept {
        nodes = std::move(other.nodes);
        edges = std::move(other.edges);
        conflict_edges_from = std::move(other.conflict_edges_from);
        conflict_edges_to = std::move(other.conflict_edges_to);
        startNode = std::move(other.startNode);
        exitNode = std::move(other.exitNode);
        defs = std::move(other.defs);
        concurrent_events = std::move(other.concurrent_events);
        readcount = other.readcount;
        prec = std::move(other.prec);
        pis_and_depth = std::move(other.pis_and_depth);
        fiNodes = std::move(other.fiNodes);
        endconcNodes = std::move(other.endconcNodes);
        boolnameBlocks = std::move(other.boolnameBlocks);
        return *this;
    }

    ~CCFG() {
        for (auto &blk : nodes) {
            blk->nexts.clear();
        }
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
    void copy_tree(const CCFG& a) {
        readcount = a.readcount;
        auto oldMapsTo = std::map<basicblock*, std::shared_ptr<basicblock>>{};
        for (auto &n : a.nodes) {
            auto newNode = std::make_shared<basicblock>(basicblock(*n));
            if (a.startNode == n) startNode = newNode;
            else if (a.exitNode == n) exitNode = newNode;
            nodes.insert(newNode);
            oldMapsTo.insert({n.get(), newNode});
        }
        if (!a.boolnameBlocks.empty()) {
            for (const auto &p : a.boolnameBlocks) {
                boolnameBlocks.insert({p.first, oldMapsTo[p.second.get()]});
            }
        }
        for (const auto &n : a.nodes) {
            oldMapsTo[n.get()]->concurrentBlock = std::make_pair(oldMapsTo[n->concurrentBlock.first].get(), n->concurrentBlock.second);

            for (size_t i = 0; i < n->parents.size(); ++i) {
                oldMapsTo[n.get()]->parents.push_back(oldMapsTo[n->parents[i].lock().get()]);
            }

            for (const auto &next : n->nexts) {
                oldMapsTo[n.get()]->nexts.push_back(oldMapsTo[next.get()]);
            }

            if (n->type == Coend) {
                auto concnode = reinterpret_cast<endConcNode*>(oldMapsTo[n.get()]->statements.back().get());
                auto prev_parent = concnode->getConcNode();
                concnode->setConcNode(oldMapsTo[concnode->getConcNode().get()]);
                endconcNodes.push_back(oldMapsTo[n.get()]);
            } else if (n->type == joinNode) {
                auto finode = reinterpret_cast<fiNode*>(oldMapsTo[n.get()]->statements.back().get());
                std::set<std::shared_ptr<basicblock>> parents;
                for (const auto &p : *finode->get_parents()) parents.insert(oldMapsTo[p.get()]);
                finode->set_parents(std::move(parents));
                finode->first_parent = oldMapsTo[finode->first_parent.get()];
                fiNodes.push_back(oldMapsTo[n.get()]);
            }
        }
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
        if (!a.defs.empty()) {
            for (const auto &def : a.defs) {
                defs.emplace(def.first, oldMapsTo[def.second.get()]);
            }
        }
        if (!a.concurrent_events.empty()) {
            for (const auto &concEvent : a.concurrent_events) {
                auto event = oldMapsTo[concEvent.first];
                concurrent_events.insert({event.get(), {}});
                for (const auto &concwith : concEvent.second) {
                    concurrent_events.find(event.get())->second.insert(oldMapsTo[concwith].get());
                }
            }
        }
        for (const auto &p : a.prec) {
            std::pair<std::shared_ptr<basicblock>, std::unordered_set<std::shared_ptr<basicblock>>> nPrec;
            nPrec.first = oldMapsTo[p.first.get()];
            for (const auto &o : p.second) {
                nPrec.second.insert(oldMapsTo[o.get()]);
            }
            prec.insert(nPrec);
        }
        for (const auto &pair : a.pis_and_depth) {
            pis_and_depth.emplace_back(oldMapsTo[pair.first.get()], pair.second);
        }
    }

    void assign_parents() {
        for (const auto &blk : nodes) {
            blk->parents.clear();
        }
        std::set<std::shared_ptr<basicblock>> blks;
        assign_parents_helper(nullptr,startNode, &blks);
    }

    static void assign_parents_helper(const std::shared_ptr<basicblock>& parent, const std::shared_ptr<basicblock>& current, std::set<std::shared_ptr<basicblock>> *visited) {
        current->parents.push_back(parent);
        if (!current->statements.empty()) {
            if (auto t = dynamic_cast<phiNode *>(current->statements.front().get())) {
                if (t->getName() == "result_3") {
                    std::cout << "break";
                }
            }
        }
        if (visited->insert(current).second) {
            for (const auto &nxt : current->nexts) {
                assign_parents_helper(current, nxt, visited);
            }
        }
    }

    static void assign_blocknames(const std::shared_ptr<basicblock>& startnode, CCFG *ccfg) {
        std::set<std::shared_ptr<basicblock>> visited;
        int32_t name = 0;
        assign_blocknames_helper(startnode, &visited, &name, ccfg);
    }

    static void assign_blocknames_helper(const std::shared_ptr<basicblock>& node, std::set<std::shared_ptr<basicblock>> *visited, int32_t *name, CCFG *ccfg) {
        if (node->type == joinNode || node->type == Coend) {
            if (visited->find(node->parents.back().lock()) == visited->end()) {
                //Cannot name this block yet as right-most parent hasn't yet been visited
                return;
            }
        }
        if (visited->insert(node).second) { //Break cycles in case of whiles. Don't want to name the same block twice
            node->set_name(++(*name));
            ccfg->boolnameBlocks.insert({node->get_name(), node});
            for (const auto& n : node->nexts) {
                assign_blocknames_helper(n, visited, name, ccfg);
            }
        }
    }

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

    static bool is_sequential_helper(const std::shared_ptr<basicblock> &a, const std::shared_ptr<basicblock> &b, std::set<std::shared_ptr<basicblock>> *visited) {
        if (!a) return false;
        else if (a == b) return true;
        else if (visited->insert(a).second) {
            for (const auto &nxt : a->nexts) {
                if (is_sequential_helper(nxt, b, visited)) return true;
            }
            return false;
        }
        else return false;
    }

    static bool is_sequential(std::shared_ptr<basicblock> &a, std::shared_ptr<basicblock> &b) {
        std::set<std::shared_ptr<basicblock>> visited;
        bool sequential = is_sequential_helper(a,b,&visited);
        if (sequential) return true;
        visited.clear();
        return is_sequential_helper(b,a,&visited);
    }
};



#endif //ANTLR_CPP_TUTORIAL_CCFG_HPP
