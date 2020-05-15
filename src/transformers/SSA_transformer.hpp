//
// Created by hu on 20/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SSA_TRANSFORMER_HPP
#define ANTLR_CPP_TUTORIAL_SSA_TRANSFORMER_HPP

#include <src/CFGs/SSA_CCFG.hpp>
//#include <dominatorTreeConstructor.hpp>
#include <src/transformers/lengauerTarjan.hpp>
#include <unordered_map>
#include <stack>
#include <list>

class SSA_TRANSFORMER {
public:
    static std::shared_ptr<SSA_CCFG> transform_CFG_to_SSAForm(const std::shared_ptr<CCFG>& _ccfg, std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> _symboltable) {
        auto form = SSA_TRANSFORMER(_ccfg, std::move(_symboltable));
        return form.ccfg;
    }

private:
    std::shared_ptr<SSA_CCFG> ccfg;
    int counter;

    SSA_TRANSFORMER(const std::shared_ptr<CCFG>& _ccfg, std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> _symboltable)
    : table{std::move(_symboltable)} {
        Variables.reserve(table->size());

        ccfg = std::make_shared<SSA_CCFG>(SSA_CCFG(*_ccfg));
        domTree = std::make_shared<DomTree>(DomTree(ccfg));

        initialise();

        place_phi_functions();

        rename(domTree->root);
        ccfg->readcount = Count["-readVal"];
        counter = Count["-b"];
        setSSA();

        insert_assignments_that_can_be_undefined(domTree->root);

        update_uses_and_defines();

        update_coend_nodes();

        //splitblocks_with_phinodes();

        //remove duplicate variables. Possibly a dumb idea
        //remove_duplicates();

    };

    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<expressionNode>>> table;
    std::vector<std::string> Variables;
    std::shared_ptr<DomTree> domTree;
    std::unordered_map<std::string, uint32_t> Count;
    std::unordered_map<std::string, std::stack<uint32_t>> Stack;
    std::unordered_map<std::string, std::list<std::shared_ptr<basicblock>>> defsites;
    std::unordered_map<std::shared_ptr<basicblock>, std::unordered_set<std::string>> Aorig; //variable definitions in block
    std::unordered_map<std::shared_ptr<basicblock>, std::unique_ptr<std::unordered_set<std::string>>> Aphi; //Does block have a phi function for variable
    //std::unordered_map<std::string, std::string> name_to_boolname; //useful later for symengine work for use with pinodes


