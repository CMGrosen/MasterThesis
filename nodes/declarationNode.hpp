//
// Created by CMG on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_DECLARATIONNODE_H
#define ANTLR_CPP_TUTORIAL_DECLARATIONNODE_H

#include <nodes/node.hpp>
#include <string>

class declarationNode : public node {
public:
    declarationNode(Type _type, std::string _name, std::shared_ptr<expressionNode> _val) {
        type = _type;
        name = std::move(_name);
        value = std::move(_val);
    };

    /*
    declarationNode(Type _type, std::string _name){
        type = _type;
        name = std::move(_name);
        value = getStartValue();
    };
*/
    std::string name;
    std::shared_ptr<expressionNode> value;
    /*
private:
    std::shared_ptr<expressionNode> getStartValue(){
        std::shared_ptr<expressionNode> val;
        switch (type) {
            case  intType:
                val = std::shared_ptr<expressionNode>(new literalNode(0));
                break;
            default:
                val = std::shared_ptr<expressionNode>(new literalNode(0));
                break;
        }
        return val;
    }
*/
};
#endif //ANTLR_CPP_TUTORIAL_DECLARATIONNODE_H
