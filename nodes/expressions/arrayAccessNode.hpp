//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP

#include <nodes/node.hpp>

class arrayAccessNode : public node {
public:
    arrayAccessNode(Type t, std::shared_ptr<node> a, std::string n) :
        node(t, ArrayAccess), value{std::move(a)}, name{std::move(n)} {};

    node *getAccessor() const {return value.get();};
    std::string getName() const { return name;}
private:
    std::shared_ptr<node> value;
    std::string name;
};


#endif //ANTLR_CPP_TUTORIAL_ARRAYACCESSNODE_HPP
