//
// Created by CMG on 19/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP

//#include <nodes/basicblock.hpp>

class DFSNode;
public class DominatorTree {
public:

    std::shared_ptr<DFSNode> createDFSTree(){
        
    }
};


class DFSNode {

    //std::shared_ptr<basicblock> node;
    std::shared_ptr<DFSNode> parent;
    int dfnum;
};
#endif //ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP
