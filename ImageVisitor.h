#include "antlr4-runtime.h"
#include "SmallVisitor.h"
#include "scope.hpp"


/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ImageVisitor : public SmallVisitor {
private:
    int scopelvl = 0;
    int nnnn = 0;
public:
    std::vector<std::unordered_map<std::string, symbol>> symboltables;

    virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
        std::cout << "file" << ctx->depth() << "\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitMain(SmallParser::MainContext *ctx) override {
        std::cout << "main" << ctx->depth() << "\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFuncs(SmallParser::FuncsContext *ctx) override {
        std::cout << "functions" << ctx->depth() << "\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFunc(SmallParser::FuncContext *ctx) override {
        std::cout << "func" << ctx->depth() << "\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitDcls(SmallParser::DclsContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitDcl(SmallParser::DclContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *ctx) override {
        std::cout << "assignment" << ctx->depth() << "\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *ctx) override {
        std::cout << "while" << ctx->depth() << "\n";
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitReturnStmt(SmallParser::ReturnStmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitThread(SmallParser::ThreadContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitThreads(SmallParser::ThreadsContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitEvent(SmallParser::EventContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitScope(SmallParser::ScopeContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitRead(SmallParser::ReadContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitWrite(SmallParser::WriteContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitOutput(SmallParser::OutputContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitOrexpr(SmallParser::OrexprContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitAndexpr(SmallParser::AndexprContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitBexpr(SmallParser::BexprContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitAexpr1(SmallParser::Aexpr1Context *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitAexpr2(SmallParser::Aexpr2Context *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitTerm(SmallParser::TermContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitValue(SmallParser::ValueContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFunctionCall(SmallParser::FunctionCallContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitParams(SmallParser::ParamsContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitParam(SmallParser::ParamContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *ctx) override {
        return visitChildren(ctx);
    }


};