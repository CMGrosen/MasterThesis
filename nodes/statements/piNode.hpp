//
// Created by hu on 15/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_PINODE_HPP
#define ANTLR_CPP_TUTORIAL_PINODE_HPP

class piNode : public statementNode {
    std::string _variable;
    std::string name;
    int num;
    std::vector<std::string> _possible_variables;

public:
    piNode(std::string variable, int _num, std::vector<std::string> possible_variables)
        : _variable{std::move(variable)}, _possible_variables{std::move(possible_variables)}, num{_num} {
        name = "-T_" + _variable + "_" + std::to_string(num);
        setType(okType);
        setNodeType(Pi);
    }

    std::string to_string() override {
        std::string res = nameToTikzName(name, true) + " = $\\pi($";
        if (!_possible_variables.empty()) {
            res += nameToTikzName(_possible_variables[0], true);
            for (auto i = 1; i < _possible_variables.size(); ++i)
                res += (", " + nameToTikzName(_possible_variables[i], true));
        }
        res += ")";
        return res;
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::vector<std::string> _pos_vars;
        for (const auto &v : _possible_variables) {
            _pos_vars.push_back(v);
        }
        auto _this = std::make_shared<piNode>(piNode(_variable, num, std::move(_pos_vars)));
        _this->setSSA(onSSA);
        return _this;
    }

    std::vector<std::string> *get_variables() {return &_possible_variables;}

    void addVariable(std::string var) {_possible_variables.push_back(std::move(var));}
    std::string getName() const {return name;}
    std::string getVar() const {return _variable;}
    void setSSA(bool t) override {
        onSSA = t;
    }
};
#endif //ANTLR_CPP_TUTORIAL_PINODE_HPP
