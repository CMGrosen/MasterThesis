//
// Created by hu on 18/10/2019.
//

#include "ImageVisitor.h"

antlrcpp::Any ImageVisitor::visitFile(SmallParser::FileContext *context) {
    std::cout << "file\n";
    return 0;
}

antlrcpp::Any ImageVisitor::visitMain(SmallParser::MainContext *context) {
    std::cout << "main\n";
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitFuncs(SmallParser::FuncsContext *context) {
    std::cout << "funcs\n";
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitFunc(SmallParser::FuncContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitStmts(SmallParser::StmtsContext *context) {
    std::cout << "stms\n";
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitStmt(SmallParser::StmtContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitIter(SmallParser::IterContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitIfs(SmallParser::IfsContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitDcls(SmallParser::DclsContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitDcl(SmallParser::DclContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitAssign(SmallParser::AssignContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitExpr(SmallParser::ExprContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitOrexpr(SmallParser::OrexprContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitAndexpr(SmallParser::AndexprContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitBexpr(SmallParser::BexprContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitAexpr1(SmallParser::Aexpr1Context *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitAexpr2(SmallParser::Aexpr2Context *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitTerm(SmallParser::TermContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitValue(SmallParser::ValueContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitArrayAccess(SmallParser::ArrayAccessContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitFunctionCall(SmallParser::FunctionCallContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitParams(SmallParser::ParamsContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitParam(SmallParser::ParamContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitLiteral(SmallParser::LiteralContext *context) {
    return antlrcpp::Any();
}
