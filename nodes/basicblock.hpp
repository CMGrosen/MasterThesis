//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP

#include <nodes/statements/arrayFieldAssignNode.hpp>
#include <nodes/statements/writeNode.hpp>
#include <nodes/statements/eventNode.hpp>

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
        if (!statements.empty() && statements[0]->getNodeType() == Concurrent && this != blk.get()) {
            concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{blk, threadNum};
            for (int i = 0; i < nexts.size(); ++i) {
                nexts[i]->setConcurrentBlock(std::shared_ptr<basicblock>{this}, threadNum+i);
            }
        } else if (!statements.empty() && statements[0]->getNodeType() == Concurrent) {
            for (int i = 0; i < nexts.size(); ++i) {
                nexts[i]->setConcurrentBlock(blk, threadNum + i);
            }
        } else {
            concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{blk, threadNum};
            for (const auto &nxt : nexts) {
                nxt->setConcurrentBlock(blk, threadNum);
            }
        }
    }

    void updateUsedVariables() {
        for (auto stmt : statements) {
            switch (stmt->getNodeType()) {
                case Assign: {
                    auto assStmt = dynamic_cast<assignNode*>(stmt.get());
                    variables.insert(variableNode(okType, assStmt->getName()));
                    auto expr = assStmt->getExpr();
                    auto res = get_variables_from_expression(expr);
                    for(auto var : res) {
                        variables.insert(var);
                    }
                    break;
                }
                case AssignArrField: {
                    auto assArrStmt = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                    variables.insert(variableNode(okType, assArrStmt->getName()));
                    auto res = get_variables_from_expression(assArrStmt->getField()->getAccessor());
                    for(auto var : res) {
                        variables.insert(var);
                    }

                    res = get_variables_from_expression(assArrStmt->getExpr());
                    for(auto var : res) {
                        variables.insert(var);
                    }
                    break;
                }
                case While: {
                    auto whileStmt = dynamic_cast<whileNode*>(stmt.get());
                    auto res = get_variables_from_expression(whileStmt->getCondition());
                    for(auto var : res) {
                        variables.insert(var);
                    }
                    break;
                }
                case If: {
                    auto ifStmt = dynamic_cast<ifElseNode*>(stmt.get());
                    auto res = get_variables_from_expression(ifStmt->getCondition());
                    for(auto var : res) {
                        variables.insert(var);
                    }
                    break;
                }
                case Write: {
                    auto writeStmt = dynamic_cast<writeNode*>(stmt.get());
                    auto res = get_variables_from_expression(writeStmt->getExpr());
                    for(auto var : res) {
                        variables.insert(var);
                    }
                    break;
                }
                case Event: {
                    auto eventStmt = dynamic_cast<eventNode*>(stmt.get());
                    auto res = get_variables_from_expression(eventStmt->getCondition());
                    for(auto var : res) {
                        variables.insert(var);
                    }
                    break;
                }
                default: {

                }
            }
        }
    }

    std::set<variableNode> variables;

    std::pair<std::shared_ptr<basicblock>, int> concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{nullptr, 0};

private:
    static std::vector<variableNode> get_variables_from_expression(const expressionNode *expr) {
        std::vector<variableNode> vars{};
        while (expr) {
            switch (expr->getNodeType()) {
                case ArrayAccess: {
                    auto arrAcc = dynamic_cast<const arrayAccessNode*>(expr);
                    vars.push_back(variableNode(okType, arrAcc->getName()));
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
                    vars.push_back(*dynamic_cast<const variableNode*>(expr));
                    break;
                }
                default: {
                    break;
                }
            }
            if (!expr->getNext()) return vars;
            else expr = expr->getNext().get();
        }
    }
};


#endif //ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
