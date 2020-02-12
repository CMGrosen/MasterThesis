//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP

enum edgeType {flow, conflict};

struct edge {
    edgeType type;
    std::vector<std::shared_ptr<basicblock>> neighbours;
    bool operator<(const edge& s) const { return neighbours[0] < s.neighbours[1];
        if (neighbours[0] == s.neighbours[1] || neighbours[1] == s.neighbours[0])
            return neighbours[0] < s.neighbours[1] && neighbours[1] < s.neighbours[0] && type < s.type;
        else
            return neighbours[0] < s.neighbours[0] && neighbours[1] < s.neighbours[1] && type < s.type;
    }
    bool operator==(const edge& s) const {
        if (neighbours[0] == s.neighbours[1])
            return neighbours[0] == s.neighbours[1] && neighbours[1] == s.neighbours[0] && type == s.type;
        else
            return neighbours[0] == s.neighbours[0] && neighbours[1] == s.neighbours[1] && type == s.type;
    }
    edge() : type{flow} {neighbours.reserve(2);}
    edge(std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
        type{flow}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}
    edge(edgeType typeOfEdge, std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
            type{typeOfEdge}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


namespace std {

    template<> struct hash<edge> {
        size_t operator()(const edge& s) const {
            size_t seed = 0;
            if (s.neighbours[0] < s.neighbours[1]) hash_combine(seed, s.neighbours[0]);
            else hash_combine(seed, s.neighbours[1]);
            hash_combine(seed, s.type);
            return seed;
        }
    };
}

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
        const std::shared_ptr<basicblock> startNode = get_tree(startTree);
        std::shared_ptr<basicblock> current = startNode;
        //std::unordered_set<edge> edges{};
        while(!current->nexts.empty()) {
            current = current->nexts[current->nexts.size()-1];
        }
        std::shared_ptr<basicblock> exit = current;

        auto res = get_all_blocks_and_edges(startNode, exit);

        std::unordered_set<edge> edges{!res.second.empty() ? res.second[0] : edge(startNode, std::make_shared<basicblock>(basicblock()))};
        for(auto it : res.second) edges.insert(it);

        for(const auto &it : res.first) //add threadnum to blocks. Don't want to add to children already visited
            if (!it->statements.empty() && it->statements[0]->getNodeType() == Concurrent && !it->concurrentBlock.first)
                it->setConcurrentBlock(it, 0);

        std::vector<std::shared_ptr<basicblock>> blocksToAdd;
        std::vector<edge> edgesToAdd;

        for(const auto &it : res.first) {
            if (it->concurrentBlock.first && it->statements.size() > 1) {
                std::list<std::shared_ptr<statementNode>> stmts;
                for(auto stmt : it->statements) {
                    stmts.push_back(stmt);
                }
                stmts.pop_front();
                auto blk = split_up_concurrent_basicblocks(&blocksToAdd, &edgesToAdd, it->nexts, stmts, it->concurrentBlock);
                edgesToAdd.push_back(edge(it, blk));
                for (auto nxt : it->nexts)
                    edges.erase(edge(it, nxt));
                it->nexts = std::vector<std::shared_ptr<basicblock>>{blk};
                it->statements.resize(1);
            }
        }

        for (auto blk : blocksToAdd) res.first.insert(blk);
        for (auto ed : edgesToAdd) edges.insert(ed);

        std::cout << "hej\n";

        return CCFG(std::move(res.first), std::move(edges), startNode, exit);
    }

