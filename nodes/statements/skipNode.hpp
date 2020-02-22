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
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _this = std::make_shared<skipNode>(skipNode());
        _this->setSSA(onSSA);
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
    }
};

#endif //ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
