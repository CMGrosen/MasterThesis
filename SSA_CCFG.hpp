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
            Stack.insert({it.first, std::stack<uint32_t>{}});
            Stack.find(it.first)->second.push(0);
            defsites.insert({it.first, std::list<std::shared_ptr<basicblock>>{}});
            Variables.emplace_back(it.first);
        }

        init_AorigMap_and_Aphi();

        place_phi_functions();

        rename(domTree->root);

        for (const auto blk : ccfg->nodes) {
            if (!blk->statements.empty()) {
                for (const auto stmt : blk->statements) {
                    stmt->setSSA(true);
                }
            }
        }
    };

private:
    std::vector<std::string> Variables;
    std::shared_ptr<DominatorTree> domTree;
    std::unordered_map<std::string, uint32_t> Count;
    std::unordered_map<std::string, std::stack<uint32_t>> Stack;
    std::unordered_map<std::string, std::list<std::shared_ptr<basicblock>>> defsites;
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::string>> Aorig; //variable definitions in block
    std::unordered_map<std::shared_ptr<basicblock>, std::unique_ptr<std::unordered_set<std::string>>> Aphi; //Does block have a phi function for variable

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
            Aphi.insert({blk, std::make_unique<std::unordered_set<std::string>>(std::unordered_set<std::string>{})});
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
                    std::unordered_set<std::string> *aphi = Aphi.find(Y)->second.get();
                    if (aphi->insert(var).second) { // Y is not in Aphi[n], do this block and insert (lines 10->)
                        std::vector<std::shared_ptr<statementNode>> stmts;
                        std::vector<std::string> args;
                        for (auto i = 0; i < Y->parents.size(); ++i) args.push_back(var);
                        std::shared_ptr<statementNode> stmt = std::make_shared<phiNode>(phiNode(var, args));
                        stmts.push_back(stmt);
                        for(const auto &s : Y->statements) stmts.push_back(s);
                        Y->statements = stmts;

                        bool found = false;
                        for (const auto &i : Worklist) {
                            // Possibly not correct, but code looks the node up in Aorig
                            // and adds to worklist if it's not in it.
                            // However, worklist is initialized to defsites, for the current variable
                            // Which iterates over all blocks in Aorig.
                            // So add the block Y to worklist if it's not already in it. Should be similar to pseudo-code
                            if (Y == i) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) Worklist.emplace_back(Y);
                    }
                }
            }
        }
    }

    void update_def_stmtNode(std::shared_ptr<statementNode> stmt, std::map<std::string, uint32_t> *defcounts) {
        if (stmt->getNodeType() == Assign) {
            auto node = dynamic_cast<assignNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            node->setName(node->getName() + "_" + std::to_string(i));
        } else if (stmt->getNodeType() == Phi) {
            auto node = dynamic_cast<phiNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            node->setName(node->getName() + "_" + std::to_string(i));
        } else if (stmt->getNodeType() == AssignArrField) {

        }
    }

    void update_uses_exprNode(expressionNode *expr) {
        if(expr) {
            switch (expr->getNodeType()) {
                case ArrayAccess: {
                    auto node = dynamic_cast<arrayAccessNode*>(expr);
                    //
                    break;
                } case ArrayLiteral: {
                    auto node = dynamic_cast<arrayLiteralNode*>(expr);
                    for (auto &e : node->getArrLit()) {
                        update_uses_exprNode(e.get());
                    }
                    break;
                } case Variable: {
                    auto node = dynamic_cast<variableNode*>(expr);
                    node->name += ("_" + std::to_string(Stack[node->name].top()));
                    break;
                } default:
                    break;
            }
            update_uses_exprNode(expr->getNext().get());
        }
    }
    void update_uses_stmtNode(const std::shared_ptr<statementNode> &stmt) {
        switch (stmt->getNodeType()) { // for each use
            case Assign: {
                auto node = dynamic_cast<assignNode*>(stmt.get());
                update_uses_exprNode(node->getExpr());
                break;
            } case AssignArrField: {
                auto node = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                //update_uses_exprNode()
                break;
            } case While: {
                auto node = dynamic_cast<whileNode*>(stmt.get());
                update_uses_exprNode(node->getCondition());
                break;
            } case If: {
                auto node = dynamic_cast<ifElseNode*>(stmt.get());
                update_uses_exprNode(node->getCondition());
                break;
            } case Write: {
                auto node = dynamic_cast<writeNode*>(stmt.get());
                update_uses_exprNode(node->getExpr());
                break;
            } case Event: {
                auto node = dynamic_cast<eventNode*>(stmt.get());
                update_uses_exprNode(node->getCondition());
                break;
            } default:
                break;
        }
    }

    void rename(std::shared_ptr<DOMNode> n) {
        std::map<std::string, uint32_t> defcounts;
        for (auto stmt : n->basic_block->statements) {
            if (stmt->getNodeType() != Phi) {
                update_uses_stmtNode(stmt);
            }
            update_def_stmtNode(stmt, &defcounts);
        }
        for (auto successor : n->basic_block->nexts) {
            // Suppose n is the j'th predecessor of successor
            uint32_t j;
            for (j = 0; j < successor->parents.size(); ++j)
                if (successor->parents[j].lock() == n->basic_block)
                    break;
            for (auto stmt : successor->statements) {
                if (stmt->getNodeType() == Phi) {
                    // Suppose the j'th operand of the phi-function is 'a'
                    auto phi = dynamic_cast<phiNode*>(stmt.get());
                    std::string a = phi->get_variables()->at(j);
                    phi->get_variables()->at(j) += ("_" + std::to_string(Stack[a].top()));
                }
            }
        }
        for (auto child : n->children) {
            rename(child);
        }
        for (const auto &def : defcounts) {
            Stack[def.first].pop();
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
