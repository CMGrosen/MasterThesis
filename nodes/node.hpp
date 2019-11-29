//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

#include <typeinfo>

enum Type { intType, boolType, arrayIntType, arrayBoolType, okType, errorType};
enum NodeType { Assign, AssignArrField, Concurrent, Sequential, While, If, Write, Read, Literal, ArrayAccess, ArrayLiteral, Event, Variable, BinaryExpression, UnaryExpression, ConstraintNode};

static std::map< Type, const char * > info = {
        {intType, "int"},
        {boolType, "bool"},
        {arrayIntType, "int array"},
        {arrayBoolType, "bool array"},
        {okType, "well-typed"},
        {errorType, "not well-typed"}
};

static std::map< NodeType, const char * > nodeInfo = {
        {Assign, "Assign"},
        {AssignArrField, "Array Field Assign"},
        {Concurrent, "Concurrent"},
        {Sequential, "Sequential"},
        {While, "While"},
        {If, "If"},
        {Write, "Write"},
        {Read, "Read"},
        {Literal, "Literal"},
        {ArrayAccess, "ArrayAccess"},
        {ArrayLiteral, "ArrayLiteral"},
        {Event, "Event"},
        {Variable, "Variable"},
        {BinaryExpression, "BinaryExpression"},
        {UnaryExpression, "UnaryExpression"}
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
