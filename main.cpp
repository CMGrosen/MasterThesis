#include <iostream>

//#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallLexer.h"
#include "antlr4-runtime/SmallParser.h"
#include "DST.h"
#include "basicblockTreeConstructor.hpp"
#include <antlr4-runtime.h>
#include <CCFGIllustrator.hpp>
#include <symengine/symEngine.hpp>
#include <CSSA_CFG.hpp>
#include <dominatorTreeConstructor.hpp>
#include <SSA_CCFG.hpp>
#include <statementsTransformer.hpp>
#include <z3++.h>

using namespace std;
using namespace antlr4;

static std::map< const char *, const char * > files = {
        {"concurrency", "../code_examples/concurrency.small"},
        {"out_of_bounds", "../code_examples/out_of_bounds.small"},
        {"race_condition", "../code_examples/race_condition.small"},
        {"bubblesort", "../code_examples/bubblesort.small"},
        {"stateTest", "../code_examples/nodeptr_test.small"},
        {"precedence", "../code_examples/precendenceTest.small"},
        {"temp", "../code_examples/temp.small"},
        {"oob_race-condition", "../code_examples/out_of_bounds_race-condition.small"},
        {"testAssign", "../code_examples/testAssign.small"},
        {"coffee_maker", "../code_examples/coffee_maker.small"},
        {"idom_test", "../code_examples/idomTest.small"},
        {"reportExample", "../code_examples/report_example.small"},
        {"constraint_test", "../code_examples/constraint_test.small"}
};

SSA_CCFG do_stuff(basicBlockTreeConstructor test, std::pair<const std::shared_ptr<statementNode>, const std::unordered_map<std::string, std::shared_ptr<expressionNode>>> *treeAndSymbolTable) {
    auto ccfg = std::make_shared<CCFG>(test.get_ccfg(treeAndSymbolTable->first));

    std::shared_ptr<DominatorTree> dominatorTree = std::make_shared<DominatorTree>(DominatorTree(ccfg));

    std::cout << "got here  " << std::to_string(ccfg->startNode->get_number_of_blocks()) << "\n\n";//a << std::endl;

    //symbolicExecutionEngine symEngine;
    //auto constraintsToReachBug = symEngine.execute(treeAndSymbolTable);

//    symbolicExecutionEngine symEngine;
//    symEngine.execute(treeAndSymbolTable);
    //std::cout << ccfg->startNode->draw_picture(&ccfg->edges) << "\n\n\n";

    auto first = CCFGTree(*ccfg);
    std::cout << "made first:  " << std::to_string(ccfg->startNode->get_number_of_blocks()) << "\n";

    std::cout << first.DrawCCFG() << "\n";

    /*CSSA_CFG cssa = CSSA_CFG(ccfg);

    int c = cssa.ccfg->startNode->get_number_of_blocks();
    std::cout << "cssa: " << std::to_string(c) << "\n";

    std::cout << "finished\nAfter: " << std::to_string(cssa.ccfg->startNode->get_number_of_blocks()) << "\n";*/

    auto symboltable = std::make_shared<std::unordered_map<std::string, std::shared_ptr<expressionNode>>>(
            treeAndSymbolTable->second);

    SSA_CCFG ssa_ccfg = SSA_CCFG(ccfg, symboltable, dominatorTree);

    auto second = CCFGTree(*ssa_ccfg.ccfg);
    std::cout << "\n\nmade second:  \n" << second.DrawCCFG() << "\n";


    std::cout << ssa_ccfg.ccfg->startNode->statements[0]->getNodeType() << "\n";

    auto cssaccfg = std::make_shared<CSSA_CFG>(CSSA_CFG(*ssa_ccfg.ccfg, dominatorTree, symboltable));

    auto third = CCFGTree(*cssaccfg->ccfg);
    std::cout << "\nmade third: \n" << third.DrawCCFG() << "\n";

    auto newccfg = statementsTransformer::get_transformedCCFG(cssaccfg);

    auto fourth = CCFGTree(*newccfg);
    std::cout << "\nmade fourth: \n" << fourth.DrawCCFG() << "\n";

    symEngine engine = symEngine(cssaccfg, treeAndSymbolTable->second);
    //symEngine engine = symEngine(cssaccfg, treeAndSymbolTable->second);

    //auto res = engine.execute();

    //auto res = engine.execute();

    return std::move(ssa_ccfg);
}



int main(int argc, const char* argv[]) {
    std::ifstream stream;
    //stream.open("../code.small");
    stream.open(files["constraint_test"]);
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
    if(visitor.getNumErrors())// || treeAndSymbolTable.first->getType() == errorType)
        return 0;


    basicBlockTreeConstructor test;

    auto ccfg = std::make_shared<SSA_CCFG>(do_stuff(test, &treeAndSymbolTable));

    std::shared_ptr<expressionNode> expr = std::make_shared<literalNode>(literalNode(intType, "10"));
    std::shared_ptr<statementNode> stmt = std::make_shared<assignNode>(assignNode(intType, "a", expr));
    basicblock b = basicblock(stmt);

    std::cout << "done: " << std::to_string(b.get_number_of_blocks()) << "\n";
    //int a = 50, p = 2, s = 1;
    //a = p = s = 3;
    //std::cout << std::to_string(a) << std::to_string(p) << std::to_string(s) << std::endl;

/*
    z3::context c;

    z3::solver s = z3::solver(c);


    z3::expr _a1 = c.int_const("_a1");
    s.add(a0 <= INT16_MAX && a0 >= INT16_MIN);

    z3::expr b0 = c.int_const("b0");
    s.add(b0 == 0);


    //true

    z3::expr cond_true = a0 > 10;
    z3::expr b1 = c.int_val(50);
    z3::expr true1 = b1 == 50;
    z3::expr trueBranch = cond_true && true1;

    //false
    z3::expr cond_false = !(a0 > 10);
    z3::expr b2 = c.int_val(20);
    z3::expr false1 = b2 == 20;
    z3::expr falseBranch = cond_false && false1;

    s.add(trueBranch || falseBranch);
    //z3::expr b3 = b1 || b2;
    //s.add(b3);

    if (s.check() == z3::sat) {
        auto model = s.get_model();
        std::cout << "sat\n" << model << std::endl;
    } else {
        std::cout << "unsat" << std::endl;
    }
*/


    return 0;
}
