//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP

#include <nodes/expressions/expressionNode.hpp>

class arrayAccessNode : public expressionNode {
public:
    arrayAccessNode(Type t, std::shared_ptr<expressionNode> a, std::string n) : value{std::move(a)}, name{std::move(n)} {type = t; setNodeType(ArrayAccess);};

    expressionNode *getAccessor() const {return value.get();};
    std::string getName() const { return name;}
private:
    std::shared_ptr<expressionNode> value;
    std::string name;
};


#endif //ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
