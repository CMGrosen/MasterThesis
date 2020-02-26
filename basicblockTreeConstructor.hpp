//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP

#include <nodes/basicblock.hpp>
#include <unordered_set>
#include <list>
#include <unordered_map>
#include <iostream>

struct CCFG {
    std::set<std::shared_ptr<basicblock>> nodes;
    std::unordered_set<edge> edges;
    std::shared_ptr<basicblock> startNode;
    std::shared_ptr<basicblock> exitNode;
    void updateConflictEdges() {add_conflict_edges();};
    CCFG(std::set<std::shared_ptr<basicblock>> _nodes, std::unordered_set<edge> _edges, std::shared_ptr<basicblock> _start, std::shared_ptr<basicblock> _exit)
        : nodes{std::move(_nodes)}, edges{std::move(_edges)}, startNode{std::move(_start)}, exitNode{std::move(_exit)} {
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
                if (n == startNode) n->type = Entry;
                else if (n == exitNode) n->type = Exit;
                else switch (n->statements[0]->getNodeType()) {
                    case Assign:
                    case AssignArrField:
                    case Write:
                    case Skip:
                    case Phi:
                    case Pi:
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
        }
    }

    CCFG(const CCFG& a) {
        copy_tree(a);
    }

    CCFG(CCFG&& o) noexcept
            : nodes{std::move(o.nodes)}, edges{std::move(o.edges)}, startNode{std::move(o.startNode)}, exitNode{std::move(o.exitNode)} {
    }

    CCFG& operator=(const CCFG& a) {
        copy_tree(a);
        return *this;
    }

    CCFG& operator=(CCFG&& other) noexcept {
        nodes = std::move(other.nodes);
        edges = std::move(other.edges);
        startNode = std::move(other.startNode);
        exitNode = std::move(other.exitNode);
        return *this;
    }

    ~CCFG() {
        for (auto &blk : nodes) {
            blk->nexts.clear();
        }
    }

private:
    void copy_tree(const CCFG& a) {
        auto oldMapsTo = std::map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>>{};
        for (auto &n : a.nodes) {
            auto newNode = std::make_shared<basicblock>(basicblock(*n));
            if (newNode->type == Entry) startNode = newNode;
            else if (newNode->type == Exit) exitNode = newNode;
            nodes.insert(newNode);
            oldMapsTo.insert({n, newNode});
        }
        for (const auto &n : a.nodes) {
            oldMapsTo[n]->concurrentBlock = std::make_pair(oldMapsTo[n->concurrentBlock.first], n->concurrentBlock.second);
            for (const auto &next : n->nexts) {
                oldMapsTo[n]->nexts.push_back(oldMapsTo[next]);
                oldMapsTo[next]->parents.push_back(oldMapsTo[n]);
            }
        }
        for (const auto &ed : a.edges) {
            edges.insert(edge(ed.type, oldMapsTo[ed.neighbours[0]], oldMapsTo[ed.neighbours[1]]));
        }

    }

    void assign_parents() {
        for (const auto it : nodes) {
            if (!it->nexts.empty()) {
                for (const auto child : it->nexts) {
                    child->parents.push_back(it);
                }
            }
        }
    }

    void add_conflict_edges() {
        for (auto blk : nodes) {
            for (auto cmp : nodes) {
                if (concurrent(blk, cmp)) {
                    for (const auto var : blk->defines) {
                        if ((cmp->uses.find(var.first)) != cmp->uses.end() || cmp->defines.find(var.first) != cmp->defines.end()) {
                            //blocks are concurrent
                            // and have a variable in common,
                            // thus there's a conflict
                            edges.insert(edge(conflict, blk, cmp));
                            break;
                        }
                    }
                }
            }
        }
    }

    static bool concurrent(std::shared_ptr<basicblock> &a, std::shared_ptr<basicblock> &b) {
        if (a == b) return false; //same nodes aren't concurrent
        if (a->concurrentBlock.second == b->concurrentBlock.second) { //If they're in the same thread, they're not concurrent
            return false;
        }
        std::vector<std::shared_ptr<basicblock>> concurrentNodesForA;
        std::shared_ptr<basicblock> tmp = a->concurrentBlock.first;
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
                    return true;
                }
            }
            tmp = tmp->concurrentBlock.first;
        }
        return false; //if we get here, no conditions were met, thus not concurrent
    }
};

class basicBlockTreeConstructor {
public:
    basicBlockTreeConstructor() = default;

