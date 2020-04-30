//
// Created by CMG on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_IFELSENODE_H
#define ANTLR_CPP_TUTORIAL_IFELSENODE_H

class ifElseNode : public statementNode {
public:
    ifElseNode(Type t, std::shared_ptr<expressionNode> c, std::shared_ptr<statementNode> tb, std::shared_ptr<statementNode> fb, int linenum)
        : condition{std::move(c)}, trueBranch{std::move(tb)}, falseBranch{std::move(fb)} {
        setType(t);
        set_linenum(linenum);
        setNodeType(If);
    }
    ifElseNode(Type t, std::shared_ptr<expressionNode> c, std::shared_ptr<statementNode> tb, std::shared_ptr<statementNode> fb, int linenum, bool w)
            : ifElseNode(t, std::move(c), std::move(tb), std::move(fb), linenum) {
        isWhile = w;
    }
    expressionNode *getCondition() {return condition.get();}
    const std::shared_ptr<statementNode> getTrueBranch() const {return trueBranch;}
    const std::shared_ptr<statementNode> getFalseBranch() const {return falseBranch;}

    std::string to_string() const override {
        return "if(" + condition->to_string() + ")";
    }

    std::string strOnSourceForm() const override {
        std::string res = isWhile ? "while(" : "if(";
        return res + condition->strOnSourceForm() + ") { ...";
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _condition = condition->copy_expression();
        std::shared_ptr<statementNode> _trueBranch = trueBranch->copy_statement();
        std::shared_ptr<statementNode> _falseBranch = falseBranch->copy_statement();
        std::shared_ptr<ifElseNode> _this = std::make_shared<ifElseNode>(ifElseNode(type, _condition, _trueBranch, _falseBranch, get_linenum()));
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        _this->boolnamesForTrueBranch = boolnamesForTrueBranch;
        _this->boolnamesForFalseBranch = boolnamesForFalseBranch;

        return _this;
    }

    void setSSA(bool t) override {
        onSSA = t;
        condition->setSSA(t);
        /*trueBranch->setSSA(t);
        falseBranch->setSSA(t);*/
    }

    std::vector<std::string> boolnamesForTrueBranch;
    std::vector<std::string> boolnamesForFalseBranch;

private:
    bool isWhile = false;
    std::shared_ptr<expressionNode> condition;
    std::shared_ptr<statementNode> trueBranch;
    std::shared_ptr<statementNode> falseBranch;
};

#endif //ANTLR_CPP_TUTORIAL_IFELSENODE_H
