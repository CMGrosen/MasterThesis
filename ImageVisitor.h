#include <string>
#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallBaseVisitor.h"
//#include "Scene.h"

class  ImageVisitor : SmallVisitor {
public:
    virtual antlrcpp::Any visitFile(SmallParser::FileContext *context);

    virtual antlrcpp::Any visitMain(SmallParser::MainContext *context);

    virtual antlrcpp::Any visitFuncs(SmallParser::FuncsContext *context);

    virtual antlrcpp::Any visitFunc(SmallParser::FuncContext *context);

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *context);

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *context);

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *context);

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *context);

    virtual antlrcpp::Any visitDcls(SmallParser::DclsContext *context);

    virtual antlrcpp::Any visitDcl(SmallParser::DclContext *context);

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *context);

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *context);

    virtual antlrcpp::Any visitOrexpr(SmallParser::OrexprContext *context);

    virtual antlrcpp::Any visitAndexpr(SmallParser::AndexprContext *context);

    virtual antlrcpp::Any visitBexpr(SmallParser::BexprContext *context);

    virtual antlrcpp::Any visitAexpr1(SmallParser::Aexpr1Context *context);

    virtual antlrcpp::Any visitAexpr2(SmallParser::Aexpr2Context *context);

    virtual antlrcpp::Any visitTerm(SmallParser::TermContext *context);

    virtual antlrcpp::Any visitValue(SmallParser::ValueContext *context);

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *context);

    virtual antlrcpp::Any visitFunctionCall(SmallParser::FunctionCallContext *context);

    virtual antlrcpp::Any visitParams(SmallParser::ParamsContext *context);

    virtual antlrcpp::Any visitParam(SmallParser::ParamContext *context);

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *context);

};