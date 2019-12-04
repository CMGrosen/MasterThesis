
#ifndef DST_H
#define DST_H
#include "antlr4-runtime.h"
#include "SmallVisitor.h"
#include <nodes/nodes.hpp>
//#include "symengine/Constraint.hpp"


/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */


class  DST : public SmallVisitor {
public:
    std::unordered_map<std::string, std::shared_ptr<node>> symboltables;

    std::pair<const std::shared_ptr<node>, const std::unordered_map<std::string, std::shared_ptr<node>>> getTree(SmallParser::FileContext *ctx) {
        std::shared_ptr<node> a = visitStmts(ctx->stmts());
//        std::shared_ptr<statementNode> aNew = deepCopy(a.get());
        return std::pair<std::shared_ptr<node>, std::unordered_map<std::string, std::shared_ptr<node>>>(std::move(a), symboltables);
    }

    virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
        //antlrcpp::Any result = visitChildren(ctx);
        std::shared_ptr<node> a = visitStmts(ctx->stmts());

        //std::vector<std::shared_ptr<statementNode>> l = a->debug_getAllNodes();
        //std::cout << "\n\n\nwriting out what we have:\n";
  //      WriteType(a);
        //std::cout << dynamic_cast<literalNode*>(dynamic_cast<additionNode*>(a[0]->value)->getLeft())->value << std::endl;
        //a->setNextStatement(firstStatement);

