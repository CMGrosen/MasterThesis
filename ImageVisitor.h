#include <string>
#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallBaseVisitor.h"
//#include "Scene.h"

class  ImageVisitor : SmallVisitor {
public:
    virtual antlrcpp::Any visitFile(SmallParser::FileContext *context);

    virtual antlrcpp::Any visitDecls(SmallParser::DeclsContext *context);

    virtual antlrcpp::Any visitDecl(SmallParser::DeclContext *context);

//    virtual antlrcpp::Any visitValues(SmallParser::ValuesContext *context);

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *context);


};