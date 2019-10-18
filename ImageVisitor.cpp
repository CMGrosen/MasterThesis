//
// Created by hu on 18/10/2019.
//

#include "ImageVisitor.h"

antlrcpp::Any ImageVisitor::visitFile(SmallParser::FileContext *context) {
    return 0;
}

antlrcpp::Any ImageVisitor::visitDecls(SmallParser::DeclsContext *context) {
    return antlrcpp::Any();
}

antlrcpp::Any ImageVisitor::visitDecl(SmallParser::DeclContext *context) {
    return antlrcpp::Any();
}

/*antlrcpp::Any ImageVisitor::visitValues(SmallParser::ValuesContext *context) {
    return antlrcpp::Any();
}*/

antlrcpp::Any ImageVisitor::visitExpr(SmallParser::ExprContext *context) {
    return antlrcpp::Any();
}