        return a;
    }

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *ctx) override {
        if(ctx->stmts()) {
            std::shared_ptr<node> first = visitStmt(ctx->stmt());
            std::shared_ptr<node> last = first->getLast();
            if (!last) last = first;
            if (last->getNodeType() == While) {
                std::vector<std::shared_ptr<node>> tmp = last->getNexts();
                tmp[1] = visitStmts(ctx->stmts());
                last->setNexts(tmp);
            } else if (last->getNodeType() == If) {
                std::vector<std::shared_ptr<node>> tmp = last->getNexts();
                tmp[0] = tmp[1];
                tmp[1] = tmp[2];
                tmp.pop_back();
                auto lastTrue = tmp[0]->getLast();
                auto lastFalse = tmp[1]->getLast();
                if (!lastTrue) lastTrue = tmp[0];
                if (!lastFalse) lastFalse = tmp[1];
                std::shared_ptr<node> remainingStatements = visitStmts(ctx->stmts());
                lastTrue->setNext(remainingStatements);
                lastFalse->setNext(remainingStatements);
                last->setNexts(tmp);
            } else {
                last->setNext(visitStmts(ctx->stmts()));
            }
            /*
            while(last->getNext()) {
                last = last->getNext();
            }*/
            return first;
        } else {
            std::shared_ptr<node> result = visitStmt(ctx->stmt());
            return result;
        }
    }

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *ctx) override {
        if (ctx->assign()) {
            std::shared_ptr<node> stmt = visitAssign(ctx->assign());
            return stmt;
        } else if (ctx->write()) {
            std::shared_ptr<node> stmt = visitWrite(ctx->write());
            return stmt;
        } else if (ctx->iter()) {
            std::shared_ptr<node> stmt = visitIter(ctx->iter());
            return stmt;
        } else if (ctx->ifs()) {
            std::shared_ptr<node> stmt = visitIfs(ctx->ifs());
            return stmt;
        } else if (ctx->thread()) {
            std::shared_ptr<node> stmt = visitThread(ctx->thread());
            return stmt;
        } else { //only event is remaining
            std::shared_ptr<node> stmt = visitEvent(ctx->event());
            return stmt;
        }
    }

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *ctx) override {
        Type t;
        if (ctx->arrayAccess()) {
            std::shared_ptr<node> first = visitExpr(ctx->expr());
            std::shared_ptr<node> last = first->getLast();
            if (!last) last = first;/*
            while (last->getNext()) {
                last = last->getNext();
            }*/
            std::shared_ptr<node> arrAcc = visitArrayAccess(ctx->arrayAccess());
            std::shared_ptr<node> last2 = arrAcc->getLast();
            if (!last2) last2 = arrAcc;/*
            while (last2->getNext()) {
                last2 = last2->getNext();
            }*/
            if (last->getType() == arrayIntType || last->getType() == arrayBoolType) {
                t = errorType;
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                << "] Cannot assign an array to a field of an array\n";
            } else if (last2->getType() != intType) {
                t = errorType;
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                << "] Type mismatch in assignment. Expected field accessor to be of type " << info[intType] << " got " << info[last2->getType()] << "\n";
            } else if (last->getType() == errorType) {
                t = errorType;
            } else {
                t = okType;
            }
            std::shared_ptr<node> n = std::make_shared<node>(node(t, AssignArrField));
            last->setNext(arrAcc);
            last2->setNext(n);
            return first;
        } else if (ctx->NAME() && ctx->arrayLiteral()) {
            std::shared_ptr<node> first = visitArrayLiteral(ctx->arrayLiteral());
            std::shared_ptr<node> last = first->getLast();
            if (!last) last = first;/*
            while(last->getNext()) {
                last = last->getNext();
            }*/
            if (last->getType() == errorType) {
                t = errorType;
            } else {
                t = last->getType();
            }
            std::shared_ptr<node> n = std::make_shared<node>(node(t, Assign, ctx->NAME()->getText()));
            int length = std::stoi(last->getValue());
            if (symboltables.insert({ctx->NAME()->getText(), std::make_shared<constraintNode>(constraintNode(t))}).second) {
                while (length-->0)
                    symboltables.insert({(ctx->NAME()->getText() + "[" + std::to_string(length) + "]"), std::make_shared<constraintNode>(constraintNode(first->getType()))});
            } else {
                if (symboltables.find(ctx->NAME()->getText() + "[" + std::to_string(length) + "]") != symboltables.end()
                || (symboltables.find(ctx->NAME()->getText() + "[" + std::to_string(length-1) + "]") == symboltables.end())
                ) {
                    symboltables.find(ctx->NAME()->getText())->second->setType(errorType);
                }
                else {
                    while (length-->0)
                        symboltables.insert({(ctx->NAME()->getText() + "[" + std::to_string(length) + "]"), std::make_shared<constraintNode>(constraintNode(first->getType()))});
                }
            }
            last->setNext(n);
            return first;
        } else {
            std::shared_ptr<node> first = visitExpr(ctx->expr());
            std::shared_ptr<node> last = first->getLast();
            if (!last) last = first;/*
            while(last->getNext()) {
                last = last->getNext();
            }*/
            auto pair = symboltables.find(ctx->NAME()->getText());
            if (pair == symboltables.end()) {
                symboltables.insert({ctx->NAME()->getText(), std::make_shared<constraintNode>(constraintNode(last->getType()))});
                pair = symboltables.find(ctx->NAME()->getText());
            }
            if (pair->second->getType() == errorType) t = errorType; else t = okType;
            last->setNext(std::make_shared<node>(node(t,Assign, ctx->NAME()->getText())));
            return first;
        }
    }

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *ctx) override {
        Type t = okType;
        std::shared_ptr<node> first = visitExpr(ctx->expr());
        std::shared_ptr<node> lastInBoolExpr = first->getLast();
        if (!lastInBoolExpr) lastInBoolExpr = first;
        if (lastInBoolExpr->getType() != boolType) t = errorType;

        std::shared_ptr<node> firstStmt = visitStmts(ctx->scope()->stmts());
        std::shared_ptr<node> lastExecutableBitStmt = firstStmt->getLast();
        if (!lastExecutableBitStmt) lastExecutableBitStmt = firstStmt;

        std::shared_ptr<node> n = std::make_shared<node>(node(t, While));
        lastExecutableBitStmt->setNext(first);
        lastInBoolExpr->setNext(n);
        n->setNexts(std::vector<std::shared_ptr<node>>{first, nullptr});
        return n;
        /*std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        std::shared_ptr<statementNode> body = visitStmts(ctx->scope()->stmts());
        Type t = okType;
        if (condition->getType() != boolType || body->getType() != okType) t = errorType;
        std::shared_ptr<statementNode> res = std::make_shared<whileNode>(whileNode(t, condition, body));
        return res;*/
    }

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *ctx) override {
        std::shared_ptr<node> firstExpr = visitExpr(ctx->expr());
        std::shared_ptr<node> lastExpr = firstExpr->getLast();
        if (!lastExpr) lastExpr = firstExpr;

        std::shared_ptr<node> firstTrue = visitStmts(ctx->scope(0)->stmts());
        std::shared_ptr<node> lastTrue = firstTrue->getLast();
        if (!lastTrue) lastTrue = firstTrue;

        std::shared_ptr<node> firstFalse = visitStmts(ctx->scope(1)->stmts());
        std::shared_ptr<node> lastFalse = firstFalse->getLast();
        if (!lastFalse) lastFalse = firstFalse;

        Type t = okType;
        if (lastExpr->getType() != boolType || lastTrue->getType() != okType || lastFalse->getType() != okType) t = errorType;

        std::shared_ptr<node> n = std::make_shared<node>(node(t,If));
        lastExpr->setNext(n);
        n->setNexts(std::vector<std::shared_ptr<node>>{nullptr, firstTrue, firstFalse});
        return firstExpr;
       /* std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        std::shared_ptr<statementNode> trueBranch = visitStmts(ctx->scope(0)->stmts());
        std::shared_ptr<statementNode> falseBranch = visitStmts(ctx->scope(1)->stmts());
        Type t = okType;
        if (condition->getType() != boolType || trueBranch->getType() != okType || falseBranch->getType() != okType) t = errorType;
        std::shared_ptr<statementNode> res = std::make_shared<ifElseNode>(ifElseNode(t, condition, trueBranch, falseBranch));
        return res;*/
    }

    virtual antlrcpp::Any visitThread(SmallParser::ThreadContext *ctx) override {
        std::vector<std::shared_ptr<node>> statements;
        for (auto scopeContext : ctx->threads) {
            statements.push_back(visitStmts(scopeContext->stmts()));
        }
        Type t = okType;
        for (auto &statement : statements) {
            if (statement->getType() != okType) {
                t = errorType;
                break;
            }
        }
        std::shared_ptr<node> res = std::make_shared<concurrentNode>(concurrentNode(t, statements));
        return res;
    }

    virtual antlrcpp::Any visitEvent(SmallParser::EventContext *ctx) override {
        std::shared_ptr<node> condition = visitExpr(ctx->expr());
        std::shared_ptr<node> last = condition->getLast();
        if (!last) last = condition;
        Type t = (last->getType() == boolType) ? okType : errorType;
        std::shared_ptr<node> ifNode = std::make_shared<node>(node(t, If));
        ifNode->setNexts(std::vector<std::shared_ptr<node>>{nullptr, condition});
        last->setNext(ifNode);
        std::shared_ptr<node> res = std::make_shared<eventNode>(eventNode(t, condition));
        return res;
    }

    virtual antlrcpp::Any visitScope(SmallParser::ScopeContext *ctx) override {
        return nullptr; //not used
    }

    virtual antlrcpp::Any visitRead(SmallParser::ReadContext *ctx) override {
       /* auto symbol = symboltables.find(ctx->NAME()->getText());
        std::string name = ctx->NAME()->getText();
        Type type = errorType;
        Type typeToWrite = errorType;
        if (symbol != symboltables.end() && symbol->second->getType() == intType) {
            type = intType;
        } else if (symbol != symboltables.end() && symbol->second->getType() != errorType) {
            typeToWrite = symbol->second->getType();
        }
        if (type != intType && typeToWrite != errorType) {
            std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                      << "] Type mismatch of variable used to read to. Expected "
                      << info[intType] << " got " << info[typeToWrite] << "\n";
        }
        std::shared_ptr<variableNode> nameNode = std::make_shared<variableNode>(variableNode(type, name));
        readNode node = readNode((type == intType) ? okType : errorType, nameNode);
        node.setType(type);
        std::shared_ptr<expressionNode> res = std::make_shared<readNode>(node);
        return res;*/
    }

    virtual antlrcpp::Any visitWrite(SmallParser::WriteContext *ctx) override {
        auto pair = symboltables.find(ctx->NAME()->getText());
        Type t = okType;
        if (pair == symboltables.end()) {
            t = errorType;
        }
        std::shared_ptr<node> name = std::make_shared<node>(node(intType, Variable, ctx->NAME()->getText()));
        name->setNext(visitExpr(ctx->expr()));
        std::shared_ptr<node> last = name->getLast();
        if (!last) last = name;/*
        while(last->getNext()) {
            last = last->getNext();
        }*/
        t = (t != errorType && last->getType() == intType) ? okType : errorType;
        last->setNext(std::make_shared<node>(node(t, Write)));
        return name;
       /* std::shared_ptr<expressionNode> expr = visitExpr(ctx->expr());
        auto symbol = symboltables.find(ctx->NAME()->getText());
        Type t = intType;
        if (symbol == symboltables.end() || symbol->second->getType() != intType) {
            t = errorType;
        }
        if (t != intType && symbol != symboltables.end()) {
            std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                      << "] Type mismatch of variable used to write from. Expected "
                      << info[t] << " got " << info[symbol->second->getType()] << "\n";
        }
        std::shared_ptr<variableNode> var = std::make_shared<variableNode>(variableNode(t, ctx->NAME()->getText()));
        std::shared_ptr<statementNode> res = std::make_shared<writeNode>(writeNode(var, expr));
        return res;*/
    }

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
        if (ctx->literal()) {
            std::string val = ctx->literal()->getText();
            std::shared_ptr<node> l = std::make_shared<node>(node((val == "true" || val == "false") ? boolType : intType, Literal, val));
            return l;
        } else if (ctx->NAME()) {
            auto pair = symboltables.find(ctx->NAME()->getText());
            Type t = (pair != symboltables.end())
                    ? (pair->second->getType())
                    : errorType;
            if (t == errorType) {
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                          << "] Attempted to use variable that hasn't been assigned a value\n";
            }
            std::shared_ptr<node> res = std::make_shared<node>(node(t,Variable,ctx->NAME()->getText()));
            return res;
        }
        /*if (ctx->LPAREN()) {
            return visitExpr(ctx->expr(0));
        } else if (ctx->OP_ADD()) {
            return binary_expression(ctx, PLUS);
        } else if (ctx->OP_SUB()) {
            if (ctx->left) {
                return binary_expression(ctx, MINUS);
            } else {
                std::shared_ptr<expressionNode> node = visitExpr(ctx->expr(0));
                Type t = intType;
                if (node->getType() != intType) t = errorType;
                if (auto n = dynamic_            std::shared_ptr<node> first = visitExpr(ctx->expr());
            std::shared_ptr<node> last = first;
            while(last->getNext()) {
                last = last->getNext();
            }
            auto pair = symboltables.find(ctx->NAME()->getText());
            if (pair == symboltables.end()) {
                symboltables.insert({ctx->NAME()->getText(), std::make_shared<constraintNode>(constraintNode(last->getType()))});
                pair = symboltables.find(ctx->NAME()->getText());
            }
            if (pair->second->getType() == errorType) t = errorType;
            last->setNext(std::make_shared<node>(node(t,Assign, ctx->NAME()->getText())));
            return first;cast<literalNode*>(node.get())) {
                    if (t != errorType) {
                        return compute_new_literal(*n, *n, NEG, t);
                    }
                }
                std::shared_ptr<expressionNode> res = std::make_shared<unaryExpressionNode>(unaryExpressionNode(t, NEG, node));
                return res;
            }
        } else if (ctx->OP_MUL()) {
            return binary_expression(ctx, MULT);
        } else if (ctx->OP_DIV()) {
            return binary_expression(ctx, DIV);
        } else if (ctx->OP_MOD()) {
            return binary_expression(ctx, MOD);
        } else if (ctx->OP_LEQ()) {
            return binary_expression(ctx, LEQ);
        } else if (ctx->OP_LT()) {
            return binary_expression(ctx, LE);
        } else if (ctx->OP_GT()) {
            return binary_expression(ctx, GE);
        } else if (ctx->OP_GEQ()) {
            return binary_expression(ctx, GEQ);
        } else if (ctx->OP_EQ()) {
            return binary_expression(ctx, EQ);
        } else if (ctx->OP_NEQ()) {
            return binary_expression(ctx, NEQ);
        } else if (ctx->OP_AND()) {
            return binary_expression(ctx, AND);
        } else if (ctx->OP_OR()) {
            return binary_expression(ctx, OR);
        } else if (ctx->OP_NOT()) {
            std::shared_ptr<expressionNode> node = visitExpr(ctx->expr(0));
            Type t = boolType;
            if (node->getType() != boolType) t = errorType;
            if (node->getType() != errorType && node->getType() != boolType) {
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                      << "] Type mismatch in unary expression. Expected "
                      << info[boolType] << " got " << info[node->getType()] << "\n";
            }
            if (auto n = dynamic_cast<literalNode*>(node.get())) {
                if (t != errorType) {
                    return compute_new_literal(*n, *n, NOT, t);
                }
            }
            std::shared_ptr<expressionNode> res = std::make_shared<unaryExpressionNode>(unaryExpressionNode(t, NOT, node));
            return res;
        } else if (ctx->literal()) {
            std::shared_ptr<expressionNode> p = std::make_shared<literalNode>(literalNode(ctx->literal()->getText()));
            return p;
        } else if (ctx->arrayAccess()) {
            std::shared_ptr<expressionNode> exp = visitArrayAccess(ctx->arrayAccess());
            return exp;
        } else if (ctx->NAME()) {
            auto pair = symboltables.find(ctx->NAME()->getText());
            variableNode node = (pair != symboltables.end())
                                ? variableNode(pair->second->getType(), ctx->NAME()->getText())
                                : variableNode(errorType, ctx->NAME()->getText());
            if (node.getType() == errorType) {
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                          << "] Attempted to use variable that hasn't been assigned a value\n";
            }
            std::shared_ptr<expressionNode> res = std::make_shared<variableNode>(node);
            return res;
        } else if (ctx->read()) {
            return visitRead(ctx->read());
        }*/
    }

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *ctx) override {
        std::shared_ptr<node> first = visitExpr(ctx->expr());
        std::shared_ptr<node> last = first->getLast();
        if (!last) last = first;
        /*
        while (last->getNext()) {
            last = last->getNext();
        }*/
        auto it = symboltables.find(ctx->NAME()->getText());
        std::string name = ctx->NAME()->getText();
        auto val = it->second;
        Type t;
        if (last->getType() != intType || it == symboltables.end()) {
            t = errorType;
        } else if (it->second->getType() == arrayIntType) {
            t = intType;
        } else {
            t = boolType;
        }
        std::shared_ptr<node> n = std::make_shared<node>(node(t, ArrayAccess, ctx->NAME()->getText()));
        last->setNext(n);
        return first;
    }

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *ctx) override {
        literalNode* result = visitChildren(ctx);
        return result;
    }

    virtual antlrcpp::Any visitArrayLiteral(SmallParser::ArrayLiteralContext *ctx) override {
        std::vector<std::shared_ptr<node>> inter;
        Type t = okType;
        for (auto i : ctx->expr()) {
            inter.push_back(visitExpr(i));
        }
        for (unsigned long l = 0; l < inter.size()-1; l++) {
            if (inter[l]->getType() != inter[l+1]->getType()) {
                t = errorType;
            }
            std::shared_ptr<node> tmp = inter[l]->getLast();
            if(tmp) {
                inter[l] = tmp;
            }/*
            inter[l] = inter[l]->getLast();
            while (inter[l]->getNext()) {
                inter[l] = inter[l]->getNext();
            }*/
            inter[l]->setNext(inter[l+1]);
        }
        if (t == okType) {
            if (inter[0]->getType() == intType) {
                t = arrayIntType;
            } else {
                t = arrayBoolType;
            }
        }
        std::shared_ptr<node> n = std::make_shared<node>(node(t, ArrayLiteral,std::to_string(inter.size())));
        inter[inter.size()-1]->setNext(n);
        return inter[0];
    }

    static std::string btos (bool val) {
        return val ? "true" : "false";
    }
