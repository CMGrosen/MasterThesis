//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

#include <typeinfo>

enum Type { intType, boolType, arrayIntType, arrayBoolType, okType, errorType};
enum NodeType { Assign, AssignArrField, Concurrent, Sequential, While, If, Write, Read, Literal, ArrayAccess, ArrayLiteral, Event, Variable, BinaryExpression, UnaryExpression, ConstraintNode};
enum op { PLUS, MINUS, MULT, DIV, MOD, NOT, AND, OR, LE, LEQ, GE, GEQ, EQ, NEQ, NEG, ASSIGNED, NOTUSED};

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
    node(Type type, NodeType nodeType) : type{type}, nodetype{nodeType} {};
    node(Type type, NodeType nodeType, op _operator) : type{type}, nodetype{nodeType}, _operator{_operator} {};
    node(Type type, NodeType nodeType, op _operator, std::string value) : type{type}, nodetype{nodeType}, _operator{_operator}, _val{std::move(value)} {};
    node(Type type, NodeType nodeType, std::string value) : type{type}, nodetype{nodeType}, _val{std::move(value)} {};
    Type getType() const {return type;};
    void setType(Type t) {type = t;};
    NodeType getNodeType() const {return nodetype;};
    void setNodeType(NodeType t) {nodetype = t;};
    op getOperator() const {return _operator;};
    void setOperator(op o) {_operator = o;};
    virtual std::string getValue() const {return _val;};
    std::vector<std::shared_ptr<node>> getNexts() const {return _nexts;}
    void setNext(std::shared_ptr<node> n) {_nexts = std::vector<std::shared_ptr<node>>{std::move(n)};}
    void setNexts(std::vector<std::shared_ptr<node>> n) {_nexts = std::move(n);}
    std::shared_ptr<node> getLast() const {
        if (_nexts.empty()) return nullptr;
        std::shared_ptr<node> tmp = _nexts[0];
        if (!tmp) tmp = _nexts[1];
        while (!(tmp->_nexts.empty())) {
            if(tmp->getNodeType() == While) {
                if(tmp->_nexts[1]) {
                    tmp = tmp->_nexts[1];
                } else {
                    return tmp;
                }
            } else if (tmp->getNodeType() == If) {
                if(tmp->_nexts[0]) {
                    tmp = tmp->_nexts[0];
                } else {
                    return tmp;
                }
            } else {
                tmp = tmp->_nexts[0];
            }
        }
        return tmp;
    }

protected:
    Type type;
    NodeType nodetype;
    op _operator = NOTUSED;
    std::vector<std::shared_ptr<node>> _nexts = std::vector<std::shared_ptr<node>>();
    std::string _val = "";
};

#endif //ANTLR_CPP_TUTORIAL_NODE_HPP
