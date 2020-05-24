//
// Created by hu on 15/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_EVENTNODE_HPP

class eventNode : public statementNode {
public:
    eventNode(Type t, std::shared_ptr<expressionNode> condition, int linenum) : _condition{std::move(condition)} {
        setType(t);
        set_linenum(linenum);
        setNodeType(Event);
    }
    expressionNode *getCondition() {return _condition.get();}
    void setCondition(std::shared_ptr<expressionNode>e) {_condition = std::move(e);}

    std::string to_string() const override {
        return "event(" + _condition->to_string() + ")";
    }
    std::string strOnSourceForm() const override {
        return "when(" + _condition->strOnSourceForm() + ");";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _expr = _condition->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<eventNode>(eventNode(type, _expr, get_linenum()));
        _this->setSSA(onSSA);
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
        _condition->setSSA(t);
    }
    bool replacePiWithLit(const std::string &piname, Type t, std::string val) override {
        if (_condition->getNodeType() == Variable && reinterpret_cast<variableNode*>(_condition.get())->name == piname) {
            _condition = std::make_shared<literalNode>(literalNode(t, val));
            return true;
        } else {
            return _condition->replacePiWithLit(piname, t, val);
        }
    }
private:
    std::shared_ptr<expressionNode> _condition;
};

#endif //ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
