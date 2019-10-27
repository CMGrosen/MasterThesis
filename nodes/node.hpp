//
// Created by hu on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_NODE_HPP
#define ANTLR_CPP_TUTORIAL_NODE_HPP

enum Type { intType, boolType, arrayType, okType, errorType, ignoreType};


class node {
public:
    virtual Type getType() {return type;};
    virtual void setType(Type t) {type = t;};
    virtual node *getNextStatement() {return next_statement.get();};
    virtual void setNextStatement(std::shared_ptr<node> p) {next_statement = std::move(p);};

protected:
    Type type;
    std::shared_ptr<node> next_statement;
};

#endif //ANTLR_CPP_TUTORIAL_NODE_HPP
