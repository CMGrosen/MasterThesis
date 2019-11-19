#include "antlr4-runtime.h"
#include "SmallVisitor.h"
#include <nodes/nodes.hpp>
#include "symengine/Constraint.hpp"


/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */


class  DST : public SmallVisitor {
public:
    std::unordered_map<std::string, constraint> symboltables;

    std::pair<const std::shared_ptr<statementNode>, const std::unordered_map<std::string, constraint>> getTree(SmallParser::FileContext *ctx) {
        std::shared_ptr<statementNode> a = visitStmts(ctx->stmts());

        std::shared_ptr<statementNode> aNew = deepCopy(a.get());
        return std::pair<std::shared_ptr<statementNode>, std::unordered_map<std::string, constraint>>(std::move(aNew), symboltables);
    }

    virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
        //antlrcpp::Any result = visitChildren(ctx);
        std::shared_ptr<statementNode> a = visitStmts(ctx->stmts());

        //std::vector<std::shared_ptr<statementNode>> l = a->debug_getAllNodes();
        //std::cout << "\n\n\nwriting out what we have:\n";
  //      WriteType(a);
        //std::cout << dynamic_cast<literalNode*>(dynamic_cast<additionNode*>(a[0]->value)->getLeft())->value << std::endl;
        //a->setNextStatement(firstStatement);

