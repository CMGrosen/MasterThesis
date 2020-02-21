//
// Created by CMG on 21/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_TEST_HPP
#define ANTLR_CPP_TUTORIAL_TEST_HPP
CCFGTree CreateTestCCFG( basicBlockTreeConstructor test){
    std::shared_ptr<statementNode> A = std::make_shared<assignNode>(
            assignNode(okType,"A", std::shared_ptr<expressionNode>(std::make_shared<literalNode>(literalNode("A")))));

    CCFGTree result = test.get_ccfg();

}
#endif //ANTLR_CPP_TUTORIAL_TEST_HPP
