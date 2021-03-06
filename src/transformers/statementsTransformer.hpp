//
// Created by hu on 04/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEMENTSTRANSFORMER_HPP
#define ANTLR_CPP_TUTORIAL_STATEMENTSTRANSFORMER_HPP

#include <src/transformers/CSSA_transformer.hpp>
#include <src/nodes/nodes.hpp>

struct statementsTransformer {
    statementsTransformer() = default;

    static std::shared_ptr<CCFG> get_transformedCCFG(const std::shared_ptr<CSSA_CCFG> &cssacfg){
        std::shared_ptr<CSSA_CCFG> ccfg = std::make_shared<CSSA_CCFG>(*cssacfg);
        for (auto &blk : ccfg->nodes) {
            unpackBlock(blk, ccfg.get());
        }

        return ccfg;
    }

private:
    static void unpackBlock(const std::shared_ptr<basicblock> &blk, CCFG *ccfg) {
        for (auto &stmt : blk->statements) {
            stmt = unpackStatement(stmt, ccfg);
        }
    }

    static std::shared_ptr<statementNode> unpackStatement(std::shared_ptr<statementNode> stmt, CCFG *ccfg) {
        std::shared_ptr<statementNode> unStmt;
        switch (stmt->getNodeType()) {
            case Assert: {
                auto assert_node = dynamic_cast<assertNode*>(stmt.get());
                auto expr = unpackExpr(assert_node->getCondition(), ccfg);
                expr->get_last()->next = std::make_shared<unpacked>(boolType, Assert);
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(expr, stmt->get_linenum()));
                break;
            }
            case Assign: {
                auto assNode = dynamic_cast<assignNode*>(stmt.get());
                auto unpack = unpackExpr(assNode->getExpr(), ccfg);
                std::string newname;
                auto chars_to_remove = assNode->getOriginalName().size()+1;
                int num = std::stoi(assNode->getName().erase(0,chars_to_remove)) - 1;
                newname = assNode->getOriginalName() + "_" + std::to_string(num);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(assNode->getExpr()->getType(), Assign, assNode->getName(), newname));
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack, stmt->get_linenum()));
                break;
            } case AssignArrField: {
                auto assignArF = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                auto unpack = unpackExpr(assignArF->getField(), ccfg);
                unpack->get_last()->next = unpackExpr(assignArF->getExpr(), ccfg);
                std::string newname;
                auto chars_to_remove = assignArF->getOriginalName().size()+1;
                int num = std::stoi(assignArF->getName().erase(0,chars_to_remove)) - 1;
                newname = assignArF->getOriginalName() + "_" + std::to_string(num);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(assignArF->getExpr()->getType(), AssignArrField, assignArF->getName(), newname));
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack, stmt->get_linenum()));
                break;
            } case If: {
                auto ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                auto unpack = unpackExpr(ifNode->getCondition(), ccfg);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(boolType, If));
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack, stmt->get_linenum()));
                break;
            } case Write: {
                auto writeStmt = dynamic_cast<writeNode*>(stmt.get());
                auto unpack = unpackExpr(writeStmt->getExpr(), ccfg);
                unpack->get_last()->next = std::make_shared<unpacked>(okType, Write);
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack, stmt->get_linenum()));
                break;
            } case Event: {
                auto eventStmt = dynamic_cast<eventNode*>(stmt.get());
                auto unpack = unpackExpr(eventStmt->getCondition(), ccfg);
                unpack->get_last()->next = std::make_shared<unpacked>(boolType, Event);
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(unpack, stmt->get_linenum()));
                break;
            } case Skip: {
                unStmt = std::make_shared<unpackedstmt>(unpackedstmt(std::make_shared<unpacked>(unpacked(okType, Skip)), stmt->get_linenum()));
                break;
            }
            default: return stmt;
        }
        return unStmt;
    }

    static std::shared_ptr<unpacked> unpackExpr(expressionNode *expr, CCFG *ccfg) {
        std::shared_ptr<unpacked> unpack;
        switch (expr->getNodeType()) {
            case Read: {
                unpack = std::make_shared<unpacked>(unpacked(intType, Read, dynamic_cast<readNode*>(expr)->getName()));
                break;
            } case Literal: {
                auto lit = dynamic_cast<literalNode*>(expr);
                unpack = std::make_shared<unpacked>(unpacked(lit->getType(), Literal, lit->value));
                break;
            } case ArrayAccess: {
                auto arracc = dynamic_cast<arrayAccessNode*>(expr);
                unpack = unpackExpr(arracc->getAccessor(), ccfg);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(arracc->getType(), ArrayAccess, arracc->getName()));
                break;
            } case ArrayLiteral: {
                auto arrLit = dynamic_cast<arrayLiteralNode*>(expr);
                auto arrayLiteral = arrLit->getArrLit();
                unpack = unpackExpr(arrayLiteral[0].get(), ccfg);
                for (size_t i = 1; i < arrayLiteral.size(); ++i) {
                    unpack->get_last()->next = unpackExpr(arrayLiteral[i].get(), ccfg);
                }
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(arrLit->getType(), ArrayLiteral, std::to_string(arrayLiteral.size())));
                break;
            } case Variable: {
                auto var = dynamic_cast<variableNode*>(expr);
                unpack = std::make_shared<unpacked>(unpacked(var->getType(), Variable, var->name));
                break;
            } case BinaryExpression: {
                auto binExpr = dynamic_cast<binaryExpressionNode*>(expr);
                unpack = unpackExpr(binExpr->getLeft(), ccfg);
                unpack->get_last()->next = unpackExpr(binExpr->getRight(), ccfg);
                unpack->get_last()->next = std::make_shared<unpacked>(unpacked(binExpr->getType(), BinaryExpression, binExpr->getOperator()));
                break;
            } case UnaryExpression: {
                auto unExpr = dynamic_cast<unaryExpressionNode*>(expr);
                unpack = unpackExpr(unExpr->getExpr(), ccfg);
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
