//
// Created by hu on 20/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
#define ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP

#include <basicblockTreeConstructor.hpp>
//#include <dominatorTreeConstructor.hpp>
#include <lengauerTarjan.hpp>
#include <unordered_map>
#include <stack>

struct SSA_CCFG {
    std::shared_ptr<CCFG> ccfg;
    SSA_CCFG(std::shared_ptr<CCFG> _ccfg, std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> _symboltable, std::shared_ptr<DomTree> _domTree)
    : ccfg{std::move(_ccfg)}, domTree{std::move(_domTree)}, table{std::move(_symboltable)} {
        Variables.reserve(table->size());

        initialise();

        place_phi_functions();

        rename(domTree->root);

        setSSA();

        update_uses_and_defines();

        splitblocks_with_phinodes();

        std::cout << "hej";

        //remove duplicate variables. Possibly a dumb idea
        //remove_duplicates();

    };

private:
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> table;
    std::vector<std::string> Variables;
    std::shared_ptr<DomTree> domTree;
    std::unordered_map<std::string, uint32_t> Count;
    std::unordered_map<std::string, std::stack<uint32_t>> Stack;
    std::unordered_map<std::string, std::list<std::shared_ptr<basicblock>>> defsites;
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::string>> Aorig; //variable definitions in block
    std::unordered_map<std::shared_ptr<basicblock>, std::unique_ptr<std::unordered_set<std::string>>> Aphi; //Does block have a phi function for variable


