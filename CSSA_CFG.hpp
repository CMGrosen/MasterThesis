//
// Created by hu on 17/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
#define ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP

#include <basicblockTreeConstructor.hpp>

struct CSSA_CFG {
    std::shared_ptr<CCFG> ccfg;

    CSSA_CFG(std::shared_ptr<CCFG> ccfg) : ccfg{std::move(ccfg)} {
        copy_ccfg();
    }

private:
    void copy_ccfg() {
        std::unordered_map<basicblock *, std::shared_ptr<basicblock>> visited_blocks;

        std::vector<edge> edges;
        edges.reserve(ccfg->edges.size());
        std::vector<std::shared_ptr<basicblock>> children;

        std::set<std::shared_ptr<basicblock>> blocks;
        std::unordered_set<edge> new_edges;
        std::shared_ptr<basicblock> exitnode = ccfg->exitNode;

        for (auto child : ccfg->startNode->nexts) {
            children.push_back(copy_child(child, &visited_blocks, &blocks, &edges, &exitnode));
        }

        std::shared_ptr<basicblock> startNode = std::make_shared<basicblock>(basicblock(copy_statements(ccfg->startNode->statements), children));

        for(auto ed : edges) {
            new_edges.insert(std::move(ed));
        }

        ccfg = std::make_shared<CCFG>(CCFG(blocks, new_edges, startNode, exitnode));
        /*
        std::set<std::shared_ptr<basicblock>> blocks;
        std::unordered_set<std::shared_ptr<basicblock>>
        std::shared_ptr<basicblock> blk = copy_node(&visited_blocks);
        ccfg = std::make_shared<CCFG>(CCFG())*/
    }

    std::shared_ptr<basicblock> copy_node(std::shared_ptr<basicblock> child, std::unordered_map<basicblock *, std::shared_ptr<basicblock>> *visited_blocks, std::set<std::shared_ptr<basicblock>> *blocks, std::vector<edge> *edges, std::shared_ptr<basicblock> *exitNode) {

    }

    std::shared_ptr<basicblock> copy_child(std::shared_ptr<basicblock> child, std::unordered_map<basicblock *, std::shared_ptr<basicblock>> *visited_blocks, std::set<std::shared_ptr<basicblock>> *blocks, std::vector<edge> *edges, std::shared_ptr<basicblock> *exitNode) {
        return copy_node(child, visited_blocks, blocks, edges, exitNode);
    }

    std::vector<std::shared_ptr<statementNode>> copy_statements(std::vector<std::shared_ptr<statementNode>> stmts) {
        std::vector<std::shared_ptr<statementNode>> copied_statements;
        copied_statements.reserve(stmts.size());
        for (auto stmt : stmts) {
            copied_statements.push_back(stmt->copy_statement());
        }
        return std::move(copied_statements);
    };
};

#endif //ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
