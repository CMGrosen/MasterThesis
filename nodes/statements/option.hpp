//
// Created by hu on 29/04/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_OPTION_HPP
#define ANTLR_CPP_TUTORIAL_OPTION_HPP

struct option {
    std::string var;
    std::string var_boolname;
    std::string block_boolname;

    option(std::string _var, std::string _var_boolname, std::string bname) :
            var{std::move(_var)}, var_boolname{std::move(_var_boolname)}, block_boolname{std::move(bname)} {

    }
};

#endif //ANTLR_CPP_TUTORIAL_OPTION_HPP