    void initialise() {
        for (const auto &it : *table) {
            Count.insert({it.first, 0});
            Stack.insert({it.first, std::stack<uint32_t>{}});
            Stack.find(it.first)->second.push(0);
            defsites.insert({it.first, std::list<std::shared_ptr<basicblock>>{}});
            Variables.emplace_back(it.first);
        }
        Count.insert({"-readVal", 0});
        Count.insert({"-b", 1});

        //initialise Aorig and Aphi maps
        for (const auto &blk : ccfg->nodes) {
            std::unordered_set<std::string> variables;
            if (!blk->statements.empty()) {
                for (const auto &stmt : blk->statements) {
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
                        for (size_t i = 0; i < Y->parents.size(); ++i) args.push_back(var);
                        Type t = table->find(var)->second->getType();
                        std::shared_ptr<statementNode> stmt = std::make_shared<phiNode>(phiNode(t, var, &args));
                        stmts.push_back(stmt);
                        for(const auto &s : Y->statements) stmts.push_back(s);
                        Y->statements = stmts;
                        auto res = Y->defines.insert({var, std::set<std::string>{}});

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

    void update_def_stmtNode(const std::shared_ptr<statementNode> &stmt, std::map<std::string, uint32_t> *defcounts, const std::shared_ptr<basicblock> &blk) {
        std::string name;
        std::string oldName;
        //stmt->set_boolname("-b_" + std::to_string(Count["-b"]));
        //ccfg->boolnameStatements.insert({stmt->get_boolname(), stmt});
        if (stmt->getNodeType() == Assign) {
            auto node = dynamic_cast<assignNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            oldName = node->getName();
            name = oldName + "_" + std::to_string(i);
            node->setName(name);
            ccfg->defs.insert({name, blk});
        } else if (stmt->getNodeType() == Phi) {
            auto node = dynamic_cast<phiNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            oldName = node->getName();
            name = oldName + "_" + std::to_string(i);
            node->setName(name);
            ccfg->defs.insert({name, blk});
        } else if (stmt->getNodeType() == AssignArrField) {
            auto node = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
            int i = ++Count[node->getName()];
            Stack[node->getName()].push(i);
            auto res = defcounts->insert({node->getName(), 0});
            if (!res.second) res.first->second++;
            oldName = node->getName();
            name = oldName + "_" + std::to_string(i);
            node->setName(name);
            ccfg->defs.insert({name, blk});
        }
        ++Count["-b"];
        //blk->defines.find(oldName)->second.insert(name);
    }

    void update_uses_exprNode(std::shared_ptr<basicblock> &blk, expressionNode *expr) {
        switch (expr->getNodeType()) {
            case ArrayAccess: {
                auto node = reinterpret_cast<arrayAccessNode*>(expr);
                std::string oldName = node->getName();
                std::string name = oldName + ("_" + std::to_string(Stack[node->getName()].top()));
                node->setName(name);
                //blk->uses.find(oldName)->second.insert(name);
                update_uses_exprNode(blk, node->getAccessor());
                break;
            } case ArrayLiteral: {
                auto node = reinterpret_cast<arrayLiteralNode*>(expr);
                for (auto &e : node->getArrLit()) {
                    update_uses_exprNode(blk, e.get());
                }
                break;
            } case Variable: {
                auto node = reinterpret_cast<variableNode*>(expr);
                std::string oldName = node->name;
                std::string name = oldName + ("_" + std::to_string(Stack[node->name].top()));
                node->name = name;
                //blk->uses.find(oldName)->second.insert(name);
                break;
            } case BinaryExpression: {
                auto binExpr = reinterpret_cast<binaryExpressionNode*>(expr);
                update_uses_exprNode(blk, binExpr->getLeft());
                update_uses_exprNode(blk, binExpr->getRight());
                break;
            } case UnaryExpression: {
                auto unExpr = reinterpret_cast<unaryExpressionNode*>(expr);
                update_uses_exprNode(blk, unExpr->getExpr());
                break;
            } case Read: {
                auto readExpr = reinterpret_cast<readNode*>(expr);
                std::string name = "-readVal_" + std::to_string(++Count["-readVal"]);
                readExpr->setName(name);
                ccfg->defs.insert({name, blk});
            }
            default:
                break;
        }
    }
    void update_uses_stmtNode(std::shared_ptr<basicblock> &blk, const std::shared_ptr<statementNode> &stmt) {
        switch (stmt->getNodeType()) { // for each use
            case Assign: {
                auto node = reinterpret_cast<assignNode*>(stmt.get());
                update_uses_exprNode(blk, node->getExpr());
                break;
            } case AssignArrField: {
                auto node = reinterpret_cast<arrayFieldAssignNode*>(stmt.get());
                update_uses_exprNode(blk, node->getField());
                update_uses_exprNode(blk, node->getExpr());
                break;
            } case While: {
                auto node = reinterpret_cast<whileNode*>(stmt.get());
                update_uses_exprNode(blk, node->getCondition());
                break;
            } case If: {
                auto node = reinterpret_cast<ifElseNode*>(stmt.get());
                update_uses_exprNode(blk, node->getCondition());
                break;
            } case Write: {
                auto node = reinterpret_cast<writeNode*>(stmt.get());
                update_uses_exprNode(blk, node->getExpr());
                break;
            } case Event: {
                auto node = reinterpret_cast<eventNode*>(stmt.get());
                update_uses_exprNode(blk, node->getCondition());
                break;
            } default:
                break;
        }
    }

    void rename(const std::shared_ptr<DOMNode> &n) {
        std::map<std::string, uint32_t> defcounts;
        for (const auto &stmt : n->basic_block->statements) {
            if (stmt->getNodeType() != Phi) {
                update_uses_stmtNode(n->basic_block, stmt);
            }
            update_def_stmtNode(stmt, &defcounts, n->basic_block);
        }
        for (const auto &successor : n->basic_block->nexts) {
            // Suppose n is the j'th predecessor of successor
            size_t j;
            for (j = 0; j < successor->parents.size(); ++j)
                if (successor->parents[j].lock() == n->basic_block)
                    break;
            for (const auto &stmt : successor->statements) {
                if (stmt->getNodeType() == Phi) {
                    // Suppose the j'th operand of the phi-function is 'a'
                    auto phi = reinterpret_cast<phiNode*>(stmt.get());
                    std::string a = phi->get_variables()->at(j).var;
                    std::string newname = a + "_" + std::to_string(Stack[a].top());

                    auto boolnameVarPos = ccfg->defs.find(newname);
                    if (boolnameVarPos != ccfg->defs.end())
                        phi->update_variableindex(j, {newname, boolnameVarPos->second->get_name()});
                    else
                        phi->update_variableindex(j, {newname, ccfg->startNode->get_name()});
                }
            }
        }
        for (const auto &child : n->children) {
            rename(child);
        }
        for (const auto &def : defcounts) {
            Stack[def.first].pop();
        }
    }

    void setSSA() {
        for (const auto &blk : ccfg->nodes) {
            if (!blk->statements.empty()) {
                for (const auto &stmt : blk->statements) {
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

    void update_coend_nodes() {
        ccfg->update_defs();
        for (const auto &phiN : Aphi) {
            if (phiN.first->type == Coend) {
                for (const auto &stmt : phiN.first->statements) {
                    if (auto phi = dynamic_cast<phiNode *>(stmt.get())) {
                        auto vars = *phi->get_variables();
                        std::vector<option> res;
                        for (const auto &var : vars) {
                            auto conc = phiN.first->concurrentBlock.first;
                            basicblock *current_concnode = ccfg->defs[var.var].get();
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
    }

    void insert_assignments_that_can_be_undefined(const std::shared_ptr<DOMNode> &n) {
        for (auto &fi : ccfg->fiNodes) {
            for (auto &stmt : fi->statements) {
                if (stmt->getNodeType() == Phi) {
                    auto phi = reinterpret_cast<phiNode*>(stmt.get());
                    for (auto &var : *phi->get_variables()) {
                        if (*var.var.rbegin() == '0' && *(var.var.rbegin()+1) == '_') {
                            std::string name = var.var.substr(0, var.var.size()-2);
                            auto ptr = table->find(name);
                            if (ptr != table->end()) {
                                Type t = ptr->second->getType();
                                var.block_boolname = ccfg->startNode->get_name();
                                var.var_boolname = ccfg->startNode->get_name();
                                if(ccfg->defs.insert({var.var, ccfg->startNode}).second) {
                                    std::string tr = "true";
                                    std::shared_ptr<assignNode> s = std::make_shared<assignNode>(
                                            assignNode(t, name, std::make_shared<literalNode>(
                                                    literalNode(t, t == intType ? "0" : tr)), -1
                                            ));
                                    s->setSSA(true);
                                    s->setName(var.var);
                                    ccfg->startNode->statements.push_back(s);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_SSA_TRANSFORMER_HPP
