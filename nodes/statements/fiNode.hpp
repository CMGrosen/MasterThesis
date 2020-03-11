//
// Created by hu on 11/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_FINODE_HPP
#define ANTLR_CPP_TUTORIAL_FINODE_HPP


class fiNode : public statementNode {
    std::shared_ptr<basicblock> parent;
public:
    fiNode(std::shared_ptr<basicblock> parent) : parent{std::move(parent)} {
        nodetype = EndFi;
        type = okType;
    }

    basicblock *get_parent() {return parent.get();}
    void set_parent(std::shared_ptr<basicblock> blk) {parent = std::move(blk);}

    std::string to_string() override  {return "endfi";};
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> stmt = std::make_shared<fiNode>(fiNode(parent));
        return stmt;
    }
};


#endif //ANTLR_CPP_TUTORIAL_FINODE_HPP
