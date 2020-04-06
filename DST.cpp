//
// Created by hu on 18/10/2019.
//

#include "DST.hpp"

int DST::getNumErrors() const {return foundErrors;}
void DST::updateErrorCount() {
    foundErrors++;
}

std::pair<const std::shared_ptr<statementNode>, const std::unordered_map<std::string, std::shared_ptr<expressionNode>>> DST::getTree(SmallParser::FileContext *ctx) {
    std::shared_ptr<statementNode> a = visitStmts(ctx->stmts());
    //std::shared_ptr<statementNode> aNew = deepCopy(a.get());
    return std::pair<std::shared_ptr<statementNode>, std::unordered_map<std::string, std::shared_ptr<expressionNode>>>(std::move(a), symboltables);
}

antlrcpp::Any DST::visitFile(SmallParser::FileContext *ctx) {
    std::shared_ptr<statementNode> a = visitStmts(ctx->stmts());
    return a;
}

antlrcpp::Any DST::visitStmts(SmallParser::StmtsContext *ctx) {
    if(ctx->stmts()) {
        std::shared_ptr<statementNode> result;
        std::shared_ptr<statementNode> body = visitStmt(ctx->stmt());
        std::shared_ptr<statementNode> next = visitStmts(ctx->stmts());
        // loop unrolling while stmt to nested ifElse chain
        if(ctx->stmt()->iter()){
            std::shared_ptr<statementNode> _body = dynamic_cast<sequentialNode*>(body.get())->getBody();
            std::shared_ptr<statementNode> _next = std::make_shared<sequentialNode>(sequentialNode(dynamic_cast<sequentialNode*>(body.get())->getNext(), next));
            result = std::make_shared<sequentialNode>(_body, _next );
        } else {
            result = std::make_shared<sequentialNode>(sequentialNode(body, next));
        }

        return result;
    } else {
        std::shared_ptr<statementNode> result = visitStmt(ctx->stmt());
        return result;
    }
}

antlrcpp::Any DST::visitStmt(SmallParser::StmtContext *ctx) {
    std::shared_ptr<statementNode> stmt;
    if (ctx->assign()) {
        stmt = visitAssign(ctx->assign());
    } else if (ctx->write()) {
        stmt = visitWrite(ctx->write());
    } else if (ctx->iter()) {
        stmt = visitIter(ctx->iter());
    } else if (ctx->ifs()) {
        stmt = visitIfs(ctx->ifs());
    } else if (ctx->thread()) {
        stmt = visitThread(ctx->thread());
    } else if (ctx->event()) {
        stmt = visitEvent(ctx->event());
    } else { //only skip is remaining
        stmt = visitSkipStmt(ctx->skipStmt());
    }
    return stmt;
}

