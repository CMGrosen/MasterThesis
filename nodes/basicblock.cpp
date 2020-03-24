//
// Created by hu on 18/02/2020.
//

#include "basicblock.hpp"
#include <cassert>
#include <iostream>

basicblock::basicblock() : statements{}, nexts{}, depth{0}, type{Compute} {counterblocks++;}
basicblock::basicblock(std::shared_ptr<statementNode> stmt) :
        statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
        nexts{}, depth{0}, type{Compute} {counterblocks++;}
basicblock::basicblock(std::vector<std::shared_ptr<statementNode>> stmts) :
        statements{std::move(stmts)},
        nexts{}, depth{0}, type{Compute} {counterblocks++;}
basicblock::basicblock(std::shared_ptr<statementNode> stmt, std::shared_ptr<basicblock> next) :
        statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
        nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}},
        depth{0}, type{Compute} {counterblocks++;}
basicblock::basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::shared_ptr<basicblock> next) :
        statements{std::move(stmts)},
        nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}},
        depth{0}, type{Compute} {counterblocks++;}
basicblock::basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::vector<std::shared_ptr<basicblock>> nStmts) :
        statements{std::move(stmts)}, nexts{std::move(nStmts)}, depth{0}, type{Compute} {counterblocks++;}


basicblock::basicblock(const basicblock &o) {
    for (const auto &stmt : o.statements) {
        statements.push_back(stmt->copy_statement());
    }
    type = o.type;
    uses = o.uses;
    defines = o.defines;
    type = o.type;
    depth = o.depth;
    /*
    for (auto nxt : o.nexts) {
        nexts.push_back(std::make_shared<basicblock>(basicblock(*nxt)));
    }*/
   counterblocks++;
}

basicblock& basicblock::operator=(const basicblock &o) {
    for (const auto &stmt : o.statements) {
        statements.push_back(stmt->copy_statement());
    }
    depth = o.depth;
    uses = o.uses;
    defines = o.defines;
    type = o.type;
    counterblocks++;
    return *this;
}

basicblock::basicblock(basicblock &&o) noexcept : statements{std::move(o.statements)}, parents{std::move(o.parents)}, nexts{std::move(o.nexts)}, depth{o.depth}, uses{std::move(o.uses)}, defines{std::move(o.defines)}, type{o.type} {
    counterblocks++;
}

basicblock& basicblock::operator=(basicblock &&o) noexcept  {
    statements = std::move(o.statements);
    parents = std::move(o.parents);
    nexts = std::move(o.nexts);
    depth = o.depth;
    uses = std::move(o.uses);
    defines = std::move(o.defines);
    type = o.type;
    counterblocks++;
    return *this;
}

basicblock::~basicblock() {counterblocks--;}

int basicblock::get_number_of_blocks() {return counterblocks;}

void basicblock::setConcurrentBlock(basicblock *blk, int threadNum, std::set<basicblock *> *whileLoop) {
    if (type == Cobegin) {
        concurrentBlock = {blk, threadNum};
        int i = threadNum;
        for (const auto &nxt : nexts) {
            nxt->setConcurrentBlock(this, i++, whileLoop);
        }
        auto nxt = nexts[0];
        while (nxt) {
            if (auto endconc = dynamic_cast<endConcNode *>(nxt->statements.back().get())) {
                if (endconc->getConcNode().get() == this) {
                    break;
                }
            }
            nxt = nxt->nexts[0];
        }
        assert(nxt);
        for (const auto &n : nxt->nexts) {
            n->setConcurrentBlock(blk, threadNum, whileLoop);
        }
    } else if (type == Coend) {
        concurrentBlock = {blk, threadNum};
    } else {
        concurrentBlock = {blk, threadNum};
        for (const auto &nxt : nexts) {
            nxt->setConcurrentBlock(blk, threadNum, whileLoop);
        }
    }
}

