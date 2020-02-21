//
// Created by hu on 20/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
#define ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP

#include <basicblockTreeConstructor.hpp>
#include <dominatorTreeConstructor.hpp>
#include <unordered_map>
#include <stack>

struct SSA_CCFG {
    std::shared_ptr<CCFG> ccfg;
    SSA_CCFG(std::shared_ptr<CCFG> _ccfg, std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> _symboltable, std::shared_ptr<DominatorTree> _domTree)
    : ccfg{std::move(_ccfg)}, domTree{std::move(_domTree)} {
        Variables.reserve(_symboltable->size());
        for (const auto &it : *_symboltable) {
            Count.insert({it.first, 0});
            Stack.insert({it.first, std::stack<std::string>{}});
            defsites.insert({it.first, std::list<std::shared_ptr<basicblock>>{}});
            Variables.emplace_back(it.first);
        }

        init_AorigMap_and_Aphi();

        place_phi_functions();

        rename(domTree->root);
    };

private:
    std::vector<std::string> Variables;
    std::shared_ptr<DominatorTree> domTree;
    std::unordered_map<std::string, int> Count;
    std::unordered_map<std::string, std::stack<std::string>> Stack;
    std::unordered_map<std::string, std::list<std::shared_ptr<basicblock>>> defsites;
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::string>> Aorig; //variable definitions in block
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::shared_ptr<basicblock>>> Aphi; //variable definitions in block

    void init_AorigMap_and_Aphi() {
        for (const auto &blk : ccfg->nodes) {
            std::unordered_set<std::string> variables;
            if (!blk->statements.empty()) {
                for (const auto stmt : blk->statements) {
                    if (stmt->getNodeType() == Assign) {
                        variables.insert(dynamic_cast<const assignNode *>(stmt.get())->getName());
                    } else if (stmt->getNodeType() == AssignArrField) {
                        variables.insert(dynamic_cast<const arrayFieldAssignNode *>(stmt.get())->getName());
                    }
                }
            }
            Aorig.insert({blk, variables});
            Aphi.insert({blk, std::unordered_set<std::shared_ptr<basicblock>>{}});
        }
    }

    void place_phi_functions() {
        for (const auto &blk : ccfg->nodes) {
            for (const std::string &var : Aorig.find(blk)->second) {
                defsites.find(var)->second.emplace_back(blk);
            }
        }

        for (const auto &var : Variables) {
            std::list<std::shared_ptr<basicblock>> Worklist = defsites[var];
            while (!Worklist.empty()) {
                std::shared_ptr<basicblock> blk = Worklist.front();
                Worklist.pop_front();
                for (const auto &Y : domTree->DF[blk]) {
                    if (Aphi[blk].insert(Y).second) { // Y is not in Aphi[n], do this block and insert (lines 10->)
                        std::vector<std::shared_ptr<statementNode>> stmts = Y->statements;
                        std::vector<std::string> args(Y->parents.size());
                        for (auto i = 0; i < Y->parents.size(); ++i) args.push_back(var);
                        std::shared_ptr<statementNode> stmt = std::make_shared<phiNode>(phiNode(var, args));

                        for (const auto &i : Worklist) {
                            // Possibly not correct, but code looks the node up in Aorig
                            // and adds to worklist if it's not in it.
                            // However, worklist is initialized to defsites, for the current variable
                            // Which iterates over all blocks in Aorig.
                            // So add the block Y to worklist if it's not already in it. Should be similar to pseudo-code
                            if (Y == i) {
                                Worklist.emplace_back(Y);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    void update_uses(expressionNode *expr) {
        if(expr) {
            switch (expr->getNodeType()) {
                case ArrayAccess: {
                    auto node = dynamic_cast<arrayAccessNode*>(expr);
                    break;
                } case ArrayLiteral: {
                    break;
                } case Variable: {
                    break;
                } default:
                    break;
            }
        }
    }
    void update_uses(const std::shared_ptr<statementNode> &stmt) {
        switch (stmt->getNodeType()) { // for each use
            case Assign: {
                auto node = dynamic_cast<assignNode*>(stmt.get());
                update_uses(node->getExpr());
                break;
            } case AssignArrField: {
                auto node = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                break;
            } case While: {
                auto node = dynamic_cast<whileNode*>(stmt.get());
                break;
            } case If: {
                auto node = dynamic_cast<ifElseNode*>(stmt.get());
                break;
            } case Write: {
                auto node = dynamic_cast<writeNode*>(stmt.get());
                break;
            } case ArrayAccess: {
                auto node = dynamic_cast<arrayAccessNode*>(stmt.get());
                break;
            } case Event: {
                auto node = dynamic_cast<eventNode*>(stmt.get());
                break;
            } default:
                break;
        }
    }

    void rename(std::shared_ptr<DOMNode> n) {
        for (auto stmt : n->basic_block->statements) {
            update_uses(stmt);

        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