antlrcpp::Any DST::visitAssign(SmallParser::AssignContext *ctx) {
    Type t;
    if (ctx->arrayAccess()) {
        std::string name = ctx->arrayAccess()->NAME()->getText();
        std::shared_ptr<expressionNode> value = visitExpr(ctx->expr());
        std::shared_ptr<expressionNode> field = visitExpr(ctx->arrayAccess()->expr());
        if (value->getType() == arrayIntType || value->getType() == arrayBoolType) {
            t = errorType;
            std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
            << "] Cannot assign an array to a field of an array\n";
        } else if (field->getType() == errorType) {
            t = errorType;
            if (field->getType() != errorType) {
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                << "] Type mismatch in assignment. Expected field accessor to be of type " << info[intType] << "\n";
            }
        } else if (value->getType() == errorType) {
            t = errorType;
        } else {
            t = okType;
        }

        std::shared_ptr<statementNode> res = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(t, name, field, value, ctx->start->getLine()));
        return res;
    } else if (ctx->NAME() && ctx->arrayLiteral()) {
        std::shared_ptr<arrayLiteralNode> node = visitArrayLiteral(ctx->arrayLiteral());
        auto arrLit = node.get();
        std::string name = ctx->NAME()->getText();
        auto arr = arrLit->getArrLit();
        //variable name + length
        auto pair = symboltables.insert({name, std::make_shared<literalNode>(literalNode(node->getType(), std::to_string(arr.size())))});

        if(!pair.second) { //variable already exists
            auto length = dynamic_cast<literalNode*>(pair.first->second.get())->value;
            if (arr.size() != std::stoul(length)) {
                std::cout << "[" << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                          << "] Attempt to reassign an array to another literal that is not the same size\n";
                pair.first->second->setType(errorType);
            }
        }
        std::shared_ptr<expressionNode> expr = node;
        std::shared_ptr<statementNode> res = std::make_shared<assignNode>(assignNode(pair.first->second->getType() == errorType ? errorType : okType, name, expr, ctx->start->getLine()));
        return res;
        //return sequentialAssignForArray(name,0, arr);
    } else {
        std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
        std::string name = ctx->NAME()->getText();
        auto pair = symboltables.insert({name, std::make_shared<literalNode>(literalNode(node->getType(), node->getType() == intType ? "0" : "false"))});

        //variable changes type
        if(!pair.second && pair.first->second->getType() != node->getType())
            pair.first->second->setType(errorType);
//        std::shared_ptr<assignNode> res = std::make_shared<assignNode>(assignNode(name, node));

        assignNode assNode = assignNode(pair.first->second->getType() == errorType ? errorType : okType, name, node, ctx->start->getLine());
        std::shared_ptr<statementNode> a = std::make_shared<assignNode>(assNode);
        return a;
    }
}
// returns a sequential node with a early exit setup as the stmt and a unrolled loop as the stmts
antlrcpp::Any DST::visitIter(SmallParser::IterContext *ctx) {
    std::shared_ptr<expressionNode> _condition = visitExpr(ctx->expr());
    std::shared_ptr<statementNode> _body = visitStmts(ctx->scope()->stmts());
    std::shared_ptr<statementNode> falseBranch = std::make_shared<skipNode>(skipNode(ctx->start->getLine()));
    Type t = okType;
    if (_condition->getType() != boolType || _body->getType() != okType) t = errorType;
    // create early exit setup and check to help determine if loop is exited naturally
    std::string early_exit = std::to_string(early_exits) + "_early_exit";
    std::shared_ptr<expressionNode> assExpr  = std::make_shared<literalNode>(literalNode(boolType,"false"));

    std::shared_ptr<assignNode> early_exit_setup = std::make_shared<assignNode>(assignNode(okType, early_exit, assExpr, ctx->start->getLine()));

    assExpr = std::make_shared<literalNode>(literalNode(boolType,"true"));
    std::shared_ptr<assignNode> early_exit_check = std::make_shared<assignNode>(assignNode(okType, early_exit, assExpr, ctx->start->getLine()));
    // add early_exit variable to symbol table
    symboltables.insert({early_exit, assExpr});
    // increment early_exits to ensure unique names for all early_exit variables.
    early_exits++;
    // create early exit check
    std::shared_ptr<statementNode> temp = std::make_shared<ifElseNode>(ifElseNode(t, _condition, early_exit_check, falseBranch, ctx->start->getLine(), true));

    // create MAXITER number of  iterations, starts with the last iteration and ends with the first.
    for(int i = 0; i < MAXITER; i++){
        //create new nodes and pointers to ensure uniqueness
        std::shared_ptr<expressionNode> condition = _condition->copy_expression();
        std::shared_ptr<statementNode> body = _body->copy_statement();
        falseBranch = std::make_shared<skipNode>(skipNode(ctx->start->getLine()));
        // nest the next iteration into this iteration
        std::shared_ptr<statementNode> trueBranch = std::make_shared<sequentialNode>(sequentialNode(body, temp));
        // current iteration
        temp = std::make_shared<ifElseNode>(ifElseNode(t, condition, trueBranch, falseBranch, ctx->start->getLine(), true));
    }
    //std::shared_ptr<statementNode> res = std::make_shared<whileNode>(whileNode(t, condition, body));
    // sequential node with the early_exit setup before the unrolled loop.
    std::shared_ptr<statementNode> res = std::make_shared<sequentialNode>(sequentialNode(early_exit_setup, temp));
    return res;
}

antlrcpp::Any DST::visitIfs(SmallParser::IfsContext *ctx) {
    std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
    std::shared_ptr<statementNode> trueBranch = visitStmts(ctx->scope(0)->stmts());
    std::shared_ptr<statementNode> falseBranch = visitStmts(ctx->scope(1)->stmts());
    Type t = okType;
    std::string txt = ctx->IF()->getText();
    if (condition->getType() != boolType || trueBranch->getType() != okType || falseBranch->getType() != okType) t = errorType;
    std::shared_ptr<statementNode> res = std::make_shared<ifElseNode>(ifElseNode(t, condition, trueBranch, falseBranch, ctx->start->getLine()));
    return res;
}

antlrcpp::Any DST::visitThread(SmallParser::ThreadContext *ctx) {
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
    std::shared_ptr<statementNode> res = std::make_shared<concurrentNode>(concurrentNode(t, statements, ctx->start->getLine()));
    return res;
}

antlrcpp::Any DST::visitEvent(SmallParser::EventContext *ctx) {
    std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
    Type t = (condition->getType() == boolType) ? okType : errorType;
    std::shared_ptr<statementNode> res = std::make_shared<eventNode>(eventNode(t, condition, ctx->start->getLine()));
    return res;
}


antlrcpp::Any DST::visitSkipStmt(SmallParser::SkipStmtContext *ctx) {
    std::shared_ptr<statementNode> res = std::make_shared<skipNode>(skipNode(ctx->start->getLine()));
    return res;
}

