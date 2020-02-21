//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PINODE_HPP
#define ANTLR_CPP_TUTORIAL_PINODE_HPP

class piNode : public statementNode {
    std::shared_ptr<variableNode> _variable;
    std::vector<std::shared_ptr<variableNode>> _possible_variables;

public:
    piNode(std::shared_ptr<variableNode> variable, std::vector<std::shared_ptr<variableNode>> possible_variables)
        : _variable{std::move(variable)}, _possible_variables{std::move(possible_variables)} {
        setType(okType);
        setNodeType(Pi);
    }

    std::string to_string() override {
        std::string res = _variable->to_string() + " = $\\pi($";
        if (!_possible_variables.empty()) {
            res += _possible_variables[0]->to_string();
            for (auto i = 1; i < _possible_variables.size(); ++i)
                res += (", " + _possible_variables[i]->to_string());
        }
        res += ")";
        return res;
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::vector<std::shared_ptr<variableNode>> _pos_vars;
        for (auto v : _possible_variables) {
            _pos_vars.push_back(std::make_shared<variableNode>(variableNode(v->getType(), v->name)));
        }
        auto _this = std::make_shared<piNode>(piNode(std::make_shared<variableNode>(variableNode(_variable->getType(), _variable->name)), std::move(_pos_vars)));
        _this->setSSA(onSSA);
        return _this;
    }
};
#endif //ANTLR_CPP_TUTORIAL_PINODE_HPP
