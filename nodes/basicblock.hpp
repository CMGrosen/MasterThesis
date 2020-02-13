//
// Created by hu on 10/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP
#define ANTLR_CPP_TUTORIAL_BASICBLOCK_HPP

#include <nodes/statements/arrayFieldAssignNode.hpp>
#include <nodes/statements/writeNode.hpp>
#include <nodes/statements/eventNode.hpp>
#include <string>
#include <algorithm>

struct basicblock;
enum edgeType {flow, conflict};

struct edge {
    edgeType type;
    std::vector<std::shared_ptr<basicblock>> neighbours;
    bool operator<(const edge& s) const { return neighbours[0] < s.neighbours[1];
        if (neighbours[0] == s.neighbours[1] || neighbours[1] == s.neighbours[0])
            return neighbours[0] < s.neighbours[1] && neighbours[1] < s.neighbours[0] && type < s.type;
        else
            return neighbours[0] < s.neighbours[0] && neighbours[1] < s.neighbours[1] && type < s.type;
    }
    bool operator==(const edge& s) const {
        if (neighbours[0] == s.neighbours[1])
            return neighbours[0] == s.neighbours[1] && neighbours[1] == s.neighbours[0] && type == s.type;
        else
            return neighbours[0] == s.neighbours[0] && neighbours[1] == s.neighbours[1] && type == s.type;
    }
    edge() : type{flow} {neighbours.reserve(2);}
    edge(std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
            type{flow}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}
    edge(edgeType typeOfEdge, std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
            type{typeOfEdge}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


namespace std {

    template<> struct hash<edge> {
        size_t operator()(const edge& s) const {
            size_t seed = 0;
            if (s.neighbours[0] < s.neighbours[1]) hash_combine(seed, s.neighbours[0]);
            else hash_combine(seed, s.neighbours[1]);
            hash_combine(seed, s.type);
            return seed;
        }
    };
}

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

    std::string draw_picture(std::unordered_set<edge> *edges) {
        using std::string;
        std::string res = string("\\usetikzlibrary{automata,positioning}\n") + string("\\begin{tikzpicture}[shorten >=1pt, node distance=2cm, on grid, auto]\n");

        std::pair<std::string, std::int32_t> statementsStringAndMaxWidth = statements_as_string();
        res += "\\node[state] (" + get_address() + ") [text width = " + std::to_string(statementsStringAndMaxWidth.second * 6 + 12) + "pt, rectangle] { \\texttt{" + statementsStringAndMaxWidth.first + "}};\n";

        res += draw_children(this);
        res += "\n";
        for (auto i = edges->begin(); i != edges->end(); ++i) {
            res += "\\path[->] ("+ i->neighbours[0]->get_address() +") edge (" + i->neighbours[1]->get_address() + ");\n";
        }
        //res += "\n\n" + "\\path[->]";
        res += "\\end{tikzpicture}";
        return res;
    }

    std::pair<std::string, int> draw_block(basicblock *parent, int current, int total, int distanceToNeighbour) {
        using std::string;

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
        res += draw_children(this);
        return std::pair<std::string, int>{res, len};
    }

    std::string draw_children(basicblock *parent) {
        using std::string;

        string res;
        int dist = 0;
        for (auto i = 0; i < nexts.size(); i++) {
            auto pair = nexts[i]->draw_block(parent, i, nexts.size()-1, dist);
            res += pair.first;
            dist = pair.second;
        }

        return res;
    }

    std::set<variableNode> variables;

    std::pair<std::shared_ptr<basicblock>, int> concurrentBlock = std::pair<std::shared_ptr<basicblock>, int>{nullptr, 0};


private:
    std::string get_address() {
        if (name.empty()) {
            const void *address = static_cast<const void *>(this);
            std::stringstream ss;
            ss << address;
            return ss.str();
        } else return name;
    }

    std::pair<std::string, std::int32_t> statements_as_string() {
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