antlrcpp::Any DST::visitAssertStmt(SmallParser::AssertStmtContext *ctx) {
    std::shared_ptr<expressionNode> condition = visitExpr(ctx->expr());
    Type t = (condition->getType() == boolType) ? okType : errorType;
    std::shared_ptr<statementNode> res = std::make_shared<assertNode>(assertNode(t, condition, ctx->start->getLine()));
    return res;
}

antlrcpp::Any DST::visitScope(SmallParser::ScopeContext *ctx) {
    return nullptr; //not used
}

antlrcpp::Any DST::visitRead(SmallParser::ReadContext *ctx) {
    readNode node = readNode(std::stoi(ctx->INT_LITERAL()->getText()));
    std::shared_ptr<expressionNode> res = std::make_shared<readNode>(node);
    return res;
}

antlrcpp::Any DST::visitWrite(SmallParser::WriteContext *ctx) {
    std::shared_ptr<expressionNode> expr = visitExpr(ctx->expr());
    std::shared_ptr<statementNode> res = std::make_shared<writeNode>(writeNode(std::stoi(ctx->INT_LITERAL()->getText()), expr));
    return res;
}

antlrcpp::Any DST::visitExpr(SmallParser::ExprContext *ctx) {
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
            std::shared_ptr<expressionNode> una = std::make_shared<unaryExpressionNode>(unaryExpressionNode(t, NEG, node));
            return una;
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

        std::shared_ptr<expressionNode> una = std::make_shared<unaryExpressionNode>(unaryExpressionNode(t, NOT, node));
        return una;
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
        std::shared_ptr<expressionNode> res = std::make_shared<variableNode>(std::move(node));
        return res;
    } else if (ctx->read()) {
        return visitRead(ctx->read());
    } else return nullptr; //should never reach;
}

antlrcpp::Any DST::visitArrayAccess(SmallParser::ArrayAccessContext *ctx) {
    std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
    std::string name = ctx->NAME()->getText();
    auto it = symboltables.find(ctx->NAME()->getText());
    Type t;
    auto tmp = it->second;
    if (node->getType() != intType || it == symboltables.end()) {
        t = errorType;
    } else if (it->second->getType() == arrayIntType) {
        t = intType;
    } else {
        t = boolType;
    }
    std::shared_ptr<variableNode> varnode = std::make_shared<variableNode>(variableNode(it->second->getType(), it->first));
    std::shared_ptr<expressionNode> res = std::make_shared<arrayAccessNode>(arrayAccessNode(t, node, varnode));
    return res;
}

antlrcpp::Any DST::visitLiteral(SmallParser::LiteralContext *ctx) {
    literalNode* result = visitChildren(ctx);
    return result;
}

antlrcpp::Any DST::visitArrayLiteral(SmallParser::ArrayLiteralContext *ctx) {
    std::vector<std::shared_ptr<expressionNode>> inter;
    auto a = ctx->expr();
    for (auto i : ctx->expr()) {
        inter.push_back(visitExpr(i));
    }
    std::shared_ptr<arrayLiteralNode> res = std::make_shared<arrayLiteralNode>(arrayLiteralNode(inter));
    return res;
}

std::string DST::btos (bool val) {
    return val ? "true" : "false";
}

