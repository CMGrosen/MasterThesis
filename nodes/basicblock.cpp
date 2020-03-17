//
// Created by hu on 18/02/2020.
//

#include "basicblock.hpp"
#include <cassert>
#include <iostream>

basicblock::basicblock() : statements{}, nexts{} {counterblocks++;}
basicblock::basicblock(std::shared_ptr<statementNode> stmt) :
        statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
        nexts{} {counterblocks++;}
basicblock::basicblock(std::vector<std::shared_ptr<statementNode>> stmts) :
        statements{std::move(stmts)},
        nexts{} {counterblocks++;}
basicblock::basicblock(std::shared_ptr<statementNode> stmt, std::shared_ptr<basicblock> next) :
        statements{std::vector<std::shared_ptr<statementNode>>{std::move(stmt)}},
        nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}} {counterblocks++;}
basicblock::basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::shared_ptr<basicblock> next) :
        statements{std::move(stmts)},
        nexts{std::vector<std::shared_ptr<basicblock>>{std::move(next)}} {counterblocks++;}
basicblock::basicblock(std::vector<std::shared_ptr<statementNode>> stmts, std::vector<std::shared_ptr<basicblock>> nStmts) :
        statements{std::move(stmts)}, nexts{std::move(nStmts)} {counterblocks++;}


basicblock::basicblock(const basicblock &o) {
    for (auto stmt : o.statements) {
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
    for (auto stmt : o.statements) {
        statements.push_back(stmt->copy_statement());
    }
    type = o.type;
    uses = o.uses;
    defines = o.defines;
    type = o.type;
    depth = o.depth;

    counterblocks++;
    return *this;
}

basicblock::basicblock(basicblock &&o) noexcept : statements{std::move(o.statements)}, nexts{std::move(o.nexts)}, parents{std::move(o.parents)}, type{o.type}, uses{std::move(o.uses)}, defines{std::move(o.defines)}, depth{o.depth} {
    counterblocks++;
}

basicblock& basicblock::operator=(basicblock &&o) noexcept  {
    statements = std::move(o.statements);
    nexts = std::move(o.nexts);
    parents = std::move(o.parents);
    defines = std::move(o.defines);
    uses = std::move(o.uses);
    depth = o.depth;
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
    defines.clear();
    uses.clear();
    for (auto stmt : statements) {
        switch (stmt->getNodeType()) {
            case Assign: {
                auto assStmt = dynamic_cast<assignNode*>(stmt.get());
                auto pair = defines.insert({assStmt->getOriginalName(), std::set<std::string>{assStmt->getName()}});
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
                defines.insert({phi->getOriginalName(), std::set<std::string>{phi->getName()}});
            }
            default: {

            }
        }
    }
}

std::string basicblock::draw_picture(std::unordered_set<edge> *edges) {
    using std::string;
    std::set<basicblock *> drawnblocks;
    std::string res = string("\\usetikzlibrary{automata,positioning}\n") + string("\\begin{tikzpicture}[shorten >=1pt, node distance=2cm, on grid, auto]\n");

    std::pair<std::string, std::int32_t> statementsStringAndMaxWidth = statements_as_string();
    res += "\\node[state] (" + get_address() + ") [text width = " + std::to_string(statementsStringAndMaxWidth.second * 6 + 12) + "pt, rectangle] { \\texttt{" + statementsStringAndMaxWidth.first + "}};\n";
    drawnblocks.insert(this);

    res += draw_children(this, &drawnblocks);
    res += "\n";
    for (auto i = edges->begin(); i != edges->end(); ++i) {
        res += "\\path[" + (i->type == conflict ? string("->, red") : string("->")) + "] ("+ i->neighbours[0]->get_address() +") edge (" + i->neighbours[1]->get_address() + ");\n";
    }
    //res += "\n\n" + "\\path[->]";
    res += "\\end{tikzpicture}";
    return res;
}

std::pair<std::string, int> basicblock::draw_block(basicblock *parent, int current, int total, int distanceToNeighbour, std::set<basicblock *> *drawnBlocks) {
    using std::string;

    //If this block has already been visited (while loop), then stop recursion
    auto inserted = drawnBlocks->insert(this).second;
    if (!inserted) return std::pair<std::string, int>{"", 0};

    std::pair<std::string, std::int32_t> statementsStringAndMaxWidth = statements_as_string();
    int len = statementsStringAndMaxWidth.second * 6 + 12;

    string position;
    if (total > 2 && current != 0) {
        position = "right=of " + parent->nexts[current-1]->get_address();
    } else if (total > 2) {
        position = "below left=" + std::to_string(distanceToNeighbour+len) + "pt of " + parent->get_address();
    } else if (total == 2) {
        if (current == 0) {
            position = "below left=of " + parent->get_address();
        } else {
            position = "right=" + std::to_string(distanceToNeighbour+len) + "pt of " + parent->nexts[0]->get_address();
        }
    } else if (total == 1) {
        if (current == 0) {
            position = "below left=of " + parent->get_address();
        } else {
            position = "right=" + std::to_string(distanceToNeighbour+len) + "pt of " + parent->nexts[0]->get_address();
        }
    } else {
        position = "below=of " + parent->get_address();
    }
    string res = "\\node[state] (" + get_address() + ") [text width = " + std::to_string(len) + "pt, rectangle, " + position + "]";
    res += " {\\texttt{"  + statementsStringAndMaxWidth.first +  "}};\n";
    res += draw_children(this, drawnBlocks);
    return std::pair<std::string, int>{res, len};
}

std::string basicblock::draw_children(basicblock *parent, std::set<basicblock *> *drawnBlocks) {
    using std::string;

    string res;
    int dist = 0;
    for (auto i = 0; i < nexts.size(); i++) {
        auto pair = nexts[i]->draw_block(parent, i, nexts.size()-1, dist, drawnBlocks);
        res += pair.first;
        dist = pair.second;
    }

    return res;
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

std::pair<std::string, std::int32_t> basicblock::statements_as_string() {
    std::string res;
    std::string tmp = "";
    int32_t length = 0;
    for (auto stmt : statements) {
        tmp = stmt->to_string();
        if (tmp.length() > length) length = tmp.length();
        res += tmp + "; \\\\ ";
    }
    //call children
    return std::pair<std::string, std::int32_t>{res, length};
}


std::string basicblock::to_string() {
    std::string res;
    for (const auto &stmt : statements) {
        res += stmt->to_string() + "\\\\";
    }
    return res;
}

int32_t basicblock::get_stmt_length() {
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
int32_t basicblock::get_stmt_count() {
    if (type == Coend) {
        std::cout << "here";
    }
    return statements.size();
}

std::string name = std::string{};
