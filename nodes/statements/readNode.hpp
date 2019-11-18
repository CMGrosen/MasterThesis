//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_READNODE_HPP
#define ANTLR_CPP_TUTORIAL_READNODE_HPP

#include <nodes/variableNode.hpp>

class readNode : public expressionNode {
public:
    readNode(Type t, std::shared_ptr<expressionNode> node) : n{std::move(node)} {setNodeType(Read); setType(t);}

    const expressionNode *getVar() const {return n.get();};
private:
    std::shared_ptr<expressionNode> n;
};

#endif //ANTLR_CPP_TUTORIAL_READNODE_HPP
