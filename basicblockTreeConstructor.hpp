//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP

#include <nodes/basicblock.hpp>
#include <unordered_set>
#include <list>
#include <queue>
#include <unordered_map>
#include <iostream>

struct CCFG {
    std::set<std::shared_ptr<basicblock>> nodes;
    std::unordered_set<edge> edges;
    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<basicblock>>> conflict_edges;
    std::shared_ptr<basicblock> startNode;
    std::shared_ptr<basicblock> exitNode;
    std::map<std::string, std::shared_ptr<basicblock>> defs;
    std::map<basicblock *, std::set<basicblock *>> concurrent_events;
    size_t readcount;
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::shared_ptr<basicblock>>> prec;
    std::vector<std::pair<std::shared_ptr<basicblock>, size_t>> pis_and_depth;
    std::vector<std::shared_ptr<basicblock>> fiNodes;
    std::vector<std::shared_ptr<basicblock>> endconcNodes;
    std::map<std::string, std::shared_ptr<statementNode>> boolnameStatements;


    void updateConflictEdges() { add_conflict_edges(); };

    CCFG(std::set<std::shared_ptr<basicblock>> _nodes, std::unordered_set<edge> _edges,
         std::shared_ptr<basicblock> _start, std::shared_ptr<basicblock> _exit)
            : nodes{std::move(_nodes)}, edges{std::move(_edges)}, startNode{std::move(_start)},
              exitNode{std::move(_exit)} {
        assign_parents();
        if (exitNode->parents.size() == 1 && exitNode->parents[0].lock()->statements[0]->getNodeType() != While) {
            nodes.erase(exitNode);
            edges.erase(edge(flow, exitNode->parents[0].lock(), exitNode));
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
        build_partial_order_execution();
    }

    CCFG(const CCFG& a) {
        copy_tree(a);
    }

    CCFG(CCFG&& o) noexcept
            : nodes{std::move(o.nodes)}, edges{std::move(o.edges)}, conflict_edges{std::move(o.conflict_edges)},
            startNode{std::move(o.startNode)}, exitNode{std::move(o.exitNode)},
            defs{std::move(o.defs)}, concurrent_events{std::move(o.concurrent_events)}, readcount{o.readcount},
            prec{std::move(o.prec)}, pis_and_depth(std::move(o.pis_and_depth)),
            fiNodes{std::move(o.fiNodes)}, endconcNodes{std::move(o.endconcNodes)}, boolnameStatements{std::move(o.boolnameStatements)}
    {}

    CCFG& operator=(const CCFG& a) {
        copy_tree(a);
        return *this;
    }

    CCFG& operator=(CCFG&& other) noexcept {
        nodes = std::move(other.nodes);
        edges = std::move(other.edges);
        conflict_edges = std::move(other.conflict_edges);
        startNode = std::move(other.startNode);
        exitNode = std::move(other.exitNode);
        defs = std::move(other.defs);
        concurrent_events = std::move(other.concurrent_events);
        readcount = other.readcount;
        prec = std::move(other.prec);
        pis_and_depth = std::move(other.pis_and_depth);
        fiNodes = std::move(other.fiNodes);
        endconcNodes = std::move(other.endconcNodes);
        boolnameStatements = std::move(other.boolnameStatements);
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
            if (!a.boolnameStatements.empty())
                for (const auto &stmt : newNode->statements)
                    boolnameStatements.insert({stmt->get_boolname(), stmt});
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
                auto concnode = dynamic_cast<endConcNode*>(oldMapsTo[n.get()]->statements.back().get());
                auto prev_parent = concnode->getConcNode();
                concnode->setConcNode(oldMapsTo[concnode->getConcNode().get()]);
                endconcNodes.push_back(oldMapsTo[n.get()]);
            } else if (n->type == joinNode) {
                auto finode = dynamic_cast<fiNode*>(oldMapsTo[n.get()]->statements.back().get());
                std::set<std::shared_ptr<basicblock>> parents;
                for (const auto &p : *finode->get_parents()) parents.insert(oldMapsTo[p.get()]);
                finode->set_parents(std::move(parents));
                fiNodes.push_back(oldMapsTo[n.get()]);
            }
        }
        for (const auto &ed : a.edges) {
            edges.insert(edge(ed.type, oldMapsTo[ed.neighbours[0].get()], oldMapsTo[ed.neighbours[1].get()]));
            if (ed.type == conflict) {
                auto res = conflict_edges.insert({oldMapsTo[ed.neighbours[0].get()], {oldMapsTo[ed.neighbours[1].get()]}});
                if (!res.second) res.first->second.push_back(oldMapsTo[ed.neighbours[1].get()]);
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
                            edges.insert(edge(conflict, blk, cmp));
                            auto res = conflict_edges.insert({blk, {cmp}});
                            if (!res.second) res.first->second.push_back(cmp);
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

    static bool concurrent(std::shared_ptr<basicblock> &a, std::shared_ptr<basicblock> &b) {
        if (a == b) return false; //same nodes aren't concurrent
        if (a->concurrentBlock.second == b->concurrentBlock.second) { //If they're in the same thread, they're not concurrent
            return false;
        }
        std::vector<basicblock*> concurrentNodesForA;
        basicblock *tmp = a->concurrentBlock.first;
        while (tmp) {
            concurrentNodesForA.push_back(tmp);
            tmp = tmp->concurrentBlock.first;
        }
        tmp = b->concurrentBlock.first;
        while (tmp) {
            for (const auto &n : concurrentNodesForA) {
                /*if(tmp->concurrentBlock.second == n->concurrentBlock.second) {
                    return false;
                }*/
                if (tmp == n && !(a->type == Coend || b->type == Coend)) {
                    // common fork ancestor between nodes, making them concurrent.
                    // Doesn't work for nested forks
                    return (!is_sequential(a, b));
                }
            }
            tmp = tmp->concurrentBlock.first;
        }
        return false; //if we get here, no conditions were met, thus not concurrent
    }

    void build_partial_order_execution() {
        std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<edge>> E;
        for (const auto &n : nodes) prec.insert({n, {}});

        for (const auto &ed : edges) {
            auto res = E.insert({ed.neighbours[0], {ed}});
            if (!res.second) res.first->second.insert(ed);
            res = E.insert({ed.neighbours[1], {ed}});
            if (!res.second) res.first->second.insert(ed);
        }

        std::queue<std::shared_ptr<basicblock>> Q;
        for (const auto &nxt : startNode->nexts) Q.push(nxt);

        while (!Q.empty()) {
            std::shared_ptr<basicblock> n = Q.front();
            Q.pop();
            auto prec_old = prec.find(n)->second;
            std::unordered_set<std::shared_ptr<basicblock>> prec_f;

            if (n->type == Coend) { // union(m,n) in E prec(m) union n
                /*
                for (const auto &ed : E.find(n)->second) {
                    if (ed.neighbours[0] != n) {
                        for (const auto &m : prec.find(ed.neighbours[0])->second) {
                            prec_f.insert(m);
                        }
                    }
                }*/
                for (const auto &m : nodes) {
                    if (m != n) prec_f.insert(m);
                }
                prec_f.insert(n);
            } else { //disjunction(m,n) in E prec(m) union n
                for (const auto &ed : E.find(n)->second) {
                    //if (ed.type == flow) {
                        auto set = ed.neighbours[0] == n ? prec.find(ed.neighbours[1])->second : prec.find(
                                ed.neighbours[0])->second;
                        for (const auto &res : set) prec_f.insert(res);
                    //}
                }
                prec_f.insert(n);
            }
            prec.find(n)->second = prec_f;

            if (prec_old != prec_f) {
                for (const auto &nxt : n->nexts) {
                    Q.push(nxt);
                }
            }
        }
    }

};

class basicBlockTreeConstructor {
public:
    basicBlockTreeConstructor() = default;

    static CCFG get_ccfg(const std::shared_ptr<statementNode> &startTree) {
        std::shared_ptr<basicblock> exitNode = std::make_shared<basicblock>(basicblock());
        const std::shared_ptr<basicblock> startNode = get_tree(startTree, exitNode, false);

        auto tmpSet = std::set<basicblock *>();
        auto res = get_all_blocks_and_edges(startNode, exitNode, &tmpSet);

        //tmpSet;
        std::unordered_set<edge> edges{!res.second.empty() ? res.second[0] : edge(startNode, std::make_shared<basicblock>(basicblock()))};
        for(const auto &it : res.second) edges.insert(it);

        tmpSet = std::set<basicblock *>();
        /*for(const auto &it : res.first) //add threadnum to blocks. Don't want to add to children already visited
            if (!it->statements.empty() && it->statements[0]->getNodeType() == Concurrent && !it->concurrentBlock.first)
                it->setConcurrentBlock(it, 0, &tmpSet);
        */
        startNode->setConcurrentBlock(nullptr, 0, &tmpSet);

        std::vector<std::shared_ptr<basicblock>> blocksToAdd;
        std::vector<edge> edgesToAdd;

        tmpSet = std::set<basicblock *>();
        for(const auto &it : res.first) {
            if (it->concurrentBlock.first && it->statements.size() > 1) {
                std::list<std::shared_ptr<statementNode>> stmts;
                for(const auto &stmt : it->statements) {
                    stmts.push_back(stmt);
                }
                stmts.pop_front();
                auto blk = split_up_concurrent_basicblocks(&blocksToAdd, &edgesToAdd, it->nexts, {it, stmts}, it->concurrentBlock, &tmpSet);
                edgesToAdd.emplace_back(edge(it, blk));
                for (const auto &nxt : it->nexts)
                    edges.erase(edge(it, nxt));
                it->nexts = std::vector<std::shared_ptr<basicblock>>{blk};
                it->statements.resize(1);
            }
        }

        for (const auto &blk : blocksToAdd) res.first.insert(blk);
        for (const auto &ed : edgesToAdd) edges.insert(ed);

        for (const auto &blk : res.first) blk->updateUsedVariables();
        //std::cout << "hej\n";

        return CCFG(std::move(res.first), std::move(edges), startNode, exitNode);
    }

    static std::shared_ptr<basicblock> get_tree(const std::shared_ptr<statementNode> &startTree, std::shared_ptr<basicblock> nxt, bool in_if) {
        std::shared_ptr<basicblock> block;
        switch (startTree->getNodeType()) {
            case Assign:
            case AssignArrField:
            case Assert:
            case Write:
            case Event:
            case Skip:
                block = std::make_shared<basicblock>(basicblock(startTree, nxt));
                block->type = Compute;
                break;
            case Concurrent: {
                auto conNode = dynamic_cast<concurrentNode *>(startTree.get());
                std::vector<std::shared_ptr<basicblock>> threads;
                //auto concNode = std::make_shared<concurrentNode>(concurrentNode(okType, threads));
                block = std::make_shared<basicblock>(basicblock(startTree));
                block->type = Cobegin;
                //std::vector<std::shared_ptr<basicblock>> inters;
                //inters.reserve(conNode->threads.size());

                auto endConc = std::make_shared<basicblock>(
                        basicblock(std::make_shared<endConcNode>(endConcNode(conNode->threads.size(), block, conNode->get_linenum())), nxt));
                endConc->type = Coend;
                //for (auto i = 0; i < inters.capacity(); ++i) {
                //    std::shared_ptr<statementNode> stmt = std::make_shared<skipNode>(skipNode());
                //    inters.push_back(std::make_shared<basicblock>(basicblock(stmt, endConc)));
                //}

                //int i = 0;
                for (const auto &t : conNode->threads) {
                    threads.push_back(get_tree(t, endConc, in_if));
                }

                //for (auto blk : threads) concNode->threads.push_back(blk);
                block->nexts = std::move(threads);
                break;
            }
            case Sequential: {
                auto seqNode = dynamic_cast<sequentialNode*>(startTree.get());
                auto rest = get_tree(seqNode->getNext(), nxt, in_if);
                block = get_tree(seqNode->getBody(), rest, in_if);
                NodeType nType = seqNode->getBody()->getNodeType();
                if (nType == Assign || nType == AssignArrField || nType == Write || nType == Skip || nType == Assert) {
                    std::vector<std::shared_ptr<statementNode>> remainingStmts;
                    size_t stmtsRemoved = 0;
                    for (const auto &stmt : rest->statements) {
                        nType = stmt->getNodeType();
                        if (nType == Assign || nType == AssignArrField || nType == Write ||
                            nType == Skip || nType == Assert) {
                            block->statements.push_back(stmt);
                            stmtsRemoved++;
                        } else {
                            break;
                        }
                    }
                    if (stmtsRemoved == rest->statements.size()) {
                        block->nexts = rest->nexts;
                    } else if (stmtsRemoved > 0) {
                        for (auto i = stmtsRemoved; i < rest->statements.size(); ++i)
                            remainingStmts.push_back(rest->statements[i]);
                        rest->statements = remainingStmts;
                    }
                }
                break;
            }
            case While: {
                auto wNode = dynamic_cast<whileNode*>(startTree.get());
                block = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{startTree}, std::vector<std::shared_ptr<basicblock>>{nullptr, nxt}));
                block->type = Loop;
                block->nexts[0] = get_tree(wNode->getBody(), block, in_if);
                break;
            }
            case If: {
                auto ifNode = dynamic_cast<ifElseNode*>(startTree.get());
                if (nxt->type == Loop) {
                    nxt = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{}, nxt));
                }
                std::shared_ptr<basicblock> joinnode;
                std::shared_ptr<basicblock> trueBranch;
                std::shared_ptr<basicblock> falseBranch;

                if (in_if && nxt->type == joinNode) {
                    joinnode = nxt;
                } else {
                    in_if = true;
                    std::shared_ptr<statementNode> stmt = std::make_shared<fiNode>(fiNode(nullptr));
                    joinnode = std::make_shared<basicblock>(basicblock(stmt, nxt));
                    joinnode->type = joinNode;
                }
                trueBranch = get_tree(ifNode->getTrueBranch(), joinnode, in_if);
                falseBranch = get_tree(ifNode->getFalseBranch(), joinnode, in_if);
                auto nxts = std::vector<std::shared_ptr<basicblock>>{trueBranch, falseBranch};
                block = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{startTree}, nxts));
                block->type = Condition;
                dynamic_cast<fiNode*>(joinnode->statements.back().get())->add_parent(block);

                break;
            }
            default:
                break;
        }
        return block;
    }

