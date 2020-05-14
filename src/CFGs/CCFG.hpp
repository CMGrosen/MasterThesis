//
// Created by hu on 14/05/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFG_HPP
#define ANTLR_CPP_TUTORIAL_CCFG_HPP

#include <src/nodes/basicblock.hpp>
#include <string>
#include <memory>
#include <map>
#include <iostream>

struct CCFG {
    std::set<std::shared_ptr<basicblock>> nodes; //All basicblocks in the graph
    std::set<std::shared_ptr<edge>> edges; //All edges in the graph, including conflict edges after updating
    std::shared_ptr<basicblock> startNode;
    std::shared_ptr<basicblock> exitNode;
    std::map<basicblock *, std::set<basicblock *>> concurrent_events; //unused
    size_t readcount; //how many reads we have. Used in symengine to build constraints for all reads
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::shared_ptr<basicblock>>> prec; //unused
    std::vector<std::shared_ptr<basicblock>> fiNodes; //all blocks with endFi statements
    std::vector<std::shared_ptr<basicblock>> endconcNodes; //all blocks with endConc statements
    std::map<std::string, std::shared_ptr<basicblock>> boolnameBlocks; //find the statement with the given bool tracking constant

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
        copy_edges(copy_tree(a), a);
    }

    CCFG(CCFG&& o) noexcept
            : nodes{std::move(o.nodes)}, edges{std::move(o.edges)},
              startNode{std::move(o.startNode)}, exitNode{std::move(o.exitNode)},
              concurrent_events{std::move(o.concurrent_events)}, readcount{o.readcount},
              prec{std::move(o.prec)},
              fiNodes{std::move(o.fiNodes)}, endconcNodes{std::move(o.endconcNodes)}, boolnameBlocks{std::move(o.boolnameBlocks)}
    {}

    CCFG& operator=(const CCFG& a) {
        copy_edges(copy_tree(a), a);
        return *this;
    }

    CCFG& operator=(CCFG&& other) noexcept {
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
        return *this;
    }

    ~CCFG() {
        for (auto &blk : nodes) {
            blk->nexts.clear();
        }
    }

private:
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

protected:
    CCFG() {}

    std::map<basicblock*, std::shared_ptr<basicblock>> copy_tree(const CCFG& a) {
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
        if (!a.concurrent_events.empty()) {
            for (const auto &concEvent : a.concurrent_events) {
                auto event = oldMapsTo[concEvent.first];
                concurrent_events.insert({event.get(), {}});
                for (const auto &concwith : concEvent.second) {
                    concurrent_events.find(event.get())->second.insert(oldMapsTo[concwith].get());
                }
            }
        }
        return oldMapsTo;
    }

    virtual void copy_edges(std::map<basicblock*, std::shared_ptr<basicblock>> oldMapsTo, const CCFG& a) {
        for (const auto &ed : a.edges) {
            std::shared_ptr<edge> e = std::make_shared<edge>(edge(ed->type, oldMapsTo[ed->from().get()], oldMapsTo[ed->to().get()]));
            edges.insert(e);
            /*if (ed->type == conflict) {
                auto res = conflict_edges_from.insert({e->from(), {e}});
                if (!res.second) res.first->second.push_back(e);
                res = conflict_edges_to.insert({e->to(), {e}});
                if (!res.second) res.first->second.push_back(e);
            }*/
        }
    }
};



#endif //ANTLR_CPP_TUTORIAL_CCFG_HPP
