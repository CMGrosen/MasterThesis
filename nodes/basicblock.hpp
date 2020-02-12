//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP

struct basicblock : public statementNode {
    basicblock() : statements{}, nexts{} {setNodeType(BasicBlock);};
    basicblock(std::shared_ptr<statementNode> stmt) :
            statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
            nexts{} {setNodeType(BasicBlock);};
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts) :
        statements{std::move(stmts)},
        nexts{} {setNodeType(BasicBlock);};
    basicblock(std::shared_ptr<statementNode> stmt, std::shared_ptr<basicblock> next) :
            statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
            nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}} {setNodeType(BasicBlock);};
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::shared_ptr<basicblock> next) :
            statements{std::move(stmts)},
            nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}} {setNodeType(BasicBlock);};
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::vector<std::shared_ptr<basicblock>> nStmts) :
        statements{std::move(stmts)}, nexts{std::move(nStmts)} {setNodeType(BasicBlock);};


    std::vector<std::shared_ptr<statementNode>> statements;
    std::vector<std::shared_ptr<basicblock>> nexts;

    void setConcurrentBlock(const std::shared_ptr<basicblock> &blk, int threadNum) {
        concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{blk, threadNum};
        if (!blk->statements.empty() && blk->statements[0]->getNodeType() == Concurrent) {
            for (int i = 0; i < nexts.size(); ++i) {
                nexts[i]->setConcurrentBlock(blk, threadNum+i);
            }
        } else {
            for (const auto &nxt : nexts) {
                nxt->setConcurrentBlock(blk, threadNum);
            }
        }
    }
    std::pair<std::shared_ptr<basicblock>, int> concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{nullptr, 0};
};


#endif //ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