    CCFG get_ccfg(const std::shared_ptr<statementNode> &startTree) {
        std::shared_ptr<basicblock> exitNode = std::make_shared<basicblock>(basicblock());
        const std::shared_ptr<basicblock> startNode = get_tree(startTree, exitNode);

        auto tmpSet = std::set<basicblock *>();
        auto res = get_all_blocks_and_edges(startNode, exitNode, &tmpSet);

        //tmpSet;
        std::unordered_set<edge> edges{!res.second.empty() ? res.second[0] : edge(startNode, std::make_shared<basicblock>(basicblock()))};
        for(auto it : res.second) edges.insert(it);

        tmpSet = std::set<basicblock *>();
        for(const auto &it : res.first) //add threadnum to blocks. Don't want to add to children already visited
            if (!it->statements.empty() && it->statements[0]->getNodeType() == Concurrent && !it->concurrentBlock.first)
                it->setConcurrentBlock(it, 0, &tmpSet);

        std::vector<std::shared_ptr<basicblock>> blocksToAdd;
        std::vector<edge> edgesToAdd;

        tmpSet = std::set<basicblock *>();
        for(const auto &it : res.first) {
            if (it->concurrentBlock.first && it->statements.size() > 1) {
                std::list<std::shared_ptr<statementNode>> stmts;
                for(auto stmt : it->statements) {
                    stmts.push_back(stmt);
                }
                stmts.pop_front();
                auto blk = split_up_concurrent_basicblocks(&blocksToAdd, &edgesToAdd, it->nexts, stmts, it->concurrentBlock, &tmpSet);
                edgesToAdd.push_back(edge(it, blk));
                for (auto nxt : it->nexts)
                    edges.erase(edge(it, nxt));
                it->nexts = std::vector<std::shared_ptr<basicblock>>{blk};
                it->statements.resize(1);
            }
        }

        for (auto blk : blocksToAdd) res.first.insert(blk);
        for (auto ed : edgesToAdd) edges.insert(ed);

        for (const auto blk : res.first) blk->updateUsedVariables();
        //std::cout << "hej\n";

        return CCFG(std::move(res.first), std::move(edges), startNode, exitNode);
    }

    std::shared_ptr<basicblock> get_tree(const std::shared_ptr<statementNode> startTree, std::shared_ptr<basicblock> nxt) {
        std::shared_ptr<basicblock> block;
        switch (startTree->getNodeType()) {
            case Assign:
            case AssignArrField:
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
                auto endConc = std::make_shared<basicblock>(
                        basicblock(std::make_shared<endConcNode>(endConcNode(conNode->threads.size(), block)), nxt));
                endConc->type = Coend;
                for (const auto &t : conNode->threads) {
                    threads.push_back(get_tree(t, endConc));
                }

                //for (auto blk : threads) concNode->threads.push_back(blk);
                block->nexts = std::move(threads);
                break;
            }
            case Sequential: {
                auto seqNode = dynamic_cast<sequentialNode*>(startTree.get());
                auto rest = get_tree(seqNode->getNext(), nxt);
                block = get_tree(seqNode->getBody(), rest);
                NodeType nType = seqNode->getBody()->getNodeType();
                if (nType == Assign || nType == AssignArrField || nType == Write || nType == Event || nType == Skip) {
                    std::vector<std::shared_ptr<statementNode>> remainingStmts;
                    int stmtsRemoved = 0;
                    for (const auto stmt : rest->statements) {
                        nType = stmt->getNodeType();
                        if (nType == Assign || nType == AssignArrField || nType == Write || nType == Event ||
                            nType == Skip) {
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
                block->nexts[0] = get_tree(wNode->getBody(), block);
                break;
            }
            case If: {
                auto ifNode = dynamic_cast<ifElseNode*>(startTree.get());
                if (nxt->type == Loop) {
                    nxt = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{}, nxt));
                }
                auto trueBranch = get_tree(ifNode->getTrueBranch(), nxt);
                auto falseBranch = get_tree(ifNode->getFalseBranch(), nxt);
                auto nxts = std::vector<std::shared_ptr<basicblock>>{trueBranch, falseBranch};
                block = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{startTree}, nxts));
                block->type = Condition;
                break;
            }
            default:
                break;
        }
        return block;
    }

private:
    std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>> get_all_blocks_and_edges(const std::shared_ptr<basicblock> &startTree, const std::shared_ptr<basicblock> &exitNode, std::set<basicblock *> *whileLoops) {
        std::set<std::shared_ptr<basicblock>> basicblocks;
        std::vector<edge> edges;
        if (!startTree) return std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>>{basicblocks, edges};
        //If this block has already been visited (while loop), then stop recursion
        if (!startTree->statements.empty() && startTree->statements[0]->getNodeType() == While && !whileLoops->insert(startTree.get()).second) return std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>>{basicblocks, edges};

        if (startTree != exitNode) {
            auto blockInsertion = basicblocks.insert(startTree);
            for(auto i = 0; i < startTree->nexts.size(); ++i) {
                edges.push_back(edge(startTree, startTree->nexts[i]));
            }

            for (auto i = 0; i < startTree->nexts.size(); ++i) {
                std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>> res;
                res = get_all_blocks_and_edges(startTree->nexts[i], exitNode, whileLoops);
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

    std::shared_ptr<basicblock> split_up_concurrent_basicblocks(std::vector<std::shared_ptr<basicblock>> *blocksToAdd, std::vector<edge> *edgesToAdd, std::vector<std::shared_ptr<basicblock>> it, std::list<std::shared_ptr<statementNode>> stmts, std::pair<std::shared_ptr<basicblock>, int> conBlock, std::set<basicblock *> *whileloops) {
        if (stmts.size() == 1) {
            std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(stmts.front()));
            blk->nexts = it;
            for (auto ed : it) edgesToAdd->push_back(edge(blk, ed));
            blocksToAdd->push_back(blk);
            blk->setConcurrentBlock(conBlock.first, conBlock.second, whileloops);
            return blk;
        } else {
            auto firstStmt = stmts.front();
            stmts.pop_front();
            auto nxt = split_up_concurrent_basicblocks(blocksToAdd, edgesToAdd, it, stmts, conBlock, whileloops);
            std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(firstStmt, nxt));
            edgesToAdd->push_back(edge(blk, nxt));
            blocksToAdd->push_back(blk);
            blk->setConcurrentBlock(conBlock.first, conBlock.second, whileloops);
            return blk;
        }
    }
};
#endif //ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
