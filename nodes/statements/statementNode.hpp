//
// Created by hu on 11/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP

#include <nodes/node.hpp>
#include <string>

class statementNode : virtual public node {
private:
    std::string boolname;
public:
    //virtual std::vector<std::shared_ptr<statementNode>> debug_getAllNodes() {};
    virtual std::shared_ptr<statementNode> copy_statement() const = 0;
    void set_boolname(std::string name) {boolname = std::move(name);}
    std::string get_boolname() const {return boolname;}
    std::string boolname_as_tikz() const {return nameToTikzName(boolname, true);}
};

#endif //ANTLR_CPP_TUTORIAL_STATEMENTNODE_HPP
