//
// Created by CMG on 12/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
#define ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP

class sequentialNode : virtual public statementNode {
public:
    sequentialNode(std::shared_ptr<statementNode> body, std::shared_ptr<statementNode> next) : _body{std::move(body)}, _next{std::move(next)} {
        if (_body->getType() == okType && _next->getType() == okType) setType(okType);
        else setType(errorType);
        setNodeType(Sequential);
    };

    sequentialNode(Type t, std::shared_ptr<statementNode> body, std::shared_ptr<statementNode> next) : _body{std::move(body)}, _next{std::move(next)} {
        setType(t);
        setNodeType(Sequential);
    };

    std::shared_ptr<statementNode> getBody() const { return _body; }
    std::shared_ptr<statementNode> getNext() const { return _next; }

    /*std::vector<std::shared_ptr<statementNode>> debug_getAllNodes() override {
        std::vector<std::shared_ptr<statementNode>> nexts{_body};
        if(auto next = dynamic_cast<sequentialNode*>(_next.get())) {
             for (auto &s : next->debug_getAllNodes())
                 nexts.push_back(s);
        } else {
            nexts.push_back(_next);
        }
        return nexts;
    }*/
    std::string to_string() const override {
        return "";
    }
    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> __body = _body->copy_statement();
        std::shared_ptr<statementNode> __next = _next->copy_statement();
        std::shared_ptr<statementNode> _this = std::make_shared<sequentialNode>(sequentialNode(type, __body, __next));
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }
    void setSSA(bool t) override {
        onSSA = t;
    }

private:
    std::shared_ptr<statementNode> _body;
    std::shared_ptr<statementNode> _next;
};

#endif //ANTLR_CPP_TUTORIAL_SEQUENTIALNODE_HPP
