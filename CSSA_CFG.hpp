//
// Created by hu on 17/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
#define ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP

#include <basicblockTreeConstructor.hpp>

struct CSSA_CFG {
    std::shared_ptr<CCFG> ccfg;
    std::shared_ptr<DominatorTree> domTree;
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> symboltable;

    CSSA_CFG(const CCFG &_ccfg, std::shared_ptr<DominatorTree> _domTree, std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> table)
    : ccfg{std::make_shared<CCFG>(CCFG(_ccfg))}, domTree{std::move(_domTree)}, symboltable{std::move(table)} {
        update_mapstoMap(_ccfg.startNode, ccfg->startNode);
        std::cout << "here";
        ccfg->updateConflictEdges();
        //build_fud_chains();
        place_phi_functions();
    }

private:
    enum varType {use, define, phi};
    int counter = 0;
    std::map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> oldMapsTo;
    std::map<std::string, std::shared_ptr<statementNode>> currdefs;
    std::map<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> saveChain;
    std::map<std::shared_ptr<statementNode>, std::shared_ptr<statementNode>> chain;

    void update_mapstoMap(std::shared_ptr<basicblock> oldNode, std::shared_ptr<basicblock> newNode) {
        if (oldMapsTo.insert({oldNode, newNode}).second) {
            for (int i = 0; i < oldNode->nexts.size(); ++i) {
                update_mapstoMap(oldNode->nexts[i], newNode->nexts[i]);
            }
        }
    }

    void build_fud_chains() {
        for (const auto &v : *symboltable) {
            currdefs.insert({v.first, nullptr});
        }
        search(ccfg->startNode);
    }


    static std::vector<std::pair<std::string, varType>> getUsagesFromExpr(expressionNode *node) {
        std::vector<std::pair<std::string, varType>> vec;
        switch (node->getNodeType()) {
            case ArrayAccess: {
                auto arrAcc = dynamic_cast<arrayAccessNode*>(node);
                vec.emplace_back(arrAcc->getName(), use);
                auto res = getUsagesFromExpr(arrAcc->getAccessor());
                for (const auto &p : res) vec.push_back(p);
                break;
            } case ArrayLiteral: {
                auto arrLit = dynamic_cast<arrayLiteralNode*>(node);
                for (const auto &expr : arrLit->getArrLit()) {
                    auto res = getUsagesFromExpr(expr.get());
                    for(const auto &r : res) {
                        vec.push_back(r);
                    }
                }
                break;
            } case Variable: {
                vec.emplace_back(dynamic_cast<variableNode*>(node)->name, use);
                break;
            } case BinaryExpression: {
                auto binExpr = dynamic_cast<binaryExpressionNode*>(node);
                for (const auto &lexp : getUsagesFromExpr(binExpr->getLeft())) vec.push_back(lexp);
                for (const auto &rexp : getUsagesFromExpr(binExpr->getRight())) vec.push_back(rexp);
                break;
            } case UnaryExpression: {
                auto unExpr = dynamic_cast<unaryExpressionNode*>(node);
                for (const auto &expr : getUsagesFromExpr(unExpr->getExpr())) vec.push_back(expr);
                break;
            }
            default:
                break;
        }
        return vec;
    }

    static std::vector<std::tuple<std::shared_ptr<statementNode>, std::string, varType>> findUsages(std::shared_ptr<basicblock> &blk) {
        std::vector<std::tuple<std::shared_ptr<statementNode>, std::string, varType>> res;
        for (const auto &stmt : blk->statements) {
            switch (stmt->getNodeType()) {
                case Assign: {
                    auto assNode = dynamic_cast<assignNode*>(stmt.get());
                    res.emplace_back(stmt, assNode->getName(), define);
                    for (const auto &use : getUsagesFromExpr(assNode->getExpr()))
                        res.emplace_back(stmt, use.first, use.second);
                    break;
                } case AssignArrField: {
                    auto assArrF = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                    res.emplace_back(stmt, assArrF->getName(), define);
                    for (const auto &expr : getUsagesFromExpr(assArrF->getField()))
                        res.emplace_back(stmt, expr.first, expr.second);
                    for (const auto &expr : getUsagesFromExpr(assArrF->getExpr()))
                        res.emplace_back(stmt, expr.first, expr.second);
                    break;
                } case While: {
                    auto wNode = dynamic_cast<whileNode*>(stmt.get());
                    for (const auto &expr : getUsagesFromExpr(wNode->getCondition()))
                        res.emplace_back(stmt, expr.first, expr.second);
                    break;
                } case If: {
                    auto ifElse = dynamic_cast<ifElseNode*>(stmt.get());
                    for (const auto &expr : getUsagesFromExpr(ifElse->getCondition()))
                        res.emplace_back(stmt, expr.first, expr.second);
                    break;
                } case Write: {
                    auto wrNode = dynamic_cast<writeNode*>(stmt.get());
                    for (const auto &expr : getUsagesFromExpr(wrNode->getExpr()))
                        res.emplace_back(stmt, expr.first, expr.second);
                    break;
                } case Event: {
                    auto evNode = dynamic_cast<eventNode*>(stmt.get());
                    for (const auto &expr : getUsagesFromExpr(evNode->getCondition()))
                        res.emplace_back(stmt, expr.first, expr.second);
                    break;
                } case Phi: {
                    auto pNode = dynamic_cast<phiNode*>(stmt.get());
                    res.emplace_back(stmt, pNode->getName(), phi);
                    /*for (const auto &name : *pNode->get_variables())
                        res.emplace_back(stmt, name, use);*/
                    break;
                }
                default:
                    break;
            }
        }
        return res;

    } //variableUsageInStmt;


