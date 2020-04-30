//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PINODE_HPP
#define ANTLR_CPP_TUTORIAL_PINODE_HPP

#include <nodes/statements/option.hpp>

class piNode : public statementNode {
    std::string _variable;
    std::string name;
    int num;
    std::vector<option> _possible_variables;

public:
    piNode(Type t, std::string variable, int _num, const std::vector<std::pair<std::string, std::string>>& possible_variables)
        : _variable{std::move(variable)} {
        name = "-T_" + _variable + "_" + std::to_string(_num);
        num = _num;
        int i = 0;
        for (const auto &p : possible_variables) {
            _possible_variables.emplace_back(option(p.first, p.second, name + "b-" + std::to_string(++i)));
        }
        setType(t);
        set_linenum(-1);
        setNodeType(Pi);
        setSSA(true);
    }

    piNode(phiNode *phi) : _variable{phi->getOriginalName()}, name{phi->getName()} {
        std::string sub = name.substr(_variable.size()+1);
        num = std::stoi(sub);
        int i = 0;
        for (const auto &v : *phi->get_variables()) _possible_variables.emplace_back(v.var, v.var_boolname, name + "b-" + std::to_string(++i));
        setType(phi->getType());
        set_boolname(phi->get_boolname());
        set_linenum(-1);
        setNodeType(Pi);
        setSSA(true);
    }

    std::string to_string() const override {
        std::string res = nameToTikzName(name, true) + " = $\\pi$(";
        if (!_possible_variables.empty()) {
            res += nameToTikzName(_possible_variables[0].var, true);
            for (size_t i = 1; i < _possible_variables.size(); ++i)
                res += (", " + nameToTikzName(_possible_variables[i].var, true));
        }
        res += ")";
        return res;
    }

    std::string strOnSourceForm() const override {
        return to_string();
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::vector<std::pair<std::string, std::string>> _pos_vars;
        for (const auto &v : _possible_variables) {
            _pos_vars.emplace_back(v.var, v.var_boolname);
        }
        auto _this = std::make_shared<piNode>(piNode(type, _variable, num, std::move(_pos_vars)));
        _this->name = name;
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }

    std::vector<option> *get_variables() {return &_possible_variables;}
    void setVariables(std::vector<option> vars) {_possible_variables = std::move(vars);}
    bool contains(const std::string &var) {
        for (const auto &v : _possible_variables) {
            if (v.var == var) return true;
        }
        return false;
    }

    void updateVariablesAtIndex(int index, std::pair<std::string, std::string> p) {_possible_variables[index].var = p.first; _possible_variables[index].var_boolname = p.second;}
    void addVariable(std::pair<std::string, std::string> var) {_possible_variables.emplace_back(var.first, var.second, name + "b-" + std::to_string(_possible_variables.size()+1));}
    std::string getName() const {return name;}
    std::string getVar() const {return _variable;}
    void setSSA(bool t) override {
        onSSA = t;
    }
};
#endif //ANTLR_CPP_TUTORIAL_PINODE_HPP
