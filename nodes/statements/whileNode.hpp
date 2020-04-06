//
// Created by CMG on 23/10/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_WHILENODE_H
#define ANTLR_CPP_TUTORIAL_WHILENODE_H

class whileNode : virtual public statementNode {
public:
    whileNode(Type t, std::shared_ptr<expressionNode> c, std::shared_ptr<statementNode> b) : condition{std::move(c)}, body{std::move(b)} {
        setType(t);
        setNodeType(While);
    }
    expressionNode *getCondition() {return condition.get();}
    const std::shared_ptr<statementNode> getBody() const {return body;}

    std::string to_string() const override {
        return "while(" + condition->to_string() + ")";
    }

    std::string strOnSourceForm() const override {
        return "while(" + condition->strOnSourceForm() + ") { ...";
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<expressionNode> _c = condition->copy_expression();
        std::shared_ptr<statementNode> _b = body->copy_statement();
        std::shared_ptr<statementNode> _this = std::make_shared<whileNode>(whileNode(type, _c, _b));
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }

    void setSSA(bool t) override {
        onSSA = t;
        condition->setSSA(t);
        //body->setSSA(t);
    }
private:
    std::shared_ptr<expressionNode> condition;
    std::shared_ptr<statementNode> body;
};

#endif //ANTLR_CPP_TUTORIAL_WHILENODE_H
