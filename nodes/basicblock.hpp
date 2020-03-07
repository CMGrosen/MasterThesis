//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP

#include <nodes/statements/arrayFieldAssignNode.hpp>
#include <nodes/statements/writeNode.hpp>
#include <nodes/statements/eventNode.hpp>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <set>
#include "edge.hpp"
#include "nodes/nodes.hpp"
#include <sstream>

enum blocktype {Entry, Exit, Cobegin, Coend, Condition, Loop, Compute, joinNode, ThreadEntry, ThreadExit};

struct basicblock {
    basicblock();
    basicblock(std::shared_ptr<statementNode> stmt);
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts);
    basicblock(std::shared_ptr<statementNode> stmt, std::shared_ptr<basicblock> next);
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::shared_ptr<basicblock> next);
    basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::vector<std::shared_ptr<basicblock>> nStmts);


    basicblock(const basicblock &o);

    basicblock& operator=(const basicblock &o);

    basicblock(basicblock&& o) noexcept;

    basicblock& operator=(basicblock &&o) noexcept;

    ~basicblock();

    std::vector<std::shared_ptr<statementNode>> statements;
    std::vector<std::weak_ptr<basicblock>> parents;
    std::vector<std::shared_ptr<basicblock>> nexts;

    void setConcurrentBlock(const std::shared_ptr<basicblock> &blk, int threadNum, std::set<basicblock *> *whileLoop);

    void updateUsedVariables();

    void setIfParent(const std::shared_ptr<basicblock>&);
    basicblock *getIfParent();

    std::string draw_picture(std::unordered_set<edge> *edges);

    std::pair<std::string, int> draw_block(basicblock *parent, int current, int total, int distanceToNeighbour, std::set<basicblock *> *drawnBlocks);

    std::string draw_children(basicblock *parent, std::set<basicblock *> *drawnBlocks);

    static int get_number_of_blocks();

    std::map<std::string, std::set<std::string>> uses;
    std::map<std::string, std::set<std::string>> defines;


    std::pair<std::shared_ptr<basicblock>, int> concurrentBlock;
    std::string get_name();

    blocktype type;
private:
    static int counterblocks;

    std::string get_address();

    std::pair<std::string, std::int32_t> statements_as_string();

    std::string name;
    basicblock *ifparent;

    static std::vector<const variableNode*> get_variables_from_expression(const expressionNode *expr) {
        std::vector<const variableNode*> vars{};
        switch (expr->getNodeType()) {
            case ArrayAccess: {
                auto arrAcc = dynamic_cast<const arrayAccessNode*>(expr);
                vars.push_back(arrAcc->getVar());
                auto res = get_variables_from_expression(arrAcc->getAccessor());
                for (auto var : res) vars.emplace_back(var);
                break;
            }
            case ArrayLiteral: {
                auto arrLit = dynamic_cast<const arrayLiteralNode*>(expr);
                for (auto xp : arrLit->getArrLit()) {
                    auto res = get_variables_from_expression(xp.get());
                    for (auto var : res) vars.emplace_back(var);
                }
                break;
            }
            case Variable: {
                vars.push_back(dynamic_cast<const variableNode*>(expr));
                break;
            }
            case BinaryExpression: {
                auto binExpr = dynamic_cast<const binaryExpressionNode*>(expr);
                for (const auto &e : get_variables_from_expression(binExpr->getLeft())) vars.push_back(e);
                for (const auto &e : get_variables_from_expression(binExpr->getRight())) vars.push_back(e);
                break;
            }
            case UnaryExpression: {
                auto unExpr = dynamic_cast<const unaryExpressionNode*>(expr);
                for (const auto &e : get_variables_from_expression(unExpr->getExpr())) vars.push_back(e);
                break;
            }
            default: {
                break;
            }
        }
        return vars;
    }
};

#endif //ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP