#include <iostream>

//#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallLexer.h"
#include "antlr4-runtime/SmallParser.h"
#include "DST.h"
#include "basicblockTreeConstructor.hpp"
#include <antlr4-runtime.h>
#include <CCFGIllustrator.hpp>
//#include <symengine/symbolicExecutionEngine.hpp>
#include <CSSA_CFG.hpp>

using namespace std;
using namespace antlr4;

static std::map< const char *, const char * > files = {
        {"concurrency", "../code_examples/concurrency.small"},
        {"out_of_bounds", "../code_examples/out_of_bounds.small"},
        {"race_condition", "../code_examples/race_condition.small"},
        {"bubblesort", "../code_examples/bubblesort.small"},
        {"stateTest", "../nodeptr_test.small"},
        {"precedence", "../precendenceTest.small"},
        {"temp", "../temp.small"},
        {"oob_race-condition", "../code_examples/out_of_bounds_race-condition.small"},
        {"testAssign", "../code_examples/testAssign.small"},
        {"coffee_maker", "../code_examples/coffee_maker.small"}
};

void do_stuff(basicBlockTreeConstructor test, const std::shared_ptr<statementNode> tree) {
    auto ccfg = test.get_ccfg(tree);

    std::cout << "got here  " << std::to_string(ccfg.startNode->get_number_of_blocks()) << "\n\n";//a << std::endl;

    //symbolicExecutionEngine symEngine;
    //auto constraintsToReachBug = symEngine.execute(treeAndSymbolTable);

//    symbolicExecutionEngine symEngine;
//    symEngine.execute(treeAndSymbolTable);
    std::cout << ccfg.startNode->draw_picture(&ccfg.edges) << "\n\n\n";

    auto first = CCFGTree(ccfg);
    std::cout << "made first:  " << std::to_string(ccfg.startNode->get_number_of_blocks()) << "\n";

    std::cout << first.DrawCCFG() << "\n";

    CSSA_CFG cssa = CSSA_CFG(ccfg);

    int c = cssa.ccfg->startNode->get_number_of_blocks();
    std::cout << "cssa: " << std::to_string(c) << "\n";

    std::cout << "finished\nAfter: " << std::to_string(cssa.ccfg->startNode->get_number_of_blocks()) << "\n";

}

int main(int argc, const char* argv[]) {
    std::ifstream stream;
    //stream.open("../code.small");
    stream.open(files["coffee_maker"]);
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

    //auto tmp = treeAndSymbolTable.first->debug_getAllNodes();
    if(visitor.getNumErrors() || treeAndSymbolTable.first->getType() == errorType)
        return 0;


    basicBlockTreeConstructor test;

    do_stuff(test, treeAndSymbolTable.first);

    std::shared_ptr<expressionNode> expr = std::make_shared<literalNode>(literalNode(intType, "10"));
    std::shared_ptr<statementNode> stmt = std::make_shared<assignNode>(assignNode(intType, "a", expr));
    basicblock b = basicblock(stmt);

    std::cout << "done: " << std::to_string(b.get_number_of_blocks()) << "\n";
    return 0;
}