    std::shared_ptr<basicblock> get_tree(const std::shared_ptr<statementNode> startTree) {
        if (startTree->getNodeType() == Concurrent) {
            auto n = dynamic_cast<concurrentNode*>(startTree.get());
            auto length = n->threads.size();
            std::vector<std::shared_ptr<basicblock>> vec = std::vector<std::shared_ptr<basicblock>>();
            vec.reserve(length);

            for(auto i = 0; i < length; ++i) {
                auto res = get_tree(n->threads[i]);
                vec.push_back(res);
            }

            concurrentNode node = concurrentNode(okType, vec);
            basicblock res = basicblock(std::vector<std::shared_ptr<statementNode>>{std::make_shared<concurrentNode>(node)}, vec);
            return std::make_shared<basicblock>(res);
        } else if (startTree->getNodeType() == Sequential) {
            auto node = startTree;
            /*auto current = n->getBody();
            auto next = n->getNext();*/
            auto vec = std::vector<std::shared_ptr<statementNode>>{};

            while(node->getNodeType() == Sequential) {
                auto n = dynamic_cast<sequentialNode*>(node.get());
                auto body = n->getBody();
                auto next = n->getNext();
                NodeType nType = body->getNodeType();
                if (nType == Assign || nType == AssignArrField || nType == Write || nType == Event) {
                    vec.push_back(body);
                } else if (nType == While || nType == If) {
                    auto blk = std::make_shared<basicblock>(basicblock(body));
                    blocks.push_back(std::make_shared<basicblock>(basicblock(vec, blk)));
                    blocks.push_back(blk);
                    if(auto wNode = dynamic_cast<whileNode*>(body.get())) {
                        blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(wNode->getBody()), get_tree(next)};
                        std::shared_ptr<basicblock> last = blk->nexts[0];
                        while (!last->nexts.empty()) last = last->nexts[0];
                        //last->nexts =
                    } else if (auto iNode = dynamic_cast<ifElseNode*>(body.get())) {
                        blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(iNode->getTrueBranch()), get_tree(iNode->getFalseBranch())};
                        std::shared_ptr<basicblock> lastTrue = blk->nexts[0];
                        std::shared_ptr<basicblock> lastFalse = blk->nexts[1];
                        while (!lastTrue->nexts.empty()) lastTrue = lastTrue->nexts[0];
                        while (!lastFalse->nexts.empty()) lastFalse = lastFalse->nexts[0];
                        auto res = get_tree(next);
                        lastTrue->nexts = std::vector<std::shared_ptr<basicblock>>{res};
                        lastFalse->nexts = std::vector<std::shared_ptr<basicblock>>{res};
                    }
                    vec.clear();
                }
                node = next;
            }
            NodeType nType = node->getNodeType();
            if (nType == Assign || nType == AssignArrField || nType == Write || nType == Event) {
                vec.push_back(node);
            } else if (nType == While || nType == If) {
                auto blk = std::make_shared<basicblock>(basicblock(node));
                auto fststmts = std::make_shared<basicblock>(basicblock(vec, blk));
                if(auto wNode = dynamic_cast<whileNode*>(node.get())) {
                    blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(wNode->getBody())};
                    /*std::shared_ptr<basicblock> last = blk->nexts[0];
                    while (!last->nexts.empty()) last = last->nexts[0];*/
                    //last->nexts =
                } else if (auto iNode = dynamic_cast<ifElseNode*>(node.get())) {
                    blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(iNode->getTrueBranch()), get_tree(iNode->getFalseBranch())};
                    /*std::shared_ptr<basicblock> lastTrue = blk->nexts[0];
                    std::shared_ptr<basicblock> lastFalse = blk->nexts[1];
                    while (!lastTrue->nexts.empty()) lastTrue = lastTrue->nexts[0];
                    while (!lastFalse->nexts.empty()) lastFalse = lastFalse->nexts[0];
                    auto t = std::make_shared<basicblock>(basicblock());
                    lastTrue->nexts = std::vector<std::shared_ptr<basicblock>>{t};
                    lastFalse->nexts = std::vector<std::shared_ptr<basicblock>>{t};*/
                }
                return fststmts;
            } else {
                return std::make_shared<basicblock>(basicblock(vec,get_tree(node)));
            }
            if (node->getNodeType() == Concurrent) {return nullptr;}
            else {return std::make_shared<basicblock>(basicblock(vec));}
        } else {
            auto blk = std::make_shared<basicblock>(basicblock(startTree));
            if(auto wNode = dynamic_cast<whileNode*>(startTree.get())) {
                blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(wNode->getBody())};
                std::shared_ptr<basicblock> last = blk->nexts[0];
                while (!last->nexts.empty()) last = last->nexts[0];
                auto t = std::make_shared<basicblock>(basicblock());
                last->nexts = std::vector<std::shared_ptr<basicblock>>{t};
                //last->nexts =
            } else if (auto iNode = dynamic_cast<ifElseNode*>(startTree.get())) {
                blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(iNode->getTrueBranch()), get_tree(iNode->getFalseBranch())};
                std::shared_ptr<basicblock> lastTrue = blk->nexts[0];
                std::shared_ptr<basicblock> lastFalse = blk->nexts[1];
                while (!lastTrue->nexts.empty()) lastTrue = lastTrue->nexts[0];
                while (!lastFalse->nexts.empty()) lastFalse = lastFalse->nexts[0];
                auto t = std::make_shared<basicblock>(basicblock());
                lastTrue->nexts = std::vector<std::shared_ptr<basicblock>>{t};
                lastFalse->nexts = std::vector<std::shared_ptr<basicblock>>{t};
            }
            return blk;
        }
    }

    std::vector<std::shared_ptr<basicblock>> blocks = std::vector<std::shared_ptr<basicblock>>{};

private:
    uint iterator = -1;

    std::pair<std::set<std::shared_ptr<basicblock>>, std::vector<edge>> get_all_blocks_and_edges(const std::shared_ptr<basicblock> &startTree, const std::shared_ptr<basicblock> &exitNode) {
        std::set<std::shared_ptr<basicblock>> basicblocks;
        std::vector<edge> edges;
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
                auto res = get_all_blocks_and_edges(startTree->nexts[i], exitNode);
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

    std::shared_ptr<basicblock> split_up_concurrent_basicblocks(std::vector<std::shared_ptr<basicblock>> *blocksToAdd, std::vector<edge> *edgesToAdd, std::vector<std::shared_ptr<basicblock>> it, std::list<std::shared_ptr<statementNode>> stmts, std::pair<std::shared_ptr<basicblock>, int> conBlock) {
        if (stmts.size() == 1) {
            std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(stmts.front()));
            blk->nexts = it;
            for (auto ed : it) edgesToAdd->push_back(edge(blk, ed));
            blocksToAdd->push_back(blk);
            blk->setConcurrentBlock(conBlock.first, conBlock.second);
            return blk;
        } else {
            auto firstStmt = stmts.front();
            stmts.pop_front();
            auto nxt = split_up_concurrent_basicblocks(blocksToAdd, edgesToAdd, it, stmts, conBlock);
            std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(firstStmt, nxt));
            edgesToAdd->push_back(edge(blk, nxt));
            blocksToAdd->push_back(blk);
            blk->setConcurrentBlock(conBlock.first, conBlock.second);
            return blk;
        }
    }


};
#endif //ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
