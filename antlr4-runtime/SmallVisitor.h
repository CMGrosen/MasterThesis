
// Generated from Small.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "SmallParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SmallParser.
 */
class  SmallVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SmallParser.
   */
    virtual antlrcpp::Any visitFile(SmallParser::FileContext *context) = 0;

    virtual antlrcpp::Any visitDecls(SmallParser::DeclsContext *context) = 0;

    virtual antlrcpp::Any visitDecl(SmallParser::DeclContext *context) = 0;

    virtual antlrcpp::Any visitValues(SmallParser::ValuesContext *context) = 0;

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *context) = 0;


};

