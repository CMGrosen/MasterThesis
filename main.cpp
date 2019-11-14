#include <iostream>

//#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallLexer.h"
#include "antlr4-runtime/SmallParser.h"
#include "DST.h"
#include <antlr4-runtime.h>
#include <symengine/state.hpp>

using namespace std;
using namespace antlr4;

int main(int argc, const char* argv[]) {
    std::ifstream stream;
    //stream.open("../code.small");
    stream.open("../precendenceTest.small");
    //stream.open("shortExpr.small");

    ANTLRInputStream input(stream);
    SmallLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    SmallParser parser(&tokens);

    SmallParser::FileContext* tree = parser.file();
    if (parser.getNumberOfSyntaxErrors())
        return 0;
    DST visitor;
    //int a = visitor.visitFile(tree);
    visitor.visitFile(tree);

    /*lexer.reset();
    tokens.reset();
    SmallBaseVisitor visitor;
    visitor.visitFile(tree);
*/
    std::cout << "got here" << std::endl;//a << std::endl;
    return 0;
}