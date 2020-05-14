//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

#include <typeinfo>
#include <map>
#include <vector>
#include <memory>
#include <string>


enum Type { intType, boolType, arrayIntType, arrayBoolType, okType, errorType};
enum NodeType { Assign, AssignArrField, Concurrent, EndConcurrent, Sequential, While, If, EndFi, Write, Read, Literal, ArrayAccess, ArrayLiteral, Event, Variable, BinaryExpression, UnaryExpression, Skip, BasicBlock, Phi, Pi, Assert};

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
    virtual std::string to_string() const = 0;
    virtual std::string strOnSourceForm() const = 0;
    virtual Type getType() const {return type;};
    virtual void setType(Type t) {type = t;};
    virtual NodeType getNodeType() const {return nodetype;};
    virtual void setNodeType(NodeType t) {nodetype = t;};
    virtual void setSSA(bool t) {onSSA = t;};
    virtual bool isSSAForm() const {return onSSA;};

    static std::string nameToTikzName(std::string name, bool SSAForm) {
        std::string newName = name[0] == '-' ? name.substr(1) : name;
        int length = newName.length();
        for (int i = 0; i < length; i++) {
            if (newName[i] == '_') {
                newName[i] = '\\';
                i++;
                newName = newName.substr(0, i) + '_' + newName.substr(i);
                length += 1;
            }
        }
        if (SSAForm) {
            length = newName.length();
            for (int i = length - 1; i >= 0; --i) {
                if (newName[i] == '_') {
                    newName[i - 1] = '$';
                    newName[i] = '_';
                    i++;
                    std::string num = newName.substr(i);
                    return newName.substr(0,i) + '{' + num + "}$";
                }
            }
        }
        return newName;
    }

protected:
    Type type = errorType;
    NodeType nodetype = Skip;
    bool onSSA = false;
};

#endif //ANTLR_CPP_TUTORIAL_NODE_HPP
