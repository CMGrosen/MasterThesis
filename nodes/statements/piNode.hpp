//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PINODE_HPP
#define ANTLR_CPP_TUTORIAL_PINODE_HPP

class piNode : virtual public statementNode {
    std::string _variable;
    std::string name;
    int num;
    std::vector<std::pair<std::string, std::string>> _possible_variables;

public:
    piNode(Type t, std::string variable, int _num, std::vector<std::pair<std::string, std::string>> possible_variables)
        : _variable{std::move(variable)}, _possible_variables{std::move(possible_variables)} {
        name = "-T_" + _variable + "_" + std::to_string(_num);
        num = _num;
        setType(t);
        setNodeType(Pi);
        setSSA(true);
    }

    piNode(phiNode *phi) : _variable{phi->getOriginalName()}, name{phi->getName()} {
        std::string sub = name.substr(_variable.size()+1);
        num = std::stoi(sub);
        for (const auto &v : *phi->get_variables()) _possible_variables.emplace_back(v);
        setType(phi->getType());
        set_boolname(phi->get_boolname());
        setNodeType(Pi);
        setSSA(true);
    }

    std::string to_string() const override {
        std::string res = nameToTikzName(name, true) + " = $\\pi($";
        if (!_possible_variables.empty()) {
            res += nameToTikzName(_possible_variables[0].first, true);
            for (size_t i = 1; i < _possible_variables.size(); ++i)
                res += (", " + nameToTikzName(_possible_variables[i].first, true));
        }
        res += ")";
        return res;
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::vector<std::pair<std::string, std::string>> _pos_vars;
        for (const auto &v : _possible_variables) {
            _pos_vars.push_back(v);
        }
        auto _this = std::make_shared<piNode>(piNode(type, _variable, num, std::move(_pos_vars)));
        _this->name = name;
        _this->setSSA(onSSA);
        _this->set_boolname(get_boolname());
        return _this;
    }

    std::vector<std::pair<std::string, std::string>> *get_variables() {return &_possible_variables;}
    void setVariables(std::vector<std::pair<std::string, std::string>> vars) {_possible_variables = std::move(vars);}
    bool contains(const std::string &var) {
        for (const auto &v : _possible_variables) {
            if (v.first == var) return true;
        }
        return false;
    }

    void updateVariablesAtIndex(int index, std::pair<std::string, std::string> p) {_possible_variables[index] = p;}
    void addVariable(std::pair<std::string, std::string> var) {_possible_variables.push_back(std::move(var));}
    std::string getName() const {return name;}
    std::string getVar() const {return _variable;}
    void setSSA(bool t) override {
        onSSA = t;
    }
};
#endif //ANTLR_CPP_TUTORIAL_PINODE_HPP
