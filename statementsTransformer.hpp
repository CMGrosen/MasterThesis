//
// Created by hu on 04/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEMENTSTRANSFORMER_HPP
#define ANTLR_CPP_TUTORIAL_STATEMENTSTRANSFORMER_HPP

#include <CSSA_CFG.hpp>
#include <nodes/nodes.hpp>

struct statementsTransformer {
    statementsTransformer() = default;

    static std::shared_ptr<CCFG> get_transformedCCFG(const std::shared_ptr<CSSA_CFG> &cssacfg){
        int counter = 0;
        std::shared_ptr<CCFG> ccfg = std::make_shared<CCFG>(*cssacfg->ccfg);
        for (auto &blk : ccfg->nodes) {
            unpackBlock(blk, &counter);
        }


        return ccfg;
    }

private:
    static void unpackBlock(std::shared_ptr<basicblock> blk, int *counter) {
        for (auto &stmt : blk->statements) {
            stmt = unpackStatement(stmt, counter);
        }
    }

    static std::shared_ptr<statementNode> unpackStatement(std::shared_ptr<statementNode> stmt, int *counter) {
        std::shared_ptr<statementNode> unStmt;
        switch (stmt->getNodeType()) {
            case Assign: {
                auto assNode = dynamic_cast<assignNode*>(stmt.get());
                auto unpack = unpackExpr(assNode->getExpr(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(assNode->getExpr()->getType(), Assign, assNode->getName()));
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack));
                break;
            } case AssignArrField: {
                auto assignArF = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                auto unpack = unpackExpr(assignArF->getField(), counter);
                unpack->get_last()->next = unpackExpr(assignArF->getExpr(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(assignArF->getExpr()->getType(), AssignArrField));
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack));
                break;
            } case If: {
                auto ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                auto unpack = unpackExpr(ifNode->getCondition(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(boolType, If));
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack));
                break;
            } case Write: {
                auto writeStmt = dynamic_cast<writeNode*>(stmt.get());
                auto unpack = unpackExpr(writeStmt->getExpr(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(okType, Write);
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack));
                break;
            } case Event: {
                auto eventStmt = dynamic_cast<eventNode*>(stmt.get());
                auto unpack = unpackExpr(eventStmt->getCondition(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(boolType, Event);
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack));
                break;
            } default:
                return stmt;
        }
        return unStmt;
    }

    static std::shared_ptr<unpacked> unpackExpr(expressionNode *expr, int *counter) {
        std::shared_ptr<unpacked> unpack;
        switch (expr->getNodeType()) {
            case Read: {
                unpack = std::make_shared<unpacked>(unpacked(intType, Read, "-readVal" + std::to_string(*counter)));
                *counter = *counter + 1;
                break;
            } case Literal: {
                auto lit = dynamic_cast<literalNode*>(expr);
                unpack = std::make_shared<unpacked>(unpacked(lit->getType(), Literal, lit->value));
                break;
            } case ArrayAccess: {
                auto arracc = dynamic_cast<arrayAccessNode*>(expr);
                unpack = unpackExpr(arracc->getAccessor(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(arracc->getType(), ArrayAccess, arracc->getName()));
                break;
            } case ArrayLiteral: {
                auto arrLit = dynamic_cast<arrayLiteralNode*>(expr);
                auto arrayLiteral = arrLit->getArrLit();
                unpack = unpackExpr(arrLit->getArrLit()[0].get(), counter);
                for (auto i = 1; i < arrayLiteral.size(); ++i) {
                    unpack->get_last()->next = unpackExpr(arrayLiteral[i].get(), counter);
                }
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(arrLit->getType(), ArrayLiteral, std::to_string(arrayLiteral.size())));
                break;
            } case Variable: {
                auto var = dynamic_cast<variableNode*>(expr);
                unpack = std::make_shared<unpacked>(unpacked(var->getType(), Variable, var->name));
                break;
            } case BinaryExpression: {
                auto binExpr = dynamic_cast<binaryExpressionNode*>(expr);
                unpack = unpackExpr(binExpr->getLeft(), counter);
                unpack->get_last()->next = unpackExpr(binExpr->getRight(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(binExpr->getType(), BinaryExpression, binExpr->getOperator()));
                break;
            } case UnaryExpression: {
                auto unExpr = dynamic_cast<unaryExpressionNode*>(expr);
                unpack = unpackExpr(unExpr->getExpr(), counter);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(unExpr->getType(), UnaryExpression, unExpr->getOperator()));
                break;
            } case Skip: {
                unpack = std::make_shared<unpacked>(unpacked(okType, Skip));
                break;
            } default:break;
        }
        return unpack;
    }

};

#endif //ANTLR_CPP_TUTORIAL_STATEMENTSTRANSFORMER_HPP
