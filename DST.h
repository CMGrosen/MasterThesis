#include "antlr4-runtime.h"
#include "SmallVisitor.h"
#include <nodes/nodes.hpp>
#include "symbol.hpp"


/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  DST : public SmallVisitor {
private:
    //node startNode;
    std::vector<node> currentnodes;
//    std::unordered_map<std::string, functionDeclarationNode> functionTable;
    int scopelvl = 0;
    int threadnumber = 0;
    int order = 0;
    std::shared_ptr<node> firstStatement;
    std::shared_ptr<node> prevStatement;
public:
/*
    node getStartNode(){
        return startNode;
    }
*/
    std::unordered_map<std::string, symbol> symboltables;

    virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
        //antlrcpp::Any result = visitChildren(ctx);
        std::shared_ptr<statementNode> a = visitStmts(ctx->stmts());

        std::cout << "\n\n\nwriting out what we have:\n";
  //      WriteType(a);
        //std::cout << dynamic_cast<literalNode*>(dynamic_cast<additionNode*>(a[0]->value)->getLeft())->value << std::endl;
        //a->setNextStatement(firstStatement);
        std::cout << "File " << order++ << std::endl;
        return 0;
    }

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *ctx) override {
        if(ctx->stmts()) {
            sequentialNode node;
            std::shared_ptr<statementNode> intermediate = visitStmt(ctx->stmt());
            node.setBody(intermediate);
            std::shared_ptr<statementNode> next_seq = visitStmts(ctx->stmts());
            node.setNext(next_seq);
            std::shared_ptr<statementNode> result = std::make_shared<sequentialNode>(node);
            return result;
        } else {
            std::shared_ptr<statementNode> result = visitStmt(ctx->stmt());
            return result;
        }
    }

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *ctx) override {
        //auto result = visitChildren(ctx);
            //std::string name = ctx->NAME()->getText();
        if (ctx->assign()) {
            std::shared_ptr<statementNode> stmt =  visitAssign(ctx->assign());
        if (prevStatement) {
            prevStatement->setNextStatement(stmt);
        }
            prevStatement = stmt;
            return stmt;
        } else if (ctx->write()) {
            std::shared_ptr<statementNode> stmt = visitWrite(ctx->write());
            return stmt;
        } else if (ctx->read()) {
            std::shared_ptr<statementNode> stmt = visitRead(ctx->read());
            return stmt;
        } else {
            // gem symbol i symbol table
            std::cout << "Dcl " << order++ << " " << ctx->getText() << std::endl;
            //return declarationNode(result.);
            //return node();
            //return result;
        }
    }

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *ctx) override {
        //std::cout << "assignment" << ctx->depth() << "\n";
        //auto result = visitChildren(ctx);
        std::cout << "Assign " << order++  << std::endl;
        std::shared_ptr<expressionNode> node = visitExpr(ctx->expr());
        assignNode assNode = assignNode(ctx->NAME()->getText(), node);
        std::shared_ptr<statementNode> a = std::make_shared<assignNode>(assNode);
        auto pair = symboltables.insert({ctx->NAME()->getText(), symbol(ctx->NAME()->getText(), node->getType())});
        if(!pair.second && pair.first->second.type != node->getType())
            pair.first->second.type = errorType;
//        std::shared_ptr<assignNode> res = std::make_shared<assignNode>(assignNode(ctx->NAME()->getText(), node));
        return a;
        //return result;
    }

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *ctx) override {
        //std::cout << "while" << ctx->depth() << "\n";
        auto result = visitChildren(ctx);
        std::cout << "While " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "if " << order++  << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitThread(SmallParser::ThreadContext *ctx) override {
        threadnumber++;
        auto result = visitChildren(ctx);
        threadnumber--;
        return result;
    }

    virtual antlrcpp::Any visitEvent(SmallParser::EventContext *ctx) override {
        auto result = visitChildren(ctx);
        
        return result;
    }

    virtual antlrcpp::Any visitScope(SmallParser::ScopeContext *ctx) override {
        std::vector<std::shared_ptr<node>> dcls = visitStmts(ctx->stmts());
        std::cout << "scope" << std::endl;
        std::shared_ptr<scopeNode> result = std::make_shared<scopeNode>(scopeNode(Type::okType));
        if (prevStatement) {
            prevStatement->setNextStatement(result);
        }
        prevStatement = result;
        return result;
    }

    virtual antlrcpp::Any visitRead(SmallParser::ReadContext *ctx) override {
        auto symbol = symboltables.find(ctx->NAME()->getText());
        std::string name = ctx->NAME()->getText();
        Type type = errorType;
        if (symbol != symboltables.end() && symbol->second.type != errorType) {
            type = symbol->second.type;
            name = symbol->second.name;
        }
        std::shared_ptr<variableNode> nameNode = std::make_shared<variableNode>(variableNode(type, name));
        readNode node = readNode(nameNode);
        std::shared_ptr<statementNode> res = std::make_shared<readNode>(node);
        return res;
    }

    virtual antlrcpp::Any visitWrite(SmallParser::WriteContext *ctx) override {
        std::shared_ptr<expressionNode> expr = visitExpr(ctx->expr());
        writeNode wNode = writeNode(expr);
        std::shared_ptr<statementNode> node = std::make_shared<writeNode>(wNode);
        return node;
    }

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
        std::cout << ctx->OP_ADD() << " " << ctx->OP_SUB() << " " << ctx->OP_MUL()
        << " " << ctx->OP_DIV() << " " << ctx->OP_MOD() << " " << ctx->literal() << "\n";
        if(ctx->OP_ADD()) {
            std::cout << "add " << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '+');
        } else if (ctx->OP_SUB()){
            std::cout << "sub" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '-');
        } else if (ctx->OP_MUL()) {
            std::cout << "mul" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '*');
        } else if (ctx->OP_DIV()) {
            std::cout << "div" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '/');
        } else if (ctx->OP_MOD()) {
            std::cout << "mod" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '%');
        } else if (ctx->literal()){
            std::cout << "literal " << order++ << " " << ctx->getText() << std::endl;
            std::shared_ptr<expressionNode> p = (std::shared_ptr<expressionNode>)std::make_shared<literalNode>(literalNode(std::stoi(ctx->literal()->getText())));
            if (prevStatement) {
                prevStatement->setNextStatement(p);
            }
            prevStatement = p;
            if (!firstStatement) firstStatement = p;
            return p;
        } else if (ctx->NAME()) {
            auto pair = symboltables.find(ctx->NAME()->getText());
            variableNode node = (pair != symboltables.end())
                ? variableNode(pair->second.type, pair->second.name)
                : variableNode(errorType, ctx->NAME()->getText())
                ;
            std::shared_ptr<expressionNode> res = std::make_shared<variableNode>(node);
            return res;
        }
        //auto result = visitChildren(ctx);
        std::cout << "Expression " << order++ << " " << ctx->getText() << std::endl;
        //return result;
    }

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "array " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *ctx) override {
        literalNode* result = visitChildren(ctx);
        std::cout << "lit " << order++ << " " << ctx->getText() << std::endl;
        //return std::stoi(ctx->getText())
        return result;
    }

    virtual antlrcpp::Any visitArrayLiteral(SmallParser::ArrayLiteralContext *ctx) override {
        std::vector<expressionNode*> res;
        for (int i = 0; i < ctx->expr().size(); ++i) {
            res.push_back(visitExpr(ctx->expr(i)));
        }
        return res;
    }

    void WriteType(const std::shared_ptr<node> input){
        WriteType(input.get());
    }
    void WriteType(node* input){
        if (auto add = dynamic_cast<additionNode*>(input)) {
            std::cout << "add" << std::endl;
            WriteType(add->getLeft());
            std::cout << add->getOperator() << std::endl;
            WriteType(add->getRight());
        }  else if (auto sub = dynamic_cast<subtractionNode*>(input)) {
            std::cout << "sub" << std::endl;
            WriteType(sub->getLeft());
            std::cout << sub->getOperator() << std::endl;
            WriteType(sub->getRight());
        }  else if (auto mul = dynamic_cast<multiplicationNode*>(input)) {
            std::cout << "mul" << std::endl;
            WriteType(mul->getLeft());
            std::cout << mul->getOperator() << std::endl;
            WriteType(mul->getRight());
        } else if (auto div = dynamic_cast<divisionNode*>(input)) {
            std::cout << "div" << std::endl;
            WriteType(div->getLeft());
            std::cout << div->getOperator() << std::endl;
            WriteType(div->getRight());
        } else if (auto mod = dynamic_cast<moduloNode*>(input)) {
            std::cout << "mod" << std::endl;
            WriteType(mod->getLeft());
            std::cout << mod->getOperator() << std::endl;
            WriteType(mod->getRight());
        } else if (auto lit = dynamic_cast<literalNode*>(input)) {
            std::cout << "lit" << std::endl;
        } else if (auto expr = dynamic_cast<expressionNode*>(input)) {
            std::cout << "expr" << std::endl;
            WriteType(expr);
        } else if (auto ass = dynamic_cast<assignNode*>(input)) {
            std::cout << "assign" << std::endl;
            std::cout << ass->getName() << std::endl;
            WriteType(ass->getExpr());
        }

        else if (auto stmt = dynamic_cast<statementNode*>(input)) {
            std::cout << "stmt" << std::endl;
            std::cout << stmt->getNodeType() << std::endl;
            std::cout << Assign;
            WriteType(stmt);
        }

        else {
            std::cout << "failure" << std::endl;
        }
    }
    std::shared_ptr<expressionNode> binary_expression (SmallParser::ExprContext *ctx, char expressionType) {
        std::shared_ptr<expressionNode> l = (visitExpr(ctx->left));
        if (prevStatement) {
            prevStatement->setNextStatement(l);
        }
        prevStatement = l;
        std::shared_ptr<expressionNode> r = (visitExpr(ctx->right));
        if (prevStatement) {
            prevStatement->setNextStatement(r);
        }
        prevStatement = r;
        Type t;
        if (l->getType() == r->getType()) {
            t = l->getType();
        } else {
            t = errorType;
        }
        std::shared_ptr<expressionNode> p;
        switch (expressionType) {
            case '+':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<additionNode>(additionNode(t,l,r));
                break;
            case '-':
                p = (std::shared_ptr<expressionNode>)std::make_shared<subtractionNode>(subtractionNode(t,l,r));
                break;
            case '*':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<multiplicationNode>(multiplicationNode(t,l,r));
                break;
            case '/':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<divisionNode>(divisionNode(t,l,r));
                break;
            case '%':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<moduloNode>(moduloNode(t,l,r));
                break;
            default:
                p =  nullptr;
                break;
        }
        if (prevStatement) {
            prevStatement->setNextStatement(p);
        }
        prevStatement = p;
        //p->setNextStatement(nextStatement);
        //nextStatement = p;
        return p;
    }
};