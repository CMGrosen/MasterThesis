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
    node startNode;
    std::vector<node> currentnodes;
    std::unordered_map<std::string, functionDeclarationNode> functionTable;
    int scopelvl = 0;
    int threadnumber = 0;
    int order = 0;
public:

    node getStartNode(){
        return startNode;
    }

    std::vector<std::unordered_map<std::string, symbol>> symboltables;

    virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
        //antlrcpp::Any result = visitChildren(ctx);
        std::vector<declarationNode> a = visitMain(ctx->main());
        std::cout << a.size() << std::endl;
        std::cout << "File " << order++ << std::endl;
 //       return result;
    }

    virtual antlrcpp::Any visitMain(SmallParser::MainContext *ctx) override {
        //std::cout << "main" << ctx->depth() << "\n";
        auto result = (visitDcls(ctx->scope()->dcls()));
        node _startNode = node();
        // pointer til gamle startNode
        std::cout << "Main " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitFuncs(SmallParser::FuncsContext *ctx) override {
        //std::cout << "functions" << ctx->depth() << "\n";
        auto result = visitChildren(ctx);
        std::cout << "Funcdcls " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitFunc(SmallParser::FuncContext *ctx) override {
        //std::cout << "func" << ctx->depth() << "\n";
        auto result = visitChildren(ctx);
        // gem funcNode i functionTable
        std::cout << "Funcdcl " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitDcls(SmallParser::DclsContext *ctx) override {
        std::vector<declarationNode> nodes;
        if(ctx->dcls()) {
            nodes.push_back((declarationNode) visitDcl(ctx->dcl()));
            std::vector<declarationNode> res = visitDcls(ctx->dcls());
            for (auto n : res)
                nodes.emplace_back(std::move(n));
            return nodes;
        }

        //auto result = visitChildren(ctx);
        std::cout << "Dcls " << order++ << std::endl;
        return nodes;
        //return result;
    }

    virtual antlrcpp::Any visitDcl(SmallParser::DclContext *ctx) override {
        //auto result = visitChildren(ctx);
        std::string name = ctx->NAME()->getText();
        if (ctx->assign()) {
            return declarationNode(name, visit(ctx->assign()));
            return declarationNode(name, visit(ctx->assign()));
        } else {
            // gem symbol i symbol table
            std::cout << "Dcl " << order++ << " " << ctx->getText() << std::endl;
            //return declarationNode(result.);
            return node();
            //return result;
        }
    }

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "Stmts " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *ctx) override {
        auto result = visitChildren(ctx);

        std::cout << "stmt " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *ctx) override {
        //std::cout << "assignment" << ctx->depth() << "\n";
        //auto result = visitChildren(ctx);

        additionNode a = visitExpr(ctx->expr());
        std::cout << "Assign " << order++  << std::endl;
        return expressionNode(a);
        return expressionNode(visitExpr(ctx->expr()));
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

    virtual antlrcpp::Any visitReturnStmt(SmallParser::ReturnStmtContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "return " << order++  << " " << ctx->getText() << std::endl;
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
        auto result = visitChildren(ctx);
        std::cout << "scope" << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitRead(SmallParser::ReadContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "read " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitWrite(SmallParser::WriteContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "write " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitOutput(SmallParser::OutputContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "output " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
        if(ctx->OP_ADD()) {
            std::cout << "add " << order++ << " " << ctx->getText() << std::endl;
            literalNode l = visitExpr(ctx->left);
            literalNode r = visitExpr(ctx->right);
            std::cout << "left: " << ctx->left;
            return additionNode(std::move(l),std::move(r));
        } else {
            std::cout << "literal " << order++ << " " << ctx->getText() << std::endl;
            return literalNode(std::stoi(ctx->literal()->getText()));
        }
        /*
        if (ctx->OP_SUB()) {
            node left = ctx->left->accept(this);
            node right = ctx->right->accept(this);

            if (left.getType() == right.getType()) {
                return subtractionNode(left, right);
            } else {
                std::cout << "Incorrect types at " << ctx->stop->getLine() << ":" << ctx->stop->getCharPositionInLine()
                << ".\n Expected: " << left.getType() << ", was: " << right.getType() << "\n";
                return subtractionNode(left, right);
            }
            //literalNode l = literalNode(std::stoi(ctx->right->getText()));
            //nameNode n = nameNode(ctx->left->getText());
            //subtractionNode subNode = subtractionNode{n, l};

        }
         */
        auto result = visitChildren(ctx);
        std::cout << "Expression " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "array " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitFunctionCall(SmallParser::FunctionCallContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "funcCall " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitParams(SmallParser::ParamsContext *ctx) override {
        auto result = visitChildren(ctx);
        
        return result;
    }

    virtual antlrcpp::Any visitParam(SmallParser::ParamContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "param " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "lit " << order++ << " " << ctx->getText() << std::endl;
        //return std::stoi(ctx->getText())
        return result;
    }


};