    void initialise() {
        for (const auto &it : *table) {
            Count.insert({it.first, 0});
            Stack.insert({it.first, std::stack<uint32_t>{}});
            Stack.find(it.first)->second.push(0);
            defsites.insert({it.first, std::list<std::shared_ptr<basicblock>>{}});
            Variables.emplace_back(it.first);
        }

        //initialise Aorig and Aphi maps
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
                    if (Aphi.find(Y) == Aphi.end()) {
                        Aphi.insert({Y,std::make_unique<std::unordered_set<std::string>>(std::unordered_set<std::string>{})});
                    }
                    if (Aphi.find(Y)->second->insert(var).second) {
                        // Y is not in Aphi[n], do this block and insert (lines 10->)
                        std::vector<std::shared_ptr<statementNode>> stmts;
                        std::vector<std::string> args;
                        for (auto i = 0; i < Y->parents.size(); ++i) args.push_back(var);
                        Type t = table->find(var)->second->getType();
                        std::shared_ptr<statementNode> stmt = std::make_shared<phiNode>(phiNode(t, var, args));
                        stmts.push_back(stmt);
                        for(const auto &s : Y->statements) stmts.push_back(s);
                        Y->statements = stmts;
                        auto res = Y->defines.insert({var, std::set<std::string>{}});
                        if (!res.second) {
                            std::cout << "here";
                        }

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

    void update_def_stmtNode(std::shared_ptr<basicblock> &blk, std::shared_ptr<statementNode> stmt, std::map<std::string, uint32_t> *defcounts) {
        std::string name;
        std::string oldName;
        if (stmt->getNodeType() == Assign) {
            auto node = dynamic_cast<assignNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            oldName = node->getName();
            name = oldName + "_" + std::to_string(i);
            node->setName(name);

        } else if (stmt->getNodeType() == Phi) {
            auto node = dynamic_cast<phiNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            oldName = node->getName();
            name = oldName + "_" + std::to_string(i);
            node->setName(name);
        } else if (stmt->getNodeType() == AssignArrField) {
            auto node = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            oldName = node->getName();
            name = oldName + "_" + std::to_string(i);
            node->setName(name);
        } else return;
        //blk->defines.find(oldName)->second.insert(name);
    }

    void update_uses_exprNode(std::shared_ptr<basicblock> &blk, expressionNode *expr) {
        switch (expr->getNodeType()) {
            case ArrayAccess: {
                auto node = dynamic_cast<arrayAccessNode*>(expr);
                std::string oldName = node->getName();
                std::string name = oldName + ("_" + std::to_string(Stack[node->getName()].top()));
                node->setName(name);
                //blk->uses.find(oldName)->second.insert(name);
                update_uses_exprNode(blk, node->getAccessor());
                break;
            } case ArrayLiteral: {
                auto node = dynamic_cast<arrayLiteralNode*>(expr);
                for (auto &e : node->getArrLit()) {
                    update_uses_exprNode(blk, e.get());
                }
                break;
            } case Variable: {
                auto node = dynamic_cast<variableNode*>(expr);
                std::string oldName = node->name;
                std::string name = oldName + ("_" + std::to_string(Stack[node->name].top()));
                node->name = name;
                //blk->uses.find(oldName)->second.insert(name);
                break;
            } case BinaryExpression: {
                auto binExpr = dynamic_cast<binaryExpressionNode*>(expr);
                update_uses_exprNode(blk, binExpr->getLeft());
                update_uses_exprNode(blk, binExpr->getRight());
                break;
            } case UnaryExpression: {
                auto unExpr = dynamic_cast<unaryExpressionNode*>(expr);
                update_uses_exprNode(blk, unExpr->getExpr());
                break;
            }
            default:
                break;
        }
    }
    void update_uses_stmtNode(std::shared_ptr<basicblock> &blk, const std::shared_ptr<statementNode> &stmt) {
        switch (stmt->getNodeType()) { // for each use
            case Assign: {
                auto node = dynamic_cast<assignNode*>(stmt.get());
                update_uses_exprNode(blk, node->getExpr());
                break;
            } case AssignArrField: {
                auto node = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                update_uses_exprNode(blk, node->getField());
                update_uses_exprNode(blk, node->getExpr());
                break;
            } case While: {
                auto node = dynamic_cast<whileNode*>(stmt.get());
                update_uses_exprNode(blk, node->getCondition());
                break;
            } case If: {
                auto node = dynamic_cast<ifElseNode*>(stmt.get());
                update_uses_exprNode(blk, node->getCondition());
                break;
            } case Write: {
                auto node = dynamic_cast<writeNode*>(stmt.get());
                update_uses_exprNode(blk, node->getExpr());
                break;
            } case Event: {
                auto node = dynamic_cast<eventNode*>(stmt.get());
                update_uses_exprNode(blk, node->getCondition());
                break;
            } default:
                break;
        }
    }

    void rename(std::shared_ptr<DOMNode> n) {
        std::map<std::string, uint32_t> defcounts;
        for (auto stmt : n->basic_block->statements) {
            if (stmt->getNodeType() != Phi) {
                update_uses_stmtNode(n->basic_block, stmt);
            }
            update_def_stmtNode(n->basic_block, stmt, &defcounts);
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

    void setSSA() {
        for (const auto blk : ccfg->nodes) {
            if (!blk->statements.empty()) {
                for (const auto stmt : blk->statements) {
                    stmt->setSSA(true);
                }
            }
        }
    }

    void update_uses_and_defines() {
        for (const auto &blk : ccfg->nodes) {
            blk->updateUsedVariables();
        }
    }

    static int get_num_from_string(const std::string &str) {
        int i = 0;
        int pos = 1;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            if (*it == '_') {
                return i;
            } else {
                i += (*it-'0') * pos;
                pos *= 10;
            }
        }
        return i;
    }

    void splitblocks_with_phinodes() {
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> splitConcnodes;
        for (const auto &phiN : Aphi) {
            if (phiN.first->type != Coend) {
                std::vector<std::shared_ptr<statementNode>> stmts;
                if (phiN.second->size() != phiN.first->statements.size()) {
                    for (auto i = phiN.second->size(); i < phiN.first->statements.size(); ++i) {
                        stmts.push_back(phiN.first->statements[i]);
                    }

                    std::shared_ptr<basicblock> blk = std::make_shared<basicblock>(basicblock(*phiN.first));
                    phiN.first->statements.resize(phiN.second->size());
                    blk->statements = std::move(stmts);
                    for (std::shared_ptr<basicblock> &nxt : phiN.first->nexts) {
                        blk->nexts.push_back(nxt);
                        ccfg->edges.erase({flow, phiN.first, nxt});
                        ccfg->edges.insert({flow, blk, nxt});
                        for (auto &parent : nxt->parents) {
                            if (parent.lock() == phiN.first) {
                                parent = blk;
                                break;
                            }
                        }
                    }
                    blk->parents = std::vector<std::weak_ptr<basicblock>>{phiN.first};
                    phiN.first->nexts = std::vector<std::shared_ptr<basicblock>>{blk};

                    blk->updateUsedVariables();
                    phiN.first->updateUsedVariables();

                    blk->concurrentBlock = phiN.first->concurrentBlock;
                    blk->setIfParents(phiN.first->getIfParents());

                    if (phiN.first->type == Cobegin) {
                        splitConcnodes.insert({phiN.first, blk});
                    }
                    phiN.first->type = joinNode;
                    ccfg->edges.insert(edge(flow, phiN.first, blk));
                    ccfg->nodes.insert(blk);
                }
            }
        }
        ccfg->update_defs();
        for (const auto &phiN : Aphi) {
            if (phiN.first->type == Coend) {
                for (const auto &stmt : phiN.first->statements) {
                    if (auto phi = dynamic_cast<phiNode *>(stmt.get())) {
                        auto vars = *phi->get_variables();
                        std::vector<std::string> res;
                        for (const auto &var : vars) {
                            auto conc = phiN.first->concurrentBlock.first;
                            basicblock *current_concnode = ccfg->defs[var].get();
                            while (current_concnode) {
                                if (current_concnode == conc) {
                                    res.push_back(var);
                                    break;
                                }
                                current_concnode = current_concnode->concurrentBlock.first;
                            }
                        }
                        phi->set_variables(res);
                    }
                }
            }
        }
        for (const auto &pair : splitConcnodes) {
            for (const auto &blk : ccfg->nodes) {
                if (blk->concurrentBlock.first == pair.first.get()) {
                    blk->concurrentBlock.first = pair.second.get();
                }
                if (blk->type == Coend) {
                    if (auto coend = dynamic_cast<endConcNode*>(blk->statements.back().get())) {
                        if (coend->getConcNode() == pair.first) {
                            coend->setConcNode(pair.second);
                        }
                    }
                }
            }
        }
                /*
                for (const auto &stmt : phiN.first->statements) {
                    if (auto phi = dynamic_cast<phiNode *>(stmt.get())) {
                        auto vars = *phi->get_variables();
                        std::vector<std::string> res;
                        int current = 0;
                        for (const auto &var : vars) {
                            int num = get_num_from_string(var);
                            if (current < get_num_from_string(var)) {
                                res.push_back(var);
                                current = num;
                            }
                        }
                        phi->set_variables(res);
                    }
                }*/
    }

    void remove_duplicates() {
        for (const auto &blk : Aphi) {
            for (const auto &stmt : blk.first->statements) {
                if (auto phi = dynamic_cast<phiNode*>(stmt.get())) {
                    std::set<std::string> names;
                    std::vector<std::string> updateNames;
                    for (const auto &name : *phi->get_variables()) {
                        names.insert(name);
                    }
                    updateNames.reserve(names.size());
                    for (const auto &name : names) {
                        updateNames.push_back(name);
                    }
                    phi->set_variables(std::move(updateNames));
                }
            }
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_SSA_CCFG_HPP
