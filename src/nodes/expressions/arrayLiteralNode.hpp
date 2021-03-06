//
// Created by hu on 13/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_ARRAYLITERALNODE_HPP
#define ANTLR_CPP_TUTORIAL_ARRAYLITERALNODE_HPP

class arrayLiteralNode : public expressionNode {
public:
    arrayLiteralNode(Type t, std::vector<std::shared_ptr<expressionNode>> a) : value{std::move(a)} {
        setType(t);
        setNodeType(ArrayLiteral);
    };
    arrayLiteralNode(std::vector<std::shared_ptr<expressionNode>> a) : value{std::move(a)} {
        bool wellTyped = true;
        Type t = value[0]->getType();
        if (t == intType || t == boolType) {
            for (const auto &e : value) {
                if (t != e->getType()) {
                    wellTyped = false;
                    break;
                }
            }
            if (wellTyped) {
                if (t == intType) type = arrayIntType;
                else type = arrayBoolType;
            } else type = errorType;
        } else type = errorType;
        setNodeType(ArrayLiteral);
    };

    std::string to_string() const override {
        std::string res = "[";
        for (const auto &v : value) {
            res += v->to_string() + ", ";
        }
        *(res.rbegin()+2) = ']';
        return res;
    }

    std::string strOnSourceForm() const override {
        std::string res = "[";
        for (const auto &v : value) {
            res += v->strOnSourceForm() + ", ";
        }
        *(res.rbegin()+2) = ']';
        res.resize(res.size()-1);
        return res;
    }

    std::shared_ptr<expressionNode> copy_expression() const override {
        std::vector<std::shared_ptr<expressionNode>> _values;
        _values.reserve(value.size());
        for(const auto &val : value) {
            _values.push_back(val->copy_expression());
        }
        std::shared_ptr<expressionNode> _this = std::make_shared<arrayLiteralNode>(arrayLiteralNode(getType(), std::move(_values)));
        _this->setSSA(onSSA);
        return _this;
    }


    bool operator==(const expressionNode *expr) const override {
        if (nodetype == expr->getNodeType()) {
            const std::vector<std::shared_ptr<expressionNode>> arrlit = dynamic_cast<const arrayLiteralNode*>(expr)->getArrLit();
            if (value.size() != arrlit.size()) {
                for (size_t i = 0; i < value.size(); ++i) {
                    if (value[i] != arrlit[i]) return false;
                }
                return true;
            } else return false;
        } else return false;
    }

    void setSSA(bool t) override {
        onSSA = t;
        for (const auto &ele : value) ele->setSSA(t);
    }

    std::vector<std::shared_ptr<expressionNode>> getArrLit() const {return value;};
private:
    std::vector<std::shared_ptr<expressionNode>> value;
};

#endif //ANTLR_CPP_TUTORIAL_ARRAYLITERALNODE_HPP
