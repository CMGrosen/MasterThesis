
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
            std::shared_ptr<node> last = first;
            while(last->getNext()) {
                last = last->getNext();
            }
            last->setNext(visitStmts(ctx->stmts()));
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
        if (ctx->arrayLiteral()) {
            std::shared_ptr<node> last = visitArrayLiteral(ctx->arrayLiteral());
            std::shared_ptr<node> first = last;
            while(last->getNext()) {
                last = last->getNext();
            }
            if (last->getType() != okType) {
                t = errorType;
            } else {
                t = last->getType();
            }
            std::shared_ptr<node> n = std::make_shared<node>(node(t, Assign, ctx->NAME()->getText()));
            int length = std::stoi(last->getValue());
            while (length-->0)
                symboltables.insert({(ctx->NAME()->getText() + "[" + std::to_string(length) + "]"), std::make_shared<constraintNode>(constraintNode(first->getType()))});
            last->setNext(n);
            return first;
        }
        /*Type t;
        if (ctx->arrayAccess()) {
            std::shared_ptr<node> node = visitArrayLiteral(ctx->arrayLiteral());
            std::shared_ptr<node> arrAcc = visitArrayAccess(ctx->arrayAccess());
            if (node->getType() == arrayIntType || node->getType() == arrayBoolType) {
                t = errorType;
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                << "] Cannot assign an array to a field of an array\n";
            } else if (arrAcc->getType() != intType) {
                t = errorType;
                if (arrAcc->getType() != errorType) {
                    std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                    << "] Type mismatch in assignment. Expected field accessor to be of type " << info[intType] << " got " << info[arrAcc->getType()] << "\n";
                }
            } else if (node->getType() == errorType) {
                t = errorType;
            } else {
                t = okType;
            }

            std::shared_ptr<statementNode> a = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(t, arrAcc, node));
            return a;
        } else if (ctx->NAME() && ctx->arrayLiteral()) {
            std::shared_ptr<arrayLiteralNode> node = visitArrayLiteral(ctx->arrayLiteral());
            auto arrLit = node.get();
            std::string name = ctx->NAME()->getText();
            auto pair = symboltables.insert({name, std::make_shared<constraintNode>(constraintNode(node->getType()))});
            auto arr = arrLit->getArrLit();
            t = arr[0]->getType();
            int count = arr.size();
            for (int i = 0; i < count; ++i) {
                std::shared_ptr<expressionNode> temp = std::make_shared<constraintNode>(constraintNode(t));
                auto p = symboltables.insert({name + "[" + std::to_string(i) + "]", temp});
                if(!p.second && p.first->second->getType() != t) {
                    if (t != errorType) {
                        std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                                  << "] Attempt at changing type signature of array from "
                                  << info[t] << " to " << info[p.first->second->getType()] << "\n";
                    }
                    pair.first->second->setType(errorType);
                }
            }
            if(!pair.second) {
                auto size = arr.size();
                if (symboltables.find(name + "[" + std::to_string(size-1) + "]") == symboltables.end() ||
                    (symboltables.find(name + "[" + std::to_string(size) + "]") != symboltables.end())) {
                    //If this variable already exists and is assigned to a new arrayLiteral that are not the same size as the one previously assigned:
                    std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                              << "] Attempt to reassign an array to another literal that is not the same size\n";
                    pair.first->second->setType(errorType);
                }
            }
            return sequentialAssignForArray(name,0, arr);
        } else {
            std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
            std::string name = ctx->NAME()->getText();
            auto pair = symboltables.insert({name, std::make_shared<constraintNode>(constraintNode(node->getType()))});


            if(!pair.second && pair.first->second->getType() != node->getType())
                pair.first->second->setType(errorType);

            assignNode assNode = assignNode(pair.first->second->getType() == errorType ? errorType : okType, name, node);
            std::shared_ptr<statementNode> a = std::make_shared<assignNode>(assNode);
            return a;
        }*/
    }

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *ctx) override {
        /*std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        std::shared_ptr<statementNode> body = visitStmts(ctx->scope()->stmts());
        Type t = okType;
        if (condition->getType() != boolType || body->getType() != okType) t = errorType;
        std::shared_ptr<statementNode> res = std::make_shared<whileNode>(whileNode(t, condition, body));
        return res;*/
    }

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *ctx) override {
       /* std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        std::shared_ptr<statementNode> trueBranch = visitStmts(ctx->scope(0)->stmts());
        std::shared_ptr<statementNode> falseBranch = visitStmts(ctx->scope(1)->stmts());
        Type t = okType;
        if (condition->getType() != boolType || trueBranch->getType() != okType || falseBranch->getType() != okType) t = errorType;
        std::shared_ptr<statementNode> res = std::make_shared<ifElseNode>(ifElseNode(t, condition, trueBranch, falseBranch));
        return res;*/
    }

    virtual antlrcpp::Any visitThread(SmallParser::ThreadContext *ctx) override {
        /*std::vector<std::shared_ptr<statementNode>> statements;
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
        std::shared_ptr<statementNode> res = std::make_shared<concurrentNode>(concurrentNode(t, statements));
        return res;*/
    }

    virtual antlrcpp::Any visitEvent(SmallParser::EventContext *ctx) override {
        /*std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        Type t = (condition->getType() == boolType) ? okType : errorType;
        std::shared_ptr<statementNode> res = std::make_shared<eventNode>(eventNode(t, condition));
        return res;*/
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
            std::shared_ptr<node> l = std::make_shared<literalNode>(literalNode((val == "true" || val == "false") ? boolType : intType, val));
            return l;
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
                if (auto n = dynamic_cast<literalNode*>(node.get())) {
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
        /*std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
        auto it = symboltables.find(ctx->NAME()->getText());
        Type t;
        if (node->getType() != intType || it == symboltables.end()) {
            t = errorType;
        } else if (it->second->getType() == arrayIntType) {
            t = intType;
        } else {
            t = boolType;
        }
        std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(t, node, ctx->NAME()->getText()));
        return res;*/
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
            while (inter[l]->getNext()) {
                inter[l] = inter[l]->getNext();
            }
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