private:
    static std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>> get_all_blocks_and_edges(const std::shared_ptr<basicblock> &startTree, const std::shared_ptr<basicblock> &exitNode, std::set<basicblock *> *whileLoops) {
        std::set<std::shared_ptr<basicblock>> basicblocks;
        std::vector<edge> edges;
        if (!startTree) return std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>>{basicblocks, edges};
        //If this block has already been visited (while loop), then stop recursion
        if (!startTree->statements.empty() && startTree->statements[0]->getNodeType() == While && !whileLoops->insert(startTree.get()).second) return std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>>{basicblocks, edges};

        if (startTree != exitNode) {
            basicblocks.insert(startTree);
            for(const auto &nxt : startTree->nexts) {
                edges.emplace_back(edge(startTree, nxt));
            }

            for (const auto &nxt : startTree->nexts) {
                std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>> res;
                res = get_all_blocks_and_edges(nxt, exitNode, whileLoops);
                for (const auto &it : res.first) {
                    basicblocks.insert(it);
                }
                for (const auto &it : res.second) {
                    edges.push_back(it);
                }
            }
            return std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>>{basicblocks, edges};
        } else {
            basicblocks.insert(startTree);
            return std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>>{basicblocks, edges};
        }
    }

    static std::shared_ptr<basicblock> split_up_concurrent_basicblocks(std::vector<std::shared_ptr<basicblock>> *blocksToAdd, std::vector<edge> *edgesToAdd, std::vector<std::shared_ptr<basicblock>> &it, std::pair<std::shared_ptr<basicblock>, std::list<std::shared_ptr<statementNode>>> blockAndstmts, std::pair<basicblock*, int> conBlock, std::set<basicblock *> *whileloops) {
        if (blockAndstmts.second.size() == 1) {
            std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(blockAndstmts.second.front()));
            blk->nexts = it;
            for (const auto &ed : it) edgesToAdd->push_back(edge(blk, ed));
            blocksToAdd->push_back(blk);
            blk->setConcurrentBlock(conBlock.first, conBlock.second, whileloops);
            return blk;
        } else {
            auto firstStmt = blockAndstmts.second.front();
            blockAndstmts.second.pop_front();
            auto nxt = split_up_concurrent_basicblocks(blocksToAdd, edgesToAdd, it, blockAndstmts, conBlock, whileloops);
            std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(firstStmt, nxt));
            edgesToAdd->push_back(edge(blk, nxt));
            blocksToAdd->push_back(blk);
            blk->setConcurrentBlock(conBlock.first, conBlock.second, whileloops);
            return blk;
        }
    }
};
#endif //ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
