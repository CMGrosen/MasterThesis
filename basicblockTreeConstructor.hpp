//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP

class basicBlockTreeConstructor {
public:
    basicBlockTreeConstructor() = default;
    std::shared_ptr<basicblock> get_tree(const std::shared_ptr<statementNode> startTree) {
        if (startTree->getNodeType() == Concurrent) {
            auto n = dynamic_cast<concurrentNode*>(startTree.get());
            auto length = n->threads.size();
            std::vector<std::shared_ptr<basicblock>> vec = std::vector<std::shared_ptr<basicblock>>();
            vec.reserve(length);

            for(auto i = 0; i < length; ++i) {
                vec[i] = get_tree(n->threads[i]);
            }

            concurrentNode node = concurrentNode(okType, vec);
            basicblock res = basicblock(std::vector<std::shared_ptr<statementNode>>{std::make_shared<concurrentNode>(node)});
            return std::make_shared<basicblock>(res);
        } else if (startTree->getNodeType() == Sequential) {
            auto nn = startTree.get();
            /*auto current = n->getBody();
            auto next = n->getNext();*/
            auto vec = std::vector<std::shared_ptr<statementNode>>{};

            while(nn->getNodeType() == Sequential) {
                auto n = dynamic_cast<sequentialNode*>(nn);
                auto current = n->getBody();
                auto next = n->getNext();
                NodeType nType = current->getNodeType();
                if (nType == Assign || nType == AssignArrField || nType == Write || nType == Event) {
                    vec.push_back(current);
                } else if (nType == While || nType == If) {
                    auto blk = std::make_shared<basicblock>(basicblock(current));
                    blocks.push_back(std::make_shared<basicblock>(basicblock(vec, blk)));
                    blocks.push_back(blk);
                    if(auto wNode = dynamic_cast<whileNode*>(current.get())) {
                        blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(wNode->getBody()), blk};
                        std::shared_ptr<basicblock> last = blk->nexts[0];
                        while (!last->nexts.empty()) last = last->nexts[0];
                        //last->nexts =
                    } else if (auto iNode = dynamic_cast<ifElseNode*>(current.get())) {
                        blk->nexts = std::vector<std::shared_ptr<basicblock>>{get_tree(iNode->getTrueBranch()), get_tree(iNode->getFalseBranch())};
                        std::shared_ptr<basicblock> lastTrue = blk->nexts[0];
                        std::shared_ptr<basicblock> lastFalse = blk->nexts[1];
                        while (!lastTrue->nexts.empty()) lastTrue = lastTrue->nexts[0];
                        while (!lastFalse->nexts.empty()) lastFalse = lastFalse->nexts[0];
                    }
                    vec.clear();
                }
                nn = n->getNext();
            }
            if (nn->getNodeType() == Concurrent) {return nullptr;}
            else {return std::make_shared<basicblock>(basicblock(vec));}
        } else {
            return std::make_shared<basicblock>(std::vector<std::shared_ptr<statementNode>>{startTree});
        }
    }

    std::vector<std::shared_ptr<basicblock>> blocks = std::vector<std::shared_ptr<basicblock>>{};
private:
    uint iterator = -1;
};
#endif //ANTLR_CPP_TUTORIAL_BASICBLOCKTREECONSTRUCTOR_HPP
