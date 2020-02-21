//
// Created by CMG on 19/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP
#define ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP

#include <nodes/basicblock.hpp>
#include <basicblockTreeConstructor.hpp>
#include <utility>

class DominatorTree {
    struct DFSTree {

        std::unordered_map<int, std::shared_ptr<basicblock>> nodes;
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> parents;
        std::unordered_map<std::shared_ptr<basicblock>, int> dfnums;
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> ancestors;

    };
public:
    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<basicblock>>> DF;    // Dominator Frontier
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
        if( dfsTree->dfnums.find(_node) == dfsTree->dfnums.end() ){
            dfsTree->parents.insert({_node, _parent});
            dfsTree->nodes.insert({*num, _node});
            dfsTree->dfnums.insert({_node, *num});
            *num += 1;
            dfsTree->ancestors.insert({_node, _parent});
            for(const auto &child : _node->nexts){
                CreateDFSTreeHelper(child, _node, num, dfsTree);
            }
        } else {
            //dfsTree->ancestors.find(_node)->second.emplace_back(_parent);
        }
    }

    static void Link(const std::shared_ptr<basicblock>& _node, std::shared_ptr<basicblock> _parent, DFSTree* dfsTree){
        dfsTree->ancestors.find(_node)->second = std::move(_parent);
    }

    static std::shared_ptr<basicblock> AncestorWithLowestSemi(std::shared_ptr<basicblock> v, DFSTree* dfsTree, std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>>* semi){
        auto u = v;
        while(semi->find(dfsTree->ancestors.find(v)->second)->second){
            auto v1 = semi->find(v)->second;
            int v_dfnum = dfsTree->dfnums.find(v1)->second;
            auto u1 = semi->find(u)->second;
            int u_dfnum = dfsTree->dfnums.find(u1)->second;
            if(v_dfnum < u_dfnum){
                u = v;
            }
            v = dfsTree->ancestors.find(v)->second;
        }
        return u; //ikke lovligt. Memory der refereres til er stack. n som bliver passed har anden værdi end v som ellers er n
    }

    static std::shared_ptr<basicblock>* AncestorWithLowerDFNum(std::shared_ptr<basicblock>* v, DFSTree* dfsTree, int s_dfnum){
        auto u = v;
        while(dfsTree->ancestors.find(*v) != dfsTree->ancestors.end()){
            int v_dfnum = dfsTree->dfnums.find(*v)->second;
            if(v_dfnum < s_dfnum){
                return v;
            }
            v = &dfsTree->ancestors.find(*v)->second;
        }
        return &dfsTree->nodes.find(s_dfnum)->second;
    }


    DominatorTree(const std::shared_ptr<CCFG>& ccfg){
        std::unordered_map<std::shared_ptr<basicblock>,std::vector<std::shared_ptr<basicblock>>> bucket;
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> semi;
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> idom;
        std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<basicblock>> samedom;
        DFSTree depth_first_spanning_tree = CreateDFSTree(ccfg);
        for(auto n : depth_first_spanning_tree.nodes){
            bucket.insert({n.second, std::vector<std::shared_ptr<basicblock>>()});
            semi.insert({n.second, nullptr});
            idom.insert({n.second, nullptr});
            samedom.insert({n.second, nullptr});
        }
        for( int i = depth_first_spanning_tree.nodes.size()-1; i > 0;i-- ){
            std::shared_ptr<basicblock> n = depth_first_spanning_tree.nodes.find(i)->second;
            std::shared_ptr<basicblock> p = depth_first_spanning_tree.parents.find(n)->second;
            std::shared_ptr<basicblock> s = p;
            for(const auto& _v : n->parents){
                std::shared_ptr<basicblock> v = _v.lock();
                std::shared_ptr<basicblock> _s;
                if(depth_first_spanning_tree.dfnums.find(v)->second <= depth_first_spanning_tree.dfnums.find(n)->second){
                    _s = v;
                } else {
                    _s = (semi.find(AncestorWithLowestSemi(n, &depth_first_spanning_tree, &semi))->second);
                }
                if(depth_first_spanning_tree.dfnums.find(_s)->second <= depth_first_spanning_tree.dfnums.find(s)->second){
                    s = _s;
                }
            }
            semi.find(n)->second = s;
            bucket.find(s)->second.emplace_back(n);
            Link(n, p, &depth_first_spanning_tree);

            for(std::shared_ptr<basicblock> v : bucket.find(p)->second){
                auto y = AncestorWithLowestSemi(v, &depth_first_spanning_tree, &semi);
                if(semi.find(y)->second == semi.find(v)->second){
                    idom.insert({v, p});
                } else {
                    samedom.insert({v, y});
                }
            }
            bucket.find(p)->second.clear();
        }
        for(int i = 1; i < depth_first_spanning_tree.nodes.size(); i++){
            std::shared_ptr<basicblock> n = depth_first_spanning_tree.nodes.find(i)->second;
            if(samedom.find(n)->second){
                idom.find(n)->second = idom.find(samedom.find(n)->second)->second;
            }
        }
        std::cout << "Depth-First-Spanning-Tree constructed" << std::endl;
    }
};




#endif //ANTLR_CPP_TUTORIAL_DOMINATORTREECONSTRUCTOR_HPP
