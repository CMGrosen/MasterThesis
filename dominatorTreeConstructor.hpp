//
// Created by CMG on 19/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP

#include <nodes/basicblock.hpp>
#include <basicblockTreeConstructor.hpp>

class DominatorTree {
    struct DFSTree {

        std::unordered_map<int, std::shared_ptr<basicblock>> nodes;
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> parents;
        std::unordered_map<std::shared_ptr<basicblock>, int> dfnums;
        std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<basicblock>>> ancestors;

    };
public:
    std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> DF;    // Dominator Frontier
    static DFSTree CreateDFSTree(const std::shared_ptr<CCFG> &ccfg){
        DFSTree dfsTree;
        std::shared_ptr<basicblock> node = ccfg->startNode;
        int num = 0;
        dfsTree.nodes.insert({num, node});
        dfsTree.parents.insert({node, nullptr});
        dfsTree.dfnums.insert({node, num});
        num += 1;
        for(const auto &child : node->nexts){
            CreateDFSTreeHelper(child, node, &num, &dfsTree);
        }
        return dfsTree;
    }

    static void CreateDFSTreeHelper(const std::shared_ptr<basicblock>& _node, const std::shared_ptr<basicblock>& _parent, int* num, DFSTree* dfsTree){
        if( dfsTree->parents.find(_node) == dfsTree->parents.end() ){
            dfsTree->parents.insert({_node, _parent});
            dfsTree->nodes.insert({*num, _node});
            dfsTree->dfnums.insert({_node, *num});
            *num += 1;
            dfsTree->ancestors.insert({_node, std::vector<std::shared_ptr<basicblock>>{_parent}});
            for(const auto &child : _node->nexts){
                CreateDFSTreeHelper(child, _node, num, dfsTree);
            }
        } else {
            dfsTree->ancestors.find(_node)->second.emplace_back(_parent);
        }
    }

    void Link(const std::shared_ptr<basicblock>& _node, const std::shared_ptr<basicblock>& _parent){
        
    }

    std::shared_ptr<basicblock> AncestorWithLowestSemi(std::shared_ptr<basicblock> v, DFSTree* dfsTree){
        auto temp_a = dfsTree->parents.find(v);
        if(temp_a == dfsTree->parents.end()){
            std::shared_ptr<basicblock> a = temp_a->second;
            std::shared_ptr<basicblock> b = AncestorWithLowestSemi(a, dfsTree);



            dfsTree->parents.find(v)->second = dfsTree->parents.find(a)->second;
        }
    }


    DominatorTree(const std::shared_ptr<CCFG>& ccfg){
        DFSTree depth_first_spanning_tree = CreateDFSTree(ccfg);
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> semi;
        std::unordered_map<std::shared_ptr<basicblock>,std::vector<std::shared_ptr<basicblock>>> bucket;
        for( int i = depth_first_spanning_tree.nodes.size(); i > 0;i-- ){
            std::shared_ptr<basicblock> n = depth_first_spanning_tree.nodes.find(i)->second;
            std::shared_ptr<basicblock> p = depth_first_spanning_tree.parents.find(n)->second;
            std::shared_ptr<basicblock>* s = &p;
            for(auto v : depth_first_spanning_tree.ancestors.find(n)->second){
                std::shared_ptr<basicblock>* _s;
                if(depth_first_spanning_tree.dfnums.find(v)->second <= depth_first_spanning_tree.dfnums.find(n)->second){
                    _s = &v;
                } else {
                    _s = &n;
                }
                if(depth_first_spanning_tree.dfnums.find(*_s)->second <= depth_first_spanning_tree.dfnums.find(*s)->second){
                    s = _s;
                }
            }
            semi.insert({n, *s});
            auto res = bucket.insert({*s, std::vector<std::shared_ptr<basicblock>>{n}});
            if(!(res.second)){
                res.first->second.emplace_back(n);
            }

        }
        std::cout << "Depth-First-Spanning-Tree constructed" << std::endl;
    }
};




#endif //ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP
