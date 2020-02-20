//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PHINODE_HPP
#define ANTLR_CPP_TUTORIAL_PHINODE_HPP

class phiNode : public statementNode {
    std::string _name;
    std::vector<std::string> _variables;
public:
    phiNode(std::string name, std::vector<std::string> variables) :
    _name{std::move(name)}, _variables{std::move(variables)} {
        setNodeType(Phi);
        setType(okType);
    }

    std::string to_string() override {
        std::string res = "$\\phi($";
        if (!_variables.empty()) {
            res += _variables[0];
            for (auto i = 1; i < _variables.size(); ++i)
                res += (", " + _variables[i]);
        }
        res += ")";
        return res;
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _this = std::make_shared<phiNode>(phiNode(_name, _variables));
        return _this;
    }

    /*std::shared_ptr<expressionNode> copy_expression() const override {
        std::vector<std::shared_ptr<variableNode>> _vars;
        for (auto v : _variables) {
            _vars.push_back(std::make_shared<variableNode>(variableNode(v->getType(), v->name)));
        }
        std::shared_ptr<expressionNode> _this = std::make_shared<phiNode>(phiNode(std::move(_vars)));
        _this->setNext(this->copy_next());
        return _this;
    }

    bool operator==(const expressionNode *expr) const override {
        if (nodetype == expr->getNodeType()) {
            const auto vec = dynamic_cast<const phiNode *>(expr)->_variables;
            if (vec.size() != _variables.size()) return false;
            for (auto i = 0; i < _variables.size(); ++i) {
                if (!(_variables[i]->name == vec[i]->name)) return false;
            }
            return true;
        } else return false;
    }*/
};

#endif //ANTLR_CPP_TUTORIAL_PHINODE_HPP
