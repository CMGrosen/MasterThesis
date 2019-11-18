//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

enum Type { intType, boolType, arrayIntType, arrayBoolType, okType, errorType};
enum NodeType { Assign, Concurrent, Sequential, While, If, Write, Read, Literal, ArrayAccess, ArrayLiteral, Event, Variable, BinaryExpression, UnaryExpression};

static std::map< Type, const char * > info = {
        {intType, "int"},
        {boolType, "bool"},
        {arrayIntType, "int array"},
        {arrayBoolType, "bool array"},
        {okType, "well-typed"},
        {errorType, "not well-typed"}
};

class node {
public:
    virtual Type getType() const {return type;};
    virtual void setType(Type t) {type = t;};
    virtual NodeType getNodeType() const {return nodetype;};
    virtual void setNodeType(NodeType t) {nodetype = t;};

protected:
    Type type;
    NodeType nodetype;
};

#endif //ANTLR_CPP_TUTORIAL_NODE_HPP
