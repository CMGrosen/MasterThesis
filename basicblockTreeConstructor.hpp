//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP

#include <nodes/basicblock.hpp>






struct CCFG {
    std::set<std::shared_ptr<basicblock>> nodes;
    std::unordered_set<edge> edges;
    std::shared_ptr<basicblock> startNode;
    std::shared_ptr<basicblock> exitNode;
    CCFG(std::set<std::shared_ptr<basicblock>> nodes, std::unordered_set<edge> edges, std::shared_ptr<basicblock> start, std::shared_ptr<basicblock> exit)
        : nodes{std::move(nodes)}, edges{std::move(edges)}, startNode{std::move(start)}, exitNode{std::move(exit)} {}
};

class basicBlockTreeConstructor {
public:
    basicBlockTreeConstructor() = default;

    CCFG get_ccfg(const std::shared_ptr<statementNode> &startTree) {
        std::shared_ptr<basicblock> exitNode = std::make_shared<basicblock>(basicblock());
        const std::shared_ptr<basicblock> startNode = get_tree(startTree, exitNode);
        //std::unordered_set<edge> edges{};
        /*while(!current->nexts.empty()) {
            current = current->nexts[current->nexts.size()-1];
        }*/
        //std::shared_ptr<basicblock> exit = current;

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

        add_conflict_edges(&res.first, &edges);
        return CCFG(std::move(res.first), std::move(edges), startNode, exitNode);
    }

    std::shared_ptr<basicblock> get_tree(const std::shared_ptr<statementNode> startTree, const std::shared_ptr<basicblock> nxt) {
        std::shared_ptr<basicblock> block;
        switch (startTree->getNodeType()) {
            case Assign:
            case AssignArrField:
            case Write:
            case Event:
            case Skip:
                block = std::make_shared<basicblock>(basicblock(startTree, nxt));
                break;
            case Concurrent: {
                auto conNode = dynamic_cast<concurrentNode *>(startTree.get());
                std::vector<std::shared_ptr<basicblock>> threads;
                for (const auto &t : conNode->threads) {
                    threads.push_back(get_tree(t, nxt));
                }
                auto concNode = std::make_shared<concurrentNode>(concurrentNode(okType, threads));
                block = std::make_shared<basicblock>(std::vector<std::shared_ptr<statementNode>>{concNode}, threads);
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
                block->nexts[0] = get_tree(wNode->getBody(), block);
                break;
            }
            case If: {
                auto ifNode = dynamic_cast<ifElseNode*>(startTree.get());
                auto trueBranch = get_tree(ifNode->getTrueBranch(), nxt);
                auto falseBranch = get_tree(ifNode->getFalseBranch(), nxt);
                auto nxts = std::vector<std::shared_ptr<basicblock>>{trueBranch, falseBranch};
                block = std::make_shared<basicblock>(basicblock(std::vector<std::shared_ptr<statementNode>>{startTree}, nxts));
                break;
            }
            default:
                break;
        }
        return block;
    }

    //std::vector<std::shared_ptr<basicblock>> blocks = std::vector<std::shared_ptr<basicblock>>{};

private:
    uint iterator = -1;

    std::shared_ptr<basicblock> handle_while(std::shared_ptr<statementNode> tree, std::shared_ptr<basicblock> last) {

    }

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
/*            if (!blockInsertion.second || !edgeInsertionSuccessfull) {
                return std::pair<std::set<std::shared_ptr<basicblock>>, std::set<edge>>{basicblocks, edges};
            }
*/
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

    void add_conflict_edges(std::set<std::shared_ptr<basicblock>> *allBlocks, std::unordered_set<edge> *allEdges) {
        for (auto blk : *allBlocks) {
            for (auto cmp : *allBlocks) {
                if (concurrent(blk, cmp)) {
                    auto vars = blk->variables;
                    for (const auto var : vars) {
                        if ((cmp->variables.find(var)) != cmp->variables.end()) {
                            //blocks are concurrent
                            // and have a variable in common,
                            // thus there's a conflict
                            allEdges->insert(edge(conflict, blk, cmp));
                            break;
                        }
                    }
                }
            }
        }
    }

    bool concurrent(std::shared_ptr<basicblock> a, std::shared_ptr<basicblock> b) {
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
            for (const auto n : concurrentNodesForA) {
                /*if(tmp->concurrentBlock.second == n->concurrentBlock.second) {
                    return false;
                }*/
                if (tmp == n) { // common fork ancestor between nodes, making them concurrent
                    return true;
                }
            }
            tmp = tmp->concurrentBlock.first;
        }
        return false; //if we get here, no conditions were met, thus not concurrent
    }
};
#endif //ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
