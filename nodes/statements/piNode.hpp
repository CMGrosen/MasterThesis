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
};
#endif //ANTLR_CPP_TUTORIAL_PINODE_HPP
