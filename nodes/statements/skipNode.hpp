//
// Created by hu on 12/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
#define ANTLR_CPP_TUTORIAL_SKIPNODE_HPP

class skipNode : virtual public statementNode {
public:
    skipNode() {setNodeType(Skip); setType(okType);}

    std::string to_string() const override {
        return "skip";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _this = std::make_shared<skipNode>(skipNode());
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
    }
};

#endif //ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