    void search(std::shared_ptr<basicblock> node) {
        auto usages = findUsages(node);

        for (const auto &r : usages) {
            std::string varname = std::get<1>(r);
            if (currdefs.find(varname) == currdefs.end()) currdefs.insert({varname, nullptr});
            if (std::get<2>(r) == use) {
                chain.insert({std::get<0>(r), currdefs[varname]});
            } else {
                saveChain.find(std::get<0>(r))->second = currdefs[varname];
            }
        }

        for (int index = 0; index < node->nexts.size(); ++index) {
            auto successor = node->nexts[index];
            auto newUsages = findUsages(successor);

            for (const auto &r : newUsages) {
                if (std::get<2>(r) == phi) {
                    auto pNode = dynamic_cast<phiNode*>(std::get<0>(r).get());
                    //pNode->get_variables()->at(index) = currdefs[std::get<1>(r)];
                }
            }
        }
        return;
    }

    void place_phi_functions() {
        for (const auto &b : ccfg->nodes) {                             //foreach b in N
            for (const auto &ed : ccfg->edges) {                        //foreach conflict edge
                if (ed.type == conflict && (ed.neighbours[1] == b)) {   //(a, b) do
                    std::shared_ptr<basicblock> a = ed.neighbours[0];
                    for (const auto &v : a->defines) {                  //v variable defined in a
                        if (b->uses.find(v.first) != b->uses.end()) {
                            bool hasPiFunction = false;
                            for (const auto &stmt : b->statements) {
                                if (stmt->getNodeType() == Pi && dynamic_cast<piNode*>(stmt.get())->getVar() == v.first) {
                                    hasPiFunction = true;
                                    break;
                                }
                            }
                            if (!hasPiFunction) {                       //if b does not have a pi function
                                std::vector<std::shared_ptr<statementNode>> vec;
                                vec.reserve(1+b->statements.size());
                                std::shared_ptr<statementNode> pi = std::make_shared<piNode>(piNode(v.first, counter,
                                        std::vector<std::string>{*(b->uses.find(v.first)->second.begin())}));
                                vec.push_back(pi);
                                for (const auto &stmt: b->statements) {
                                    rename(stmt, v.first, *(b->uses.find(v.first)->second.begin()), counter);
                                    vec.push_back(stmt);
                                }
                                ++counter;
                                b->statements = vec;
                            }
                            // if (n not in prec(s) incomplete, need prec(s) function
                            if (!b->statements.empty()) {
                                for (const auto &stmt : b->statements) {
                                    if (auto pi = dynamic_cast<piNode *>(stmt.get())) {
                                        std::string var = *(a->defines.find(v.first)->second.begin());
                                        if (pi->getVar() == v.first && !pi->contains(var)) {
                                            pi->addVariable(var);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    static void rename(const std::shared_ptr<statementNode> &stmt, const std::string &varName, const std::string &use, int counter) {
        std::string piName = "-T_" + varName + "_" + std::to_string(counter);
        switch (stmt->getNodeType()) {
            case Assign: {
                auto ass = dynamic_cast<assignNode*>(stmt.get());
                rename_expr(ass->getExpr(), use, piName);
                break;
            } case AssignArrField: {
                auto assArrF = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                rename_expr(assArrF->getField(), use, piName);
                rename_expr(assArrF->getExpr(), use, piName);
                break;
            } case While: {
                auto wNode = dynamic_cast<whileNode*>(stmt.get());
                rename_expr(wNode->getCondition(), use, piName);
                break;
            } case If: {
                auto ifNode = dynamic_cast<ifElseNode*>(stmt.get());
                rename_expr(ifNode->getCondition(), use, piName);
                break;
            } case Write: {
                auto wrNode = dynamic_cast<writeNode*>(stmt.get());
                rename_expr(wrNode->getExpr(), use, piName);
                break;
            } case Event: {
                auto eNode = dynamic_cast<eventNode*>(stmt.get());
                rename_expr(eNode->getCondition(), use, piName);
                break;
            } case Phi: {
                auto phi = dynamic_cast<phiNode*>(stmt.get());
                for (auto s : *phi->get_variables()) {
                    if (s == use) {s = piName;}
                }
                break;
            } default:
                break;
        }
    }

    static void rename_expr(expressionNode *expr, const std::string &use, const std::string &piName) {
        switch (expr->getNodeType()) {
            case ArrayAccess: {
                auto arrAcc = dynamic_cast<arrayAccessNode*>(expr);
                if (arrAcc->getName() == use) arrAcc->setName(piName);
                rename_expr(arrAcc->getAccessor(), use, piName);
                break;
            } case ArrayLiteral: {
                auto arrLit = dynamic_cast<arrayLiteralNode*>(expr);
                for (const auto &l : arrLit->getArrLit()) {
                    rename_expr(l.get(), use, piName);
                }
                break;
            } case Variable: {
                auto var = dynamic_cast<variableNode*>(expr);
                if (var->name == use) var->name = piName;
                break;
            } case BinaryExpression: {
                auto binExpr = dynamic_cast<binaryExpressionNode*>(expr);
                rename_expr(binExpr->getLeft(), use, piName);
                rename_expr(binExpr->getRight(), use, piName);
                break;
            } case UnaryExpression: {
                auto unExpr = dynamic_cast<unaryExpressionNode*>(expr);
                rename_expr(unExpr->getExpr(), use, piName);
                break;
            } default:
                break;
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
