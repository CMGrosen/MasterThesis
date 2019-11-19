#include <iostream>

//#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallLexer.h"
#include "antlr4-runtime/SmallParser.h"
#include "DST.h"
#include <antlr4-runtime.h>
#include <symengine/state.hpp>

using namespace std;
using namespace antlr4;

static std::map< const char *, const char * > files = {
        {"concurrency", "../code_examples/concurrency.small"},
        {"out_of_bounds", "../code_examples/out_of_bounds.small"},
        {"race_condition", "../code_examples/race_condition.small"}
};

int main(int argc, const char* argv[]) {
    std::ifstream stream;
    //stream.open("../code.small");
    stream.open(files["concurrency"]);
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
    auto treeAndSymbolTable = visitor.getTree(tree);

    auto tmp = treeAndSymbolTable.first->debug_getAllNodes();
    if(treeAndSymbolTable.first->getType() == errorType)
        return 0;

    typeof(treeAndSymbolTable.first) a = DST::deepCopy(treeAndSymbolTable.first.get());
    std::cout << "got here" << std::endl;//a << std::endl;
    return 0;
}