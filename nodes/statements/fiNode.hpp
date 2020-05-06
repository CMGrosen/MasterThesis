//
// Created by hu on 11/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_FINODE_HPP
#define ANTLR_CPP_TUTORIAL_FINODE_HPP


class fiNode : public statementNode {
    std::set<std::shared_ptr<basicblock>> parents;

    fiNode(std::set<std::shared_ptr<basicblock>> blks, std::shared_ptr<basicblock> blk) : parents{std::move(blks)}, first_parent{std::move(blk)} {
        nodetype = EndFi;
        type = okType;
        set_linenum(-1);
    }

public:
    std::shared_ptr<basicblock> first_parent;
    fiNode(const std::shared_ptr<basicblock>& parent) {
        if (parent) { parents = {parent}; first_parent = parent; }
        nodetype = EndFi;
        type = okType;
    }

    std::set<std::shared_ptr<basicblock>> *get_parents() {return &parents;}
    void set_parents(std::set<std::shared_ptr<basicblock>> blks) {parents = std::move(blks);}
    void add_parent(std::shared_ptr<basicblock> blk) {parents.insert(std::move(blk));}

    std::string to_string() const override  {return "endfi";};
    std::string strOnSourceForm() const override {
        return to_string();
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> stmt = std::make_shared<fiNode>(fiNode(parents, first_parent));
        return stmt;
    }
};


#endif //ANTLR_CPP_TUTORIAL_FINODE_HPP
