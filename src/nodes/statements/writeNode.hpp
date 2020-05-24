//
// Created by hu on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_WRITENODE_HPP
#define ANTLR_CPP_TUTORIAL_WRITENODE_HPP

class writeNode : public statementNode {
public:
    writeNode(int16_t pin, std::shared_ptr<expressionNode> e, int linenum) : _e{std::move(e)}, _pin{pin} {
        type = (_e->getType() == intType) ? okType : errorType;
        set_linenum(linenum);
        setNodeType(Write);
    }
    writeNode(Type t, int16_t pin, std::shared_ptr<expressionNode> e, int linenum) : _e{std::move(e)}, _pin{pin} {
        setType(t);
        set_linenum(linenum);
        setNodeType(Write);
    }

    int16_t getPin() const {return _pin;};
    expressionNode *getExpr() const {return _e.get();};
    void setExpr(std::shared_ptr<expressionNode> e) {_e = std::move(e);};

    std::string to_string() const override {
        return "write(" + std::to_string(_pin) + ", " + _e->to_string() + ")";
    }

    std::string strOnSourceForm() const override {
        return "write(" + std::to_string(_pin) + ", " + _e->strOnSourceForm() + ");";
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _expr = _e->copy_expression();
        std::shared_ptr<statementNode> _this = std::make_shared<writeNode>(writeNode(type, _pin, _expr, get_linenum()));
        _this->setSSA(onSSA);
        return _this;
    }

    bool replacePiWithLit(const std::string &piname, Type t, std::string val) override {
        if (_e->getNodeType() == Variable && reinterpret_cast<variableNode*>(_e.get())->name == piname) {
            _e = std::make_shared<literalNode>(literalNode(t, val));
            return true;
        } else {
           return _e->replacePiWithLit(piname, t, val);
        }
    }

    void setSSA(bool t) override {
        onSSA = t;
        _e->setSSA(t);
    }
private:
    std::shared_ptr <expressionNode> _e;
    const int16_t _pin;
};

#endif //ANTLR_CPP_TUTORIAL_WRITENODE_HPP