/*
    static std::shared_ptr<expressionNode> compute_new_literal (literalNode l, literalNode r, op expressionType, Type t) {
        std::string lVal = l.value;
        std::string rVal = r.value;
        Type nodesType;
        int lIntVal = 0;
        int rIntVal = 0;
        bool lBoolVal = false;
        bool rBoolVal = false;
        if (l.getType() == intType) {
            lIntVal = std::stoi(lVal);
            rIntVal = std::stoi(rVal);
            nodesType = intType;
        } else {
            lBoolVal = lVal == "true";
            rBoolVal = rVal == "true";
            nodesType = boolType;
        }
        switch (expressionType) {
            case PLUS:
                return std::make_shared<literalNode>(literalNode(t, std::to_string(lIntVal + rIntVal)));
            case MINUS:
                return std::make_shared<literalNode>(literalNode(t, std::to_string(lIntVal - rIntVal)));
            case MULT:
                return std::make_shared<literalNode>(literalNode(t, std::to_string(lIntVal * rIntVal)));
            case DIV:
                return std::make_shared<literalNode>(literalNode(t, std::to_string(lIntVal / rIntVal)));
            case MOD:
                return std::make_shared<literalNode>(literalNode(t, std::to_string(lIntVal % rIntVal)));
            case AND:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal && rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal && rBoolVal)));
                }
            case OR:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal || rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal || rBoolVal)));
                }
            case LE:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal < rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal < rBoolVal)));
                }
            case LEQ:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal <= rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal <= rBoolVal)));
                }
            case GE:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal > rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal > rBoolVal)));
                }
            case GEQ:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal >= rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal >= rBoolVal)));
                }
            case EQ:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal == rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal == rBoolVal)));
                }
            case NEQ:
                if (nodesType == intType) {
                    return std::make_shared<literalNode>(literalNode(t, btos(lIntVal != rIntVal)));
                } else {
                    return std::make_shared<literalNode>(literalNode(t, btos(lBoolVal != rBoolVal)));
                }
            case NOT:
                return std::make_shared<literalNode>(literalNode(t, btos(!lBoolVal)));
            case NEG:
                return std::make_shared<literalNode>(literalNode(t, std::to_string(-lIntVal)));
            default:
                std::cout << "this shouldn't happen\n";
        }
    }

    std::shared_ptr<expressionNode> binary_expression (SmallParser::ExprContext *ctx, op expressionType) {
        std::shared_ptr<expressionNode> l = (visitExpr(ctx->left));
        std::shared_ptr<expressionNode> r = (visitExpr(ctx->right));
        Type t;

        switch (expressionType) {
            case PLUS:
            case MINUS:
            case MULT:
            case DIV:
            case MOD:
                if (l->getType() == r->getType() && l->getType() == intType) {
                    t = l->getType();
                } else {
                    std::cout << "[" << ctx->start->getLine() << ":" << ctx->start->getCharPositionInLine()
                    << "] Type mismatch in binary expression. Expected " << info[l->getType()] << " got " << info[r->getType()] << "\n";
                    t = errorType;
                }
                break;
            case AND:
            case OR:
                if (l->getType() == r->getType() && l->getType() == boolType) {
                    t = l->getType();
                } else {
                    std::cout << "[" << ctx->start->getLine() << ":" << ctx->start->getCharPositionInLine()
                              << "] Type mismatch in binary expression. Expected " << info[boolType] << " got "
                              << info[r->getType() != boolType ? r->getType() : l->getType()] << "\n";
                    t = errorType;
                }
                break;
            case LE:
            case LEQ:
            case GE:
            case GEQ:
                if (l->getType() == r->getType() && l->getType() == intType) {
                    t = boolType;
                } else {
                    std::cout << "[" << ctx->start->getLine() << ":" << ctx->start->getCharPositionInLine()
                              << "] Type mismatch in binary expression. Expected " << info[intType] << " got "
                              << info[r->getType() != intType ? r->getType() : l->getType()] << "\n";
                    t = errorType;
                }
                break;
            case EQ:
            case NEQ:
                if (l->getType() == r->getType() && (l->getType() == intType || l->getType() == boolType)) {
                    t = boolType;
                } else {
                    std::cout << "[" << ctx->start->getLine() << ":" << ctx->start->getCharPositionInLine()
                              << "] Type mismatch in binary expression. Expected " << info[l->getType()] << " got " << info[r->getType()] << "\n";
                    t = errorType;
                }
                break;
            default:
                t = errorType;
                break;
        }
        if (t != errorType) {
            if (auto lh = dynamic_cast<literalNode *>(l.get())) {
                if (auto rh = dynamic_cast<literalNode *>(r.get())) {
                    return compute_new_literal(*lh, *rh, expressionType, t);
                }
            }
        }
        std::shared_ptr<expressionNode> p = std::make_shared<binaryExpressionNode>(binaryExpressionNode(t, expressionType, l,r));
        return p;
    }
*/
    /*
    static antlrcpp::Any deepCopy(const node *tree) {
        switch(tree->getNodeType()) {
            case Assign:
                if(auto a = dynamic_cast<const assignNode*>(tree)) {
                    std::shared_ptr<expressionNode> e = deepCopy(a->getExpr());
                    std::shared_ptr<statementNode> res = std::make_shared<assignNode>(assignNode(a->getType(),a->getName(),e));
                    return res;
                }
                break;
            case AssignArrField:
                if(auto a = dynamic_cast<const arrayFieldAssignNode*>(tree)) {
                    std::shared_ptr<expressionNode> field = deepCopy(a->getField());
                    std::shared_ptr<expressionNode> e = deepCopy(a->getExpr());
                    std::shared_ptr<statementNode> res = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(a->getType(),field,e));
                    return res;
                }
                break;
            case Concurrent:
                if (auto a = dynamic_cast<const concurrentNode*>(tree)) {
                    std::vector<std::shared_ptr<statementNode>> nodes;
                    for (auto &n : a->getThreads()) {
                        std::shared_ptr<statementNode> inter = deepCopy(n.get());
                        nodes.push_back(inter);
                    }
                    std::shared_ptr<statementNode> res = std::make_shared<concurrentNode>(concurrentNode(a->getType(), std::move(nodes)));
                    return res;
                }
                break;
            case Sequential:
                if (auto a = dynamic_cast<const sequentialNode*>(tree)) {
                    std::shared_ptr<statementNode> body = deepCopy(a->getBody());
                    std::shared_ptr<statementNode> next = deepCopy(a->getNext());
                    std::shared_ptr<statementNode> res = std::make_shared<sequentialNode>(sequentialNode(a->getType(), body, next));
                    return res;
                }
                break;
            case While:
                if (auto a = dynamic_cast<const whileNode*>(tree)) {
                    std::shared_ptr<statementNode> body = deepCopy(a->getBody());
                    std::shared_ptr<expressionNode> cond = deepCopy(a->getCondition());
                    std::shared_ptr<statementNode> res = std::make_shared<whileNode>(whileNode(a->getType(),cond,body));
                    return res;
                }
                break;
            case If:
                if (auto a = dynamic_cast<const ifElseNode*>(tree)) {
                    std::shared_ptr<statementNode> tb = deepCopy(a->getTrueBranch());
                    std::shared_ptr<statementNode> fb = deepCopy(a->getFalseBranch());
                    std::shared_ptr<expressionNode> cond = deepCopy(a->getCondition());
                    std::shared_ptr<statementNode> res = std::make_shared<ifElseNode>(ifElseNode(a->getType(),cond,tb,fb));
                    return res;
                }
                break;
            case Write:
                if (auto a = dynamic_cast<const writeNode*>(tree)) {
                    std::shared_ptr<expressionNode> v = deepCopy(a->getVar());
                    std::shared_ptr<expressionNode> e = deepCopy(a->getExpr());
                    std::shared_ptr<statementNode> res = std::make_shared<writeNode>(writeNode(a->getType(),v,e));
                    return res;
                }
                break;
            case Read:
                if (auto a = dynamic_cast<const readNode*>(tree)) {
                    std::shared_ptr<expressionNode> v = deepCopy(a->getVar());
                    std::shared_ptr<expressionNode> res = std::make_shared<readNode>(readNode(a->getType(),v));
                    return res;
                }
                break;
            case Literal:
                if(auto a = dynamic_cast<const literalNode*>(tree)) {
                    std::shared_ptr<expressionNode> res = std::make_shared<literalNode>(literalNode(a->getType(), a->value));
                    return res;
                }
                break;
            case ArrayAccess:
                if(auto a = dynamic_cast<const arrayAccessNode*>(tree)) {
                    std::shared_ptr<expressionNode> acc = deepCopy(a->getAccessor());
                    std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(a->getType(), acc, a->getName()));
                    return res;
                }
                break;
            case ArrayLiteral:
                if(auto a = dynamic_cast<const arrayLiteralNode*>(tree)) {
                    std::vector<std::shared_ptr<expressionNode>> nodes;
                    for (auto &i : a->getArrLit()) {
                        std::shared_ptr<expressionNode> inter = deepCopy(i.get());
                        nodes.push_back(inter);
                    }
                    std::shared_ptr<expressionNode> res = std::make_shared<arrayLiteralNode>(arrayLiteralNode(a->getType(),nodes));
                    return res;
                }
                break;
            case Event:
                if(auto a = dynamic_cast<const eventNode*>(tree)) {
                    std::shared_ptr<expressionNode> cond = deepCopy(a->getCondition());
                    std::shared_ptr<statementNode> res = std::make_shared<eventNode>(eventNode(a->getType(), cond));
                    return res;
                }
                break;
            case Variable:
                if(auto a = dynamic_cast<const variableNode*>(tree)) {
                    std::shared_ptr<expressionNode> res = std::make_shared<variableNode>(variableNode(a->getType(),a->name));
                    return res;
                }
                break;
            case BinaryExpression:
                if(auto a = dynamic_cast<const binaryExpressionNode*>(tree)) {
                    std::shared_ptr<expressionNode> e1 = deepCopy(a->getLeft());
                    std::shared_ptr<expressionNode> e2 = deepCopy(a->getRight());
                    std::shared_ptr<expressionNode> res = std::make_shared<binaryExpressionNode>(binaryExpressionNode(a->getType(), a->getOperator(), e1, e2));
                    return res;
                }
                break;
            case UnaryExpression:
                if(auto a = dynamic_cast<const unaryExpressionNode*>(tree)) {
                    std::shared_ptr<expressionNode> e = deepCopy(a->getExpression());
                    std::shared_ptr<expressionNode> res = std::make_shared<unaryExpressionNode>(unaryExpressionNode(a->getType(), a->getOperator(),e));
                    return res;
                }
                break;
        }
    }*/
