//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP

struct basicblock : public statementNode {
    basicblock() : statements{}, nexts{} {};
    basicblock(std::shared_ptr<statementNode> stmt) :
            statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
            nexts{} {};
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts) :
        statements{std::move(stmts)},
        nexts{} {};
    basicblock(std::shared_ptr<statementNode> stmt, std::shared_ptr<basicblock> next) :
            statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
            nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}} {};
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::shared_ptr<basicblock> next) :
            statements{std::move(stmts)},
            nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}} {};
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::vector<std::shared_ptr<basicblock>> nStmts) :
        statements{std::move(stmts)}, nexts{std::move(nStmts)} {};


    std::vector<std::shared_ptr<statementNode>> statements;
    std::vector<std::shared_ptr<basicblock>> nexts;
};

#endif //ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
