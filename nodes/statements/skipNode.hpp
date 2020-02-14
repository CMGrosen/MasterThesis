//
// Created by hu on 12/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
#define ANTLR_CPP_TUTORIAL_SKIPNODE_HPP

class skipNode : public statementNode {
public:
    skipNode() {setNodeType(Skip); setType(okType);}

    std::string to_string() override {
        return "skip";
    }
};

#endif //ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
