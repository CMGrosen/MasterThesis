#ifndef ANTLR_CPP_TUTORIAL_ASSERTNODE_HPP
#define ANTLR_CPP_TUTORIAL_ASSERTNODE_HPP

class assertNode : virtual public statementNode{
public:
    assertNode(Type t, std::shared_ptr<expressionNode> condition) : _condition{std::move(condition)} {
        setType(t);
        setNodeType(Assert);
    }
    expressionNode *getCondition() {return _condition.get();}
    std::string to_string() const override {
        return "assert(" + _condition->to_string() + ")";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _expr = _condition->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<assertNode>(assertNode(type, _expr));
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
        _condition->setSSA(t);
    }
private:
    std::shared_ptr<expressionNode> _condition;
};

#endif //ANTLR_CPP_TUTORIAL_ASSERTNODE_HPP