        return a;
    }

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *ctx) override {
        if(ctx->stmts()) {
            std::shared_ptr<statementNode> body = visitStmt(ctx->stmt());
            std::shared_ptr<statementNode> next = visitStmts(ctx->stmts());
            std::shared_ptr<statementNode> result = std::make_shared<sequentialNode>(sequentialNode(body, next));
            return result;
        } else {
            std::shared_ptr<statementNode> result = visitStmt(ctx->stmt());
            return result;
        }
    }

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *ctx) override {
        if (ctx->assign()) {
            std::shared_ptr<statementNode> stmt =  visitAssign(ctx->assign());
            return stmt;
        } else if (ctx->write()) {
            std::shared_ptr<statementNode> stmt = visitWrite(ctx->write());
            return stmt;
        } else if (ctx->iter()) {
            std::shared_ptr<statementNode> stmt = visitIter(ctx->iter());
            return stmt;
        } else if (ctx->ifs()) {
            std::shared_ptr<statementNode> stmt = visitIfs(ctx->ifs());
            return stmt;
        } else if (ctx->thread()) {
            std::shared_ptr<statementNode> stmt = visitThread(ctx->thread());
            return stmt;
        } else { //only event is remaining
            std::shared_ptr<statementNode> stmt = visitEvent(ctx->event());
            return stmt;
        }
    }

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *ctx) override {
        Type t;
        std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
        if (ctx->arrayAccess()) {
            std::shared_ptr<expressionNode> arrAcc = visitArrayAccess(ctx->arrayAccess());
            if (node->getType() == arrayIntType || node->getType() == arrayBoolType || arrAcc->getType() != intType) {
                t = errorType;
            } else {
                t = okType;
            }

            std::shared_ptr<statementNode> a = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(t, arrAcc, node));
            return a;
            std::cout << "debug";
        } else {
            std::string name = ctx->NAME()->getText();
            auto pair = symboltables.insert({name, constraint(node->getType())});
            if (node->getType() == arrayIntType || node->getType() == arrayBoolType){
                if(auto arrLit = dynamic_cast<arrayLiteralNode*>(node.get())) {
                    auto arr = arrLit->getArrLit();
                    t = arr[0]->getType();
                    int count = arr.size();
                    for (int i = 0; i < count; ++i) {
                        constraint temp = constraint(t);
                        auto p = symboltables.insert({name + "[" + std::to_string(i) + "]", temp});
                        if(!p.second && p.first->second.type != t) {
                            pair.first->second.type = errorType;
                        }
                    }
                    if(!pair.second) {
                        auto size = arr.size();
                        if (symboltables.find(name + "[" + std::to_string(size-1) + "]") == symboltables.end() ||
                            (symboltables.find(name + "[" + std::to_string(size) + "]") != symboltables.end())) {
                            //If this variable already exists and is assigned to a new arrayLiteral that are not the same size as the one previously assigned:
                            pair.first->second.type = errorType;
                        }
                    }
                }
            }

            if(!pair.second && pair.first->second.type != node->getType())
                pair.first->second.type = errorType;
//        std::shared_ptr<assignNode> res = std::make_shared<assignNode>(assignNode(name, node));

            assignNode assNode = assignNode(pair.first->second.type == errorType ? errorType : okType, name, node);
            std::shared_ptr<statementNode> a = std::make_shared<assignNode>(assNode);
            return a;
        }
    }

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *ctx) override {
        std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        std::shared_ptr<statementNode> body = visitStmts(ctx->scope()->stmts());
        Type t = okType;
        if (condition->getType() != boolType || body->getType() != okType) t = errorType;
        std::shared_ptr<statementNode> res = std::make_shared<whileNode>(whileNode(t, condition, body));
        return res;
    }

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *ctx) override {
        std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        std::shared_ptr<statementNode> trueBranch = visitStmts(ctx->scope(0)->stmts());
        std::shared_ptr<statementNode> falseBranch = visitStmts(ctx->scope(1)->stmts());
        Type t = okType;
        if (condition->getType() != boolType || trueBranch->getType() != okType || falseBranch->getType() != okType) t = errorType;
        std::shared_ptr<statementNode> res = std::make_shared<ifElseNode>(ifElseNode(t, condition, trueBranch, falseBranch));
        return res;
    }

    virtual antlrcpp::Any visitThread(SmallParser::ThreadContext *ctx) override {
        std::vector<std::shared_ptr<statementNode>> statements;
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
        return res;
    }

    virtual antlrcpp::Any visitEvent(SmallParser::EventContext *ctx) override {
        std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
        Type t = (condition->getType() == boolType) ? okType : errorType;
        std::shared_ptr<statementNode> res = std::make_shared<eventNode>(eventNode(t, condition));
        return res;
    }

    virtual antlrcpp::Any visitScope(SmallParser::ScopeContext *ctx) override {
        return nullptr; //not used
    }

    virtual antlrcpp::Any visitRead(SmallParser::ReadContext *ctx) override {
        auto symbol = symboltables.find(ctx->NAME()->getText());
        std::string name = ctx->NAME()->getText();
        Type type = errorType;
        if (symbol != symboltables.end() && symbol->second.type == intType) {
            type = intType;
        }
        std::shared_ptr<variableNode> nameNode = std::make_shared<variableNode>(variableNode(type, name));
        readNode node = readNode((type == intType) ? okType : errorType, nameNode);
        node.setType(type);
        std::shared_ptr<expressionNode> res = std::make_shared<readNode>(node);
        return res;
    }

    virtual antlrcpp::Any visitWrite(SmallParser::WriteContext *ctx) override {
        std::shared_ptr<expressionNode> expr = visitExpr(ctx->expr());
        auto symbol = symboltables.find(ctx->NAME()->getText());
        Type t = intType;
        if (symbol == symboltables.end() || symbol->second.type != intType) {
            t = errorType;
        }
        std::shared_ptr<variableNode> var = std::make_shared<variableNode>(variableNode(t, ctx->NAME()->getText()));
        std::shared_ptr<statementNode> res = std::make_shared<writeNode>(writeNode(var, expr));
        return res;
    }

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
        if (ctx->LPAREN()) {
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
            std::shared_ptr<expressionNode> res = std::make_shared<unaryExpressionNode>(unaryExpressionNode(t, NOT, node));
            return res;
        } else if (ctx->literal()) {
            std::shared_ptr<expressionNode> p = std::make_shared<literalNode>(literalNode(ctx->literal()->getText()));
            return p;
        } else if (ctx->arrayLiteral()) {
            std::shared_ptr<expressionNode> exp = visitArrayLiteral(ctx->arrayLiteral());
            return exp;
        } else if (ctx->arrayAccess()) {
            std::shared_ptr<expressionNode> exp = visitArrayAccess(ctx->arrayAccess());
            return exp;
        } else if (ctx->NAME()) {
            auto pair = symboltables.find(ctx->NAME()->getText());
            variableNode node = (pair != symboltables.end())
                                ? variableNode(pair->second.type, ctx->NAME()->getText())
                                : variableNode(errorType, ctx->NAME()->getText());
            std::shared_ptr<expressionNode> res = std::make_shared<variableNode>(node);
            return res;
        } else if (ctx->read()) {
            return visitRead(ctx->read());
        }
    }

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *ctx) override {
        std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
        auto it = symboltables.find(ctx->NAME()->getText());
        Type t;
        if (node->getType() != intType || it == symboltables.end()) {
            t = errorType;
        } else if (it->second.type == arrayIntType) {
            t = intType;
        } else {
            t = boolType;
        }
        std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(t, node));
        return res;
    }

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *ctx) override {
        literalNode* result = visitChildren(ctx);
        return result;
    }

    virtual antlrcpp::Any visitArrayLiteral(SmallParser::ArrayLiteralContext *ctx) override {
        std::vector<std::shared_ptr<expressionNode>> inter;
        auto a = ctx->expr();
        for (auto i : ctx->expr()) {
            inter.push_back(visitExpr(i));
        }
        std::shared_ptr<expressionNode> res = std::make_shared<arrayLiteralNode>(arrayLiteralNode(inter));
        return res;
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
        std::shared_ptr<expressionNode> p = std::make_shared<binaryExpressionNode>(binaryExpressionNode(t, expressionType, l,r));
        return p;
    }

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
                    std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(a->getType(), acc));
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
    }
};