void basicblock::updateUsedVariables() {
    defmapping.clear();
    defines.clear();
    uses.clear();
    for (const auto &stmt : statements) {
        switch (stmt->getNodeType()) {
            case Assign: {
                auto assStmt = dynamic_cast<assignNode*>(stmt.get());
                auto pair = defines.insert({assStmt->getOriginalName(), std::set<std::string>{assStmt->getName()}});
                defmapping.insert({assStmt->getName(), assStmt->getOriginalName()});
                if(!pair.second) {
                    pair.first->second.insert(assStmt->getName());
                }

                auto expr = assStmt->getExpr();
                auto res = get_variables_from_expression(expr);
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }
                break;
            }
            case AssignArrField: {
                auto assArrStmt = dynamic_cast<arrayFieldAssignNode*>(stmt.get());
                auto pair = defines.insert({assArrStmt->getOriginalName(), std::set<std::string>{assArrStmt->getName()}});
                defmapping.insert({assArrStmt->getName(), assArrStmt->getOriginalName()});

                if(!pair.second) {
                    pair.first->second.insert(assArrStmt->getName());
                }

                auto res = get_variables_from_expression(assArrStmt->getField());
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }

                res = get_variables_from_expression(assArrStmt->getExpr());
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }
                break;
            }
            case While: {
                auto whileStmt = dynamic_cast<whileNode*>(stmt.get());
                auto res = get_variables_from_expression(whileStmt->getCondition());
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }
                break;
            }
            case If: {
                auto ifStmt = dynamic_cast<ifElseNode*>(stmt.get());
                auto res = get_variables_from_expression(ifStmt->getCondition());
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }
                break;
            }
            case Write: {
                auto writeStmt = dynamic_cast<writeNode*>(stmt.get());
                auto res = get_variables_from_expression(writeStmt->getExpr());
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }
                break;
            }
            case Event: {
                auto eventStmt = dynamic_cast<eventNode*>(stmt.get());
                auto res = get_variables_from_expression(eventStmt->getCondition());
                for(auto var : res) {
                    auto usepair = uses.insert({var->origName, std::set<std::string>{var->name}});
                    if (!usepair.second) {
                        usepair.first->second.insert(var->name);
                    }
                }
                break;
            }
            case Phi: {
                auto phi = dynamic_cast<phiNode*>(stmt.get());
                auto res = defines.insert({phi->getOriginalName(), std::set<std::string>{phi->getName()}});
                defmapping.insert({phi->getName(), phi->getOriginalName()});
                if (!res.second) res.first->second.insert(phi->getName());
                break;
            }
            case Pi: {
                auto pi = dynamic_cast<piNode*>(stmt.get());
                auto res = defines.insert({pi->getVar(), std::set<std::string>{pi->getName()}});
                defmapping.insert({pi->getName(), pi->getVar()});
                if (!res.second) res.first->second.insert(pi->getName());
                break;
            }
            default: {

            }
        }
    }
}

std::string basicblock::get_name() {
    return get_address();
}

int basicblock::counterblocks = 0;

std::string basicblock::get_address() {
    if (name.empty()) {
        const void *address = static_cast<const void *>(this);
        std::stringstream ss;
        ss << address;
        return ss.str();
    } else return name;
}


std::string basicblock::to_string() {
    std::string res;
    for (const auto &stmt : statements) {
        res += stmt->to_string() + "\\\\";
    }
    return res;
}

size_t basicblock::get_stmt_length() {
    int32_t longest_stmt = 0;
    for (const auto &stmt : statements) {
        std::string str = stmt->to_string();
        int32_t stmtlen = str.length();
        for (auto it = str.begin(); it != str.end(); ++it) {
            if (*it == '\\' || *it == '$' || *it == '{' || *it == '}' || (*it == '_' && *(it-1) != '\\')) {
                --stmtlen;
            }
        }
        for (auto it = str.begin(); it != str.end(); ++it) {
            if (*it == '$' && *(it+1) == '\\' && *(it+2) == 'p') {
                if (*(it+3) == 'h') {
                    it += 4;
                    stmtlen -= 2; //remove "phi" length, and replace with the greek symbol phi
                } else {
                    it += 3;
                    --stmtlen; //remove "pi" length, and replace with the greek symbol pi
                }
            }
        }
        if (longest_stmt < stmtlen) {
            longest_stmt = stmtlen;
        }
    }
    return longest_stmt;

}
size_t basicblock::get_stmt_count() {
    return statements.size();
}

std::string name = std::string{};
