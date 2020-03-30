//
// Created by hu on 04/03/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_UNPACKEDSTMT_HPP
#define ANTLR_CPP_TUTORIAL_UNPACKEDSTMT_HPP

struct unpackedstmt : virtual public statementNode {
    std::shared_ptr<unpacked> _this;
    unpackedstmt(std::shared_ptr<unpacked> _this) : _this{std::move(_this)} {}

    std::string to_string() const override {
        return _this->to_string();
    }

    std::shared_ptr<statementNode> copy_statement() const override {
        std::shared_ptr<statementNode> _new = std::make_shared<unpackedstmt>(unpackedstmt(_this->copy()));
        _new->set_boolname(get_boolname());
        return _new;
    }
};

#endif //ANTLR_CPP_TUTORIAL_UNPACKEDSTMT_HPP