/*
    std::shared_ptr<statementNode> sequentialAssignForArray(std::string name, int accessor, std::vector<std::shared_ptr<expressionNode>> n) {
        Type t = n[0]->getType();
        if(n.size() == 1) {
            std::shared_ptr<expressionNode> access = std::make_shared<literalNode>(t, std::to_string(accessor));
            std::shared_ptr<expressionNode> arrAcc = std::make_shared<arrayAccessNode>(arrayAccessNode(t,access,name));
            std::shared_ptr<statementNode> arrFieldAss = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(okType,arrAcc,n[0]));
            return arrFieldAss;
        } else {
            std::shared_ptr<expressionNode> access = std::make_shared<literalNode>(t,std::to_string(accessor));
            std::shared_ptr<expressionNode> arrAcc = std::make_shared<arrayAccessNode>(arrayAccessNode(t,access,name));
            std::shared_ptr<statementNode> arrFieldAss = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(okType,arrAcc,n[0]));
            std::vector<std::shared_ptr<expressionNode>> ne;
            for (auto i = 1; i < n.size(); i++) {
                ne.push_back(n[i]);
            }
            return std::make_shared<sequentialNode>(sequentialNode(okType,arrFieldAss,sequentialAssignForArray(name,accessor+1,ne)));
        }
    }*/
};

#endif //DST_H