std::pair<bool, std::shared_ptr<expressionNode>> DST::compute_new_literal (const std::shared_ptr<expressionNode> &l, const std::shared_ptr<expressionNode> &r, op expressionType, Type t) {
    std::string lVal = dynamic_cast<literalNode*>(l.get())->value;
    std::string rVal = dynamic_cast<literalNode*>(r.get())->value;
    std::shared_ptr<expressionNode> n;
    Type nodesType;
    int64_t lIntVal = 0;
    int64_t rIntVal = 0;
    bool lBoolVal = false;
    bool rBoolVal = false;
    bool success = true;
    if (l->getType() == intType) {
        lIntVal = std::stol(lVal);
        rIntVal = std::stol(rVal);
        nodesType = intType;
    } else {
        lBoolVal = lVal == "true";
        rBoolVal = rVal == "true";
        nodesType = boolType;
    }
    switch (expressionType) {
        case PLUS:
            lIntVal += rIntVal;
            if(lIntVal > INT16_MAX || lIntVal < INT16_MIN) success = false;
            n = std::make_shared<literalNode>(literalNode(t, std::to_string((int16_t)lIntVal)));
            break;
        case MINUS:
            lIntVal -= rIntVal;
            if(lIntVal > INT16_MAX || lIntVal < INT16_MIN) success = false;
            n = std::make_shared<literalNode>(literalNode(t, std::to_string((int16_t)lIntVal)));
            break;
        case MULT:
            lIntVal *= rIntVal;
            if(lIntVal > INT16_MAX || lIntVal < INT16_MIN) success = false;
            n = std::make_shared<literalNode>(literalNode(t, std::to_string((int16_t)lIntVal)));
            break;
        case DIV:
            if(rIntVal == 0) {
                success = false;
                n = std::make_shared<binaryExpressionNode>(binaryExpressionNode(t, DIV, l, r));
            }
            else {
                lIntVal /= rIntVal;
                n = std::make_shared<literalNode>(literalNode(t, std::to_string((int16_t)lIntVal)));
            }
            break;
        case MOD:
            if(rIntVal == 0) {
                success = false;
                n = std::make_shared<binaryExpressionNode>(binaryExpressionNode(t, MOD, l, r));
            } else {
                lIntVal %= rIntVal;
                n = std::make_shared<literalNode>(literalNode(t, std::to_string((int16_t)lIntVal)));
            }
            break;
        case AND:
            n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal && rBoolVal)));
            break;
        case OR:
            n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal || rBoolVal)));
            break;
        case LE:
            if (nodesType == intType) {
                n = std::make_shared<literalNode>(literalNode(t,btos(lIntVal < rIntVal)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal < rBoolVal)));
            }
            break;
        case LEQ:
            if (nodesType == intType) {
                n = std::make_shared<literalNode>(literalNode(t,btos(lIntVal <= rIntVal)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal <= rBoolVal)));
            }
            break;
        case GE:
            if (nodesType == intType) {
                n = std::make_shared<literalNode>(literalNode(t,btos(lIntVal > rIntVal)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal > rBoolVal)));
            }
            break;
        case GEQ:
            if (nodesType == intType) {
                n = std::make_shared<literalNode>(literalNode(t,btos(lIntVal >= rIntVal)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal >= rBoolVal)));
            }
            break;
        case EQ:
            if (nodesType == intType) {
                n = std::make_shared<literalNode>(literalNode(t,btos(lIntVal == rIntVal)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal == rBoolVal)));
            }
            break;
        case NEQ:
            if (nodesType == intType) {
                n = std::make_shared<literalNode>(literalNode(t,btos(lIntVal != rIntVal)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t,btos(lBoolVal != rBoolVal)));
            }
            break;
        case NOT:
            n = std::make_shared<literalNode>(literalNode(t, btos(!lBoolVal)));
            break;
        case NEG:
            if(lIntVal == INT16_MIN) {
                success = false;
                n = std::make_shared<literalNode>(literalNode(t,std::to_string(INT16_MIN)));
            } else {
                n = std::make_shared<literalNode>(literalNode(t, std::to_string(-lIntVal)));
            }
            break;
        default:
            std::cout << "this shouldn't happen\n";
    }
    return std::pair<bool, std::shared_ptr<expressionNode>>(success, n);
}

std::shared_ptr<expressionNode> DST::binary_expression (SmallParser::ExprContext *ctx, op expressionType) {
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
                updateErrorCount();
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
                updateErrorCount();
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
                updateErrorCount();
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
                updateErrorCount();
            }
            break;
        default:
            t = errorType;
            updateErrorCount();
            break;
    }
    if (t != errorType) {
        if (l->getNodeType() == Literal && r->getNodeType() == Literal) {
            return compute_new_literal(l,r, expressionType, t).second;
        }
    }

    std::shared_ptr<expressionNode> p = std::make_shared<binaryExpressionNode>(binaryExpressionNode(t, expressionType, l, r));
    return p;
}

/*
antlrcpp::Any DST::deepCopy(const node *tree) {
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
}
*/

/*std::shared_ptr<statementNode> DST::sequentialAssignForArray(std::string name, int accessor, std::vector<std::shared_ptr<expressionNode>> n) {
    Type t = n[0]->getType();
    if(n.size() == 1) {
        std::shared_ptr<expressionNode> access = std::make_shared<literalNode>(t, std::to_string(accessor));
        std::shared_ptr<arrayAccessNode> arrAcc = std::make_shared<arrayAccessNode>(arrayAccessNode(t,access,name));
        std::shared_ptr<statementNode> arrFieldAss = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(okType,arrAcc,n[0]));
        return arrFieldAss;
    } else {
        std::shared_ptr<expressionNode> access = std::make_shared<literalNode>(t,std::to_string(accessor));
        std::shared_ptr<arrayAccessNode> arrAcc = std::make_shared<arrayAccessNode>(arrayAccessNode(t,access,name));
        std::shared_ptr<statementNode> arrFieldAss = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(okType,arrAcc,n[0]));
        std::vector<std::shared_ptr<expressionNode>> ne;
        for (auto i = 1; i < n.size(); i++) {
            ne.push_back(n[i]);
        }
        return std::make_shared<sequentialNode>(sequentialNode(okType,arrFieldAss,sequentialAssignForArray(name,accessor+1,ne)));
    }
}*/
