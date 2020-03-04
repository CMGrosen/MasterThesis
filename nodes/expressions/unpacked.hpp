//
// Created by hu on 04/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_UNPACKED_HPP
#define ANTLR_CPP_TUTORIAL_UNPACKED_HPP

struct unpacked : public node {
    unpacked(Type t, NodeType nt) {
        type = t;
        nodetype = nt;
        _operator = NOTUSED;
    }
    unpacked(Type t, NodeType nt, std::string _value) : value{std::move(_value)} {
        type = t;
        nodetype = nt;
        _operator = NOTUSED;
    }
    unpacked(Type t, NodeType nt, op _operator) : _operator{_operator} {
        type = t;
        nodetype = nt;
    }

    unpacked *get_last() {
        if (next) return next->get_last();
        else return this;
    }

    std::shared_ptr<unpacked> copy() {
        std::shared_ptr<unpacked> _this = std::make_shared<unpacked>(unpacked(type, nodetype, value));
        _this->_operator = _operator;
        _this->next = next ? next->copy() : nullptr;
        return _this;
    }
    std::string to_string() {
        std::string res;
        switch (nodetype) {
            case Assign:
                res = "assign(" + nameToTikzName(value, true) + ")";
                break;
            case AssignArrField:
                res = "fieldassign(" + nameToTikzName(value, true) + ")";
                break;
            case If: res = "if";
                break;
            case Write: res = "write";
                break;
            case Read: res = "read(" + value + ")";
                break;
            case ArrayAccess: res = nameToTikzName(value, true) + "[]"; //nametotikz
                break;
            case ArrayLiteral: res = "arrlit(" + value + ")";
                break;
            case Event: res = "event";
                break;
            case Variable:
                res = nameToTikzName(value, true);
                break;
            case Literal:
                res = value;
                break;
            case BinaryExpression:
            case UnaryExpression:
                res = operatorToString[_operator];
                break;
            case Skip:
                res = "skip";
                break;
            default: res = "";
        }
        return next ? res + " " + next->to_string() : res;
    }
    std::shared_ptr<unpacked> next;
    std::string value;
    op _operator;
};

#endif //ANTLR_CPP_TUTORIAL_UNPACKED_HPP
