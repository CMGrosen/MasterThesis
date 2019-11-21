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
        {"race_condition", "../code_examples/race_condition.small"},
        {"bubblesort", "../code_examples/bubblesort.small"},
        {"stateTest", "../nodeptr_test.small"}
};

//definitely have to think carefully about how we do this
std::vector<state> f(state* s) {
    switch(s->get_position()->getNodeType()) {
        case BinaryExpression:
            return std::vector<state>();
        case Literal:
            return std::vector<state>();
        case Assign:
            if(auto a = dynamic_cast<assignNode*>(s->get_position())) {
                if (auto b = dynamic_cast<binaryExpressionNode*>(a->getExpr())) {
                    auto table = std::unordered_map<std::string, std::shared_ptr<expressionNode>>{};
                    std::shared_ptr<expressionNode> expr = DST::deepCopy(b);
                    if (expr->getNodeType() == BinaryExpression && expr->getType() == intType) {
                        if(((binaryExpressionNode*)expr.get())->getLeft()->getNodeType() == Literal
                        && ((binaryExpressionNode*)expr.get())->getRight()->getNodeType() == Literal) {
                            std::string lVal = ((literalNode*)((binaryExpressionNode*)expr.get())->getLeft())->value;
                            std::string rVal = ((literalNode*)((binaryExpressionNode*)expr.get())->getRight())->value;
                            table.insert({a->getName(), std::make_shared<literalNode>(expr->getType(), std::to_string(stoi(lVal) + stoi(rVal)))});
                        }
                    }
                    return std::vector<state>{state(nullptr, table)};
                }
            }
            return std::vector<state>();
        default:
            return std::vector<state>();
    }
}

int main(int argc, const char* argv[]) {
    std::ifstream stream;
    //stream.open("../code.small");
    stream.open(files["stateTest"]);
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

    auto table = std::unordered_map<std::string, std::shared_ptr<expressionNode>>();
    table.insert({"a", std::make_shared<binaryExpressionNode>(binaryExpressionNode(intType, PLUS, std::make_shared<literalNode>(literalNode(intType, "2")),std::make_shared<literalNode>(literalNode(intType, "2"))))});
    state no = state(a.get(), std::move(table));
    std::vector<state> succStates = no.get_successors(f);
    std::cout << "got here" << std::endl;//a << std::endl;
    return 0;
}