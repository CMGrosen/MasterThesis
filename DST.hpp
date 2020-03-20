
#ifndef DST_HPP
#define DST_HPP
#include "SmallVisitor.h"
#include <nodes/nodes.hpp>

#define MAXITER 3

/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */


class DST : public SmallVisitor {
    int early_exits = 0;
    int foundErrors = 0;
public:
    int getNumErrors() const;
    void updateErrorCount();

    std::unordered_map<std::string, std::shared_ptr<expressionNode>> symboltables;

    std::pair<const std::shared_ptr<statementNode>, const std::unordered_map<std::string, std::shared_ptr<expressionNode>>> getTree(SmallParser::FileContext *ctx);

    antlrcpp::Any visitFile(SmallParser::FileContext *context) override;

    antlrcpp::Any visitStmts(SmallParser::StmtsContext *context) override;

    antlrcpp::Any visitStmt(SmallParser::StmtContext *context) override;

    antlrcpp::Any visitAssign(SmallParser::AssignContext *context) override;

    antlrcpp::Any visitIter(SmallParser::IterContext *context) override;

    antlrcpp::Any visitIfs(SmallParser::IfsContext *context) override;

    antlrcpp::Any visitThread(SmallParser::ThreadContext *context) override;

    antlrcpp::Any visitEvent(SmallParser::EventContext *context) override;

    antlrcpp::Any visitSkipStmt(SmallParser::SkipStmtContext *context) override;

    antlrcpp::Any visitScope(SmallParser::ScopeContext *context) override;

    antlrcpp::Any visitRead(SmallParser::ReadContext *context) override;

    antlrcpp::Any visitWrite(SmallParser::WriteContext *context) override;

    antlrcpp::Any visitExpr(SmallParser::ExprContext *context) override;

    antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *context) override;

    antlrcpp::Any visitLiteral(SmallParser::LiteralContext *context) override;

    antlrcpp::Any visitArrayLiteral(SmallParser::ArrayLiteralContext *context) override;

    static std::string btos (bool);

    static std::pair<bool, std::shared_ptr<expressionNode>> compute_new_literal (const std::shared_ptr<expressionNode>&, const std::shared_ptr<expressionNode>&, op, Type);

    std::shared_ptr<expressionNode> binary_expression (SmallParser::ExprContext *, op);

    //static antlrcpp::Any deepCopy(const node *);

    //std::shared_ptr<statementNode> sequentialAssignForArray(std::string, int, std::vector<std::shared_ptr<expressionNode>>);
};

#endif //DST_HPP