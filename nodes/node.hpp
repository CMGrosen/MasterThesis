//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

enum Type { intType, boolType, arrayIntType, arrayBoolType, okType, errorType, ignoreType};
enum NodeType { Assign, Concurrent, Sequential, While, If, Write, Read, Literal, ArrayLiteral, Variable, BinaryExpression, UnaryExpression};


class node {
public:
    virtual Type getType() {return type;};
    virtual void setType(Type t) {type = t;};
    virtual NodeType getNodeType() {return nodetype;};
    virtual void setNodeType(NodeType t) {nodetype = t;};

protected:
    Type type;
    NodeType nodetype;
};

#endif //ANTLR_CPP_TUTORIAL_NODE_HPP
