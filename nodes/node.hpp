//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

enum Type { intType, boolType, arrayType, okType, errorType, ignoreType};


class node {
public:
    Type getType() {return type;};
    void setType(Type t) {type = t;};

protected:
    Type type;
};

#endif //ANTLR_CPP_TUTORIAL_NODE_HPP
