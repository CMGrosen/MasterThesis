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

    void setConcurrentBlock(const std::shared_ptr<basicblock> &blk) {
        concurrentBlock = blk;
        for (const auto &nxt : nexts) {
            nxt->setConcurrentBlock(blk);
        }
    }
    std::shared_ptr<basicblock> concurrentBlock = nullptr;
};


#endif //ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
