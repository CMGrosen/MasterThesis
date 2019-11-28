#include <iostream>

//#include "antlr4-runtime/antlr4-runtime.h"
#include "antlr4-runtime/SmallLexer.h"
#include "antlr4-runtime/SmallParser.h"
#include "DST.h"
#include <antlr4-runtime.h>
#include <symengine/symbolTable.hpp>
#include "z3++.h"

using namespace std;
using namespace antlr4;

static std::map< const char *, const char * > files = {
        {"concurrency", "../code_examples/concurrency.small"},
        {"out_of_bounds", "../code_examples/out_of_bounds.small"},
        {"race_condition", "../code_examples/race_condition.small"},
        {"bubblesort", "../code_examples/bubblesort.small"},
        {"stateTest", "../nodeptr_test.small"},
        {"precedence", "../precendenceTest.small"},
        {"temp", "../temp.small"}
};

void demorgan() { //https://github.com/Z3Prover/z3/blob/31a6788859d4fc05d2b88080d58014c328aa7262/examples/c%2B%2B/example.cpp
    std::cout << "de-Morgan example\n";

    z3::context c;

    z3::expr x = c.bool_const("x");
    z3::expr y = c.bool_const("y");
    z3::expr conjecture = (!(x && y)) == (!x || !y);

    z3::solver s(c);
    // adding the negation of the conjecture as a constraint.
    s.add(!conjecture);
    //std::cout << s << "\n";
    //std::cout << s.to_smt2() << "\n";
    switch (s.check()) {
        case z3::unsat:   std::cout << "de-Morgan is valid\n"; break;
        case z3::sat:     std::cout << "de-Morgan is not valid\n"; break;
        case z3::unknown: std::cout << "unknown\n"; break;
    }
}

int main(int argc, const char* argv[]) {
    std::ifstream stream;
    //stream.open("../code.small");
    stream.open(files["temp"]);
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

    auto table = std::unordered_map<std::string, std::shared_ptr<expressionNode>>();
    table.insert({"a", std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, PLUS, std::make_shared<literalNode>(literalNode(intType, "2")),std::make_shared<literalNode>(literalNode(intType, "2"))))});
//    std::vector<state> succStates = no.get_successors(f);

    demorgan();

    std::cout << "got here" << std::endl;//a << std::endl;
    return 0;
}