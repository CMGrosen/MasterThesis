//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PHINODE_HPP
#define ANTLR_CPP_TUTORIAL_PHINODE_HPP

#include <src/nodes/statements/option.hpp>

class phiNode : public statementNode {
    std::string _name;
    std::string origName;
    std::vector<option> _variables;
public:

    phiNode(Type t, std::string name, const std::vector<std::string> *variables) :
    _name{name}, origName{std::move(name)} {
        for (const auto &v : *variables) {
            _variables.emplace_back(v, "0", "0");
        }
        setNodeType(Phi);
        set_linenum(-1);
        setType(t);
    }

    phiNode(Type t, std::string name, std::vector<std::pair<std::string, std::string>> variables) :
    _name{name}, origName{std::move(name)} {
        for (const auto &p : variables) _variables.emplace_back(p.first, p.second, p.second);
        setNodeType(Phi);
        setType(t);
        set_linenum(-1);
        onSSA = true;
    }

    phiNode(Type t, std::string name, std::string origname, std::vector<option> variables) :
            _name{std::move(name)}, origName{std::move(origname)}, _variables{std::move(variables)} {
        setNodeType(Phi);
        setType(t);
        set_linenum(-1);
        onSSA = true;
    }

    std::string to_string() const override {
        std::string res = nameToTikzName(_name, true) + " = $\\phi$(";
        if (!_variables.empty()) {
            res += nameToTikzName(_variables[0].var, true);
            for (size_t i = 1; i < _variables.size(); ++i)
                res += (", " + nameToTikzName(_variables[i].var, true));
        }
        res += ")";
        return res;
    }

    std::string strOnSourceForm() const override {
        return to_string();
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _this = std::make_shared<phiNode>(phiNode(type, _name, origName, _variables));
        _this->setSSA(onSSA);
        return _this;
    }

    std::string getName() const {
        return _name;
    }
    std::string getOriginalName() const {return origName;}
    void setName(std::string name) {
        _name = std::move(name);
    }
    void setSSA(bool t) override {
        onSSA = t;
    }
    std::vector<option> *get_variables() {return &_variables;}
    void update_variableindex(int index, const std::pair<std::string, std::string>& p) {_variables[index].var = p.first; _variables[index].var_boolname = p.second;}
    void set_variables(std::vector<option> names) {_variables = std::move(names);}
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
