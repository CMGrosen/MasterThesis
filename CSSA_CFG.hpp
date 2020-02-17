//
// Created by hu on 17/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
#define ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP

#include <basicblockTreeConstructor.hpp>

struct CSSA_CFG {
    std::shared_ptr<CCFG> ccfg;

    CSSA_CFG(const CCFG &_ccfg) : ccfg{std::make_shared<CCFG>(CCFG(_ccfg))} {}

private:

};

#endif //ANTLR_CPP_TUTORIAL_CSSA_CFG_HPP
