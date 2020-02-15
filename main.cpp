#include <iostream>

//#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallLexer.h"
#include "antlr4-runtime/SmallParser.h"
#include "DST.h"
#include "basicblockTreeConstructor.hpp"
#include <antlr4-runtime.h>
#include <CCFGIllustrator.hpp>
//#include <symengine/symbolicExecutionEngine.hpp>

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

//    auto table = std::unordered_map<std::string, std::shared_ptr<expressionNode>>();
//    table.insert({"a", std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, PLUS, std::make_shared<literalNode>(literalNode(intType, "2")),std::make_shared<literalNode>(literalNode(intType, "2"))))});
//    std::vector<state> succStates = no.get_successors(f);

    basicBlockTreeConstructor test;
    //auto a = test.get_tree(treeAndSymbolTable.first);
    //auto b = test.blocks;
    auto ccfg = test.get_ccfg(treeAndSymbolTable.first);
    std::cout << "got here" << std::endl;//a << std::endl;

    //symbolicExecutionEngine symEngine;
    //auto constraintsToReachBug = symEngine.execute(treeAndSymbolTable);

//    symbolicExecutionEngine symEngine;
//    symEngine.execute(treeAndSymbolTable);
//    std::cout << "\n" << ccfg.startNode->draw_picture(&ccfg.edges) << "\n";


    auto first = std::make_shared<CCFGNode>(CCFGNode(nullptr, ccfg.startNode));

    std::set<basicblock *> addedBlocks = std::set<basicblock *>{ccfg.startNode.get()};
    first->construct_graph(first, &addedBlocks);
    std::cout << "\n" << first->to_string();
    std::cout << "finished\n";

    return 0;
}