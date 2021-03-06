//
// Created by hu on 19/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP

#include <src/nodes/statements/statementNode.hpp>
#include <src/nodes/expressions/expressionNodes.hpp>
#include <string>

class arrayFieldAssignNode : public statementNode {
public:
    arrayFieldAssignNode (Type t, std::string _name, std::shared_ptr<expressionNode> arrField, std::shared_ptr<expressionNode> n, int linenum) : name{std::move(_name)}, field{std::move(arrField)}, expr{std::move(n)} {
        setType(t);
        set_linenum(linenum);
        origName = name;
        setNodeType(AssignArrField);
    };

    //arrayAccessNode* getField() const {return field.get();}
    expressionNode* getField() const {return field.get();}
    expressionNode* getExpr() const {return expr.get();}
    std::string getName() const {return name;}
    std::string getOriginalName() const {return origName;}
    void setName(std::string _name) {name = _name;}

    std::string to_string() const override {
        std::string res = nameToTikzName(name, onSSA) + "[" + field->to_string() + "] = " + expr->to_string();
        return res;
    }
    std::string strOnSourceForm() const override {
        return origName + "[" + field->strOnSourceForm() + "] = " + expr->strOnSourceForm() + ";";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _field = field->copy_expression();
        //std::shared_ptr<arrayAccessNode> _field = std::make_shared<arrayAccessNode>(arrayAccessNode(field->getType(), _fieldExpr, name));
        std::shared_ptr<expressionNode> _expr = expr->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<arrayFieldAssignNode>(arrayFieldAssignNode(type, origName, _field, _expr, get_linenum()));
        _this->setSSA(onSSA);
        dynamic_cast<arrayFieldAssignNode*>(_this.get())->setName(name);
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
        field->setSSA(t);
        expr->setSSA(t);
    }
private:
    std::string name;
    std::string origName;
    std::shared_ptr<expressionNode> field;
    std::shared_ptr<expressionNode> expr;
};

#endif //ANTLR_CPP_TUTORIAL_ARRAYFIELDASSIGNNODE_HPP
