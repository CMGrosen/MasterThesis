//
// Created by hu on 12/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
#define ANTLR_CPP_TUTORIAL_SKIPNODE_HPP

class skipNode : public statementNode {
public:
    skipNode(int linenum) {setNodeType(Skip); setType(okType); set_linenum(linenum);}

    std::string to_string() const override {
        return "skip";
    }
    std::string strOnSourceForm() const override {
        return "skip;";
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _this = std::make_shared<skipNode>(skipNode(get_linenum()));
        _this->setSSA(onSSA);
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
    }

};

#endif //ANTLR_CPP_TUTORIAL_SKIPNODE_HPP
