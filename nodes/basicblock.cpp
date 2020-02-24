//
// Created by hu on 18/02/2020.
//

#include "basicblock.hpp"

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
    /*nexts = o.nexts;
    parents = o.parents;*/
    type = o.type;
    counterblocks++;
    return *this;
}

basicblock::basicblock(basicblock &&o) noexcept : statements{std::move(o.statements)}, nexts{std::move(o.nexts)}, parents{std::move(o.parents)}, type{o.type} {
    counterblocks++;
}

basicblock& basicblock::operator=(basicblock &&o) noexcept  {
    statements = std::move(o.statements);
    nexts = std::move(o.nexts);
    parents = std::move(o.parents);
    type = o.type;
    counterblocks++;
    return *this;
}

basicblock::~basicblock() {counterblocks--;}

int basicblock::get_number_of_blocks() {return counterblocks;}

void basicblock::setConcurrentBlock(const std::shared_ptr<basicblock> &blk, int threadNum, std::set<basicblock *> *whileLoop) {
    if (!statements.empty() && statements[0]->getNodeType() == Concurrent && this != blk.get()) {
        concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{blk, threadNum};
        for (int i = 0; i < nexts.size(); ++i) {
            nexts[i]->setConcurrentBlock(std::shared_ptr<basicblock>{this}, threadNum + i, whileLoop);
        }
    } else if (!statements.empty() && statements[0]->getNodeType() == Concurrent) {
        for (int i = 0; i < nexts.size(); ++i) {
            nexts[i]->setConcurrentBlock(blk, threadNum + i, whileLoop);
        }
    } else if (!statements.empty() && type == Coend && dynamic_cast<endConcNode*>(statements[0].get())->getConcNode() == blk.get()) {
        concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{blk, blk->concurrentBlock.second};
    } else {
        //If this block has already been visited (while loop), then stop recursion
        if (whileLoop && !this->statements.empty() && this->statements[0]->getNodeType() == While && !whileLoop->insert(this).second) {return;}
        concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{blk, threadNum};
        for (const auto &nxt : nexts) {
            nxt->setConcurrentBlock(blk, threadNum, whileLoop);
        }
    }
}

void basicblock::updateUsedVariables() {
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
                auto res = get_variables_from_expression(assArrStmt->getField());
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

std::string name = std::string{};

