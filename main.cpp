#include <iostream>

#include "antlr4-runtime.h"
#include "SmallLexer.h"
#include "DST.hpp"
#include "basicblockTreeConstructor.hpp"
#include <CCFGIllustrator.hpp>
#include <symengine/symEngine.hpp>
#include <CSSA_CFG.hpp>
#include <SSA_CCFG.hpp>
#include <statementsTransformer.hpp>
#include <lengauerTarjan.hpp>
#include <symengine/interpreter.hpp>

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
        {"constraint_test", "../code_examples/constraint_test.small"},
        {"tripple_maker", "../code_examples/tripple_maker.small"},
        {"if_test", "../code_examples/if_test.small"},
        {"testUnwrap", "../code_examples/testUnwrap.small"},
        {"phitest", "../code_examples/phiTest.small"},
        {"multiplepaths", "../code_examples/multiple_paths.small"},
        {"loop_unroll_test", "../code_examples/while_unroll_test.small"},
        {"arrays", "../code_examples/array.small"}
};
static std::map< const char *, const char * > test_files = {
        {"event_test", "../code_examples/test_programs/event_test.small"},
        {"duo_maker_test", "../code_examples/test_programs/coffee_maker_duo.small"},
        {"trio_maker_test", "../code_examples/test_programs/coffee_maker_trio.small"},
        {"inner_while_test", "../code_examples/test_programs/coffee_maker_inner_while.small"},
        {"outer_while_test", "../code_examples/test_programs/coffee_maker_outer_while.small"},
        {"event_based_duo_test", "../code_examples/test_programs/coffee_maker_duo_event_based.small"},
        {"event_based_trio_test", "../code_examples/test_programs/coffee_maker_trio_event_based.small"},
        {"statements_after_event_test", "../code_examples/test_programs/statements_after_event.small"},
        {"small_concurrent_events_test", "../code_examples/test_programs/small_concurrent_events.small"},
        {"unreachable", "../code_examples/test_programs/unreachable.small"},
        {"multiple_uses", "../code_examples/test_programs/multiple_uses_in_concnode.small"},
        {"while", "../code_examples/test_programs/while_unroll_test.small"},
        {"if-false", "../code_examples/test_programs/if-false-not-possible.small"}
};

std::pair<const std::shared_ptr<statementNode>, std::unordered_map<std::string, std::shared_ptr<expressionNode>>>
parse_program(const std::string& path) {
    std::ifstream stream;
    stream.open(path);

    ANTLRInputStream input(stream);
    SmallLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    SmallParser parser(&tokens);

    SmallParser::FileContext* tree = parser.file();
    if (parser.getNumberOfSyntaxErrors())
        return {nullptr, std::unordered_map<std::string, std::shared_ptr<expressionNode>>{}};
    DST visitor;

    auto treeAndSymbolTable = visitor.getTree(tree);

    if(visitor.getNumErrors() || treeAndSymbolTable.first->getType() == errorType) {
        return {nullptr, std::unordered_map<std::string, std::shared_ptr<expressionNode>>{}};
    }
    return treeAndSymbolTable;
}

SSA_CCFG do_stuff(const std::shared_ptr<statementNode> &tree, std::unordered_map<std::string, std::shared_ptr<expressionNode>> table) {
    auto ccfg = std::make_shared<CCFG>(basicBlockTreeConstructor::get_ccfg(tree));

    std::shared_ptr<DomTree> dominatorTree = std::make_shared<DomTree>(DomTree(ccfg));

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

    auto symboltable = std::make_shared<std::unordered_map<std::string, std::shared_ptr<expressionNode>>>(table);

    SSA_CCFG ssa_ccfg = SSA_CCFG(ccfg, symboltable, dominatorTree);

    auto second = CCFGTree(*ssa_ccfg.ccfg);
    std::cout << "\n\nmade second:  \n" << second.DrawCCFG() << "\n";


    std::cout << ssa_ccfg.ccfg->startNode->statements[0]->getNodeType() << "\n";

    auto cssaccfg = std::make_shared<CSSA_CFG>(CSSA_CFG(*ssa_ccfg.ccfg, symboltable, ssa_ccfg.counter));

    auto third = CCFGTree(*cssaccfg->ccfg);
    std::cout << "\nmade third: \n" << third.DrawCCFG() << "\n";

    auto newccfg = statementsTransformer::get_transformedCCFG(cssaccfg);

    auto fourth = CCFGTree(*newccfg);
    std::cout << "\nmade fourth: \n" << fourth.DrawCCFG() << "\n";

    symEngine engine = symEngine(cssaccfg, std::move(table));
    //symEngine engine = symEngine(cssaccfg, treeAndSymbolTable->second);

    interpreter checker = interpreter(engine);
    if (checker.run()) {
        std::cout << "all good\n";
    } else {
        std::cout << "error\n";
    }


    //auto res = engine.execute();

    return ssa_ccfg;
}

void run(const std::string& path) {
    auto treeAndSymbolTable = parse_program(path);
    auto ccfg = std::make_shared<SSA_CCFG>(do_stuff(treeAndSymbolTable.first, std::move(treeAndSymbolTable.second)));
    std::cout << "done with run: " << std::to_string(basicblock::get_number_of_blocks()) << "\n";
}

int main(int argc, const char* argv[]) {

    run(test_files["while"]);
    //run(files["reportExample"]);

    std::cout << "done: " << std::to_string(basicblock::get_number_of_blocks()) << "\n";

    return 0;
}
