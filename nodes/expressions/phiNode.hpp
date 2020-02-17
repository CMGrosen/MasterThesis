//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PHINODE_HPP
#define ANTLR_CPP_TUTORIAL_PHINODE_HPP

class phiNode : public expressionNode {
    std::vector<std::shared_ptr<variableNode>> _variables;
public:
    phiNode(std::vector<std::shared_ptr<variableNode>> variables) : _variables{std::move(variables)} {
        setNodeType(Phi);
        setType(okType);
    }

    std::string to_string() override {
        std::string res = "$\\phi($";
        if (!_variables.empty()) {
            res += _variables[0]->to_string();
            for (auto i = 1; i < _variables.size(); ++i)
                res += (", " + _variables[i]->to_string());
        }
        res += ")";
        return res;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::vector<std::shared_ptr<variableNode>> _vars;
        for (auto v : _variables) {
            _vars.push_back(std::make_shared<variableNode>(variableNode(v->getType(), v->name)));
        }
        std::shared_ptr<expressionNode> _this = std::make_shared<phiNode>(phiNode(std::move(_vars)));
        _this->setNext(this->copy_next());
        return _this;
    }
};

#endif //ANTLR_CPP_TUTORIAL_PHINODE_HPP
