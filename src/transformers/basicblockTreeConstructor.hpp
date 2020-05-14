//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP

#include <src/nodes/basicblock.hpp>
#include <src/CFGs/CCFG.hpp>
#include <unordered_set>
#include <list>
#include <queue>
#include <unordered_map>
#include <iostream>

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
                auto conNode = reinterpret_cast<concurrentNode *>(startTree.get());
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
                auto seqNode = reinterpret_cast<sequentialNode*>(startTree.get());
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
                auto wNode = reinterpret_cast<whileNode*>(startTree.get());
                block = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{startTree}, std::vector<std::shared_ptr<basicblock>>{nullptr, nxt}));
                block->type = Loop;
                block->nexts[0] = get_tree(wNode->getBody(), block, in_if);
                break;
            }
            case If: {
                auto ifNode = reinterpret_cast<ifElseNode*>(startTree.get());
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
                auto fi = reinterpret_cast<fiNode*>(joinnode->statements.back().get());
                fi->add_parent(block);
                if (!fi->first_parent) fi->first_parent = block;

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
