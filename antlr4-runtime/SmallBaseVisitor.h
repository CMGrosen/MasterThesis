
// Generated from Small.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "SmallVisitor.h"


/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SmallBaseVisitor : public SmallVisitor {
public:

  virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecls(SmallParser::DeclsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecl(SmallParser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValues(SmallParser::ValuesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
    return visitChildren(ctx);
  }


};

