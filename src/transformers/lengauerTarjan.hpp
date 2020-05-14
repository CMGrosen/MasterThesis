#ifndef ANTLR_CPP_TUTORIAL_LENGAUERTARJAN_HPP
#define ANTLR_CPP_TUTORIAL_LENGAUERTARJAN_HPP
#include <src/nodes/basicblock.hpp>
#include <src/transformers/basicblockTreeConstructor.hpp>
#include <utility>

struct DOMNode {
    std::weak_ptr<DOMNode> parent;
    std::vector<std::shared_ptr<DOMNode>> children;
    std::shared_ptr<basicblock> basic_block;
    DOMNode(std::shared_ptr<basicblock> _basic_block, const std::shared_ptr<DOMNode> &_parent) :
            parent{_parent},
            basic_block{std::move(_basic_block)}
    {}

    void AddChild(const std::shared_ptr<DOMNode>& child){
        for(const auto& _child : children){
            if(_child->basic_block == child->basic_block){
                return;
            }
        }
        children.emplace_back(child);
    }

};

class DomTree{
public:
    DomTree(const std::shared_ptr<CCFG>& ccfg){
        // instantiation of vectors and maps, used in Lengauer and Tarjans algorithm Dominators()
        int _i = 0;
        for(auto node : ccfg->nodes){
            idom.emplace_back(0);
            vertex.emplace_back(0);
            label.emplace_back(0);
            parent.emplace_back(0);
            ancestor.emplace_back(0);
            semi.emplace_back(0);
            bucket.insert({_i, std::set<int>()});
            pred.insert({_i, std::set<int>()});
            _i++;
        }
        idom.emplace_back(0);
        vertex.emplace_back(0);
        label.emplace_back(0);
        parent.emplace_back(0);
        ancestor.emplace_back(0);
        semi.emplace_back(0);
        bucket.insert({_i, std::set<int>()});
        pred.insert({_i, std::set<int>()});
        n = 0; v = 0;
        // finds immediate dominators for all nodes
        Dominators(ccfg); // Lengauer and Tarjan algorithm
        // creates the dominator tree
        CreateDomTree();
        // creates the dominance frontier
        CreateDominanceFrontier(nodes.find(0)->second); // Andrew Appel algorithm
        // prints immediate-dominators
        PrintIdom(); // for debugging purposes
    }

    std::unordered_map<std::shared_ptr<basicblock>, std::vector<std::shared_ptr<basicblock>>> DF; // Dominance Frontier
    std::shared_ptr<DOMNode> root;

private:
    std::unordered_map<std::shared_ptr<basicblock>, std::shared_ptr<DOMNode>> DOMTree;
    std::unordered_map<int, std::shared_ptr<basicblock>> nodes;
    std::unordered_map<std::shared_ptr<basicblock>, int> numbers;
    std::vector<int> idom;
    std::vector<int> vertex;
    std::vector<int> label;
    std::vector<int> parent;
    std::vector<int> ancestor;
    std::vector<int> semi;
    std::unordered_map<int, std::set<int>> bucket;
    std::unordered_map<int, std::set<int>> pred;
    int n, v;


    // Lengauer and Tarjan algorithm
    void Dominators(const std::shared_ptr<CCFG>& ccfg){
        CreateDFS(ccfg->startNode); // create Depth-First-Spanning-Tree
        // ensure that root is dominated by root
        label[0] = 0;
        semi[0] = 0;
        // finds the semi-dominators for all nodes except root and the first child og root
        // starts from the bottom and traverses upward
        for(int i = n; i > 1; i--){
            int w  = vertex[i];
            for(auto _v : pred.find(w)->second){
                int u = Eval(_v);
                if(semi[u] < semi[w]){
                    semi[w] = semi[u];
                }
            }
            bucket.find(vertex[semi[w]])->second.insert(w);
            Link(parent[w], w);
            auto children = bucket.find(parent[w])->second;
            if(!children.empty()){
                for(auto _v : children){
                    bucket.find(parent[w])->second.erase(_v);
                    int u = Eval(_v);
                    idom[_v] = semi[u] < semi[_v] ? u : parent[w];
                }
            }
        }
        // assigns the immediate-dominator for the node w
        for(int i = 2; i <= n; i++){
            int w = vertex[i];
            if(idom[w] != vertex[semi[w]]){
                idom[w] = idom[idom[w]];
            }
        }
        idom[0] = 0;
    }

    // Lengauer and Tarjan algorithm
    void CreateDFS(const std::shared_ptr<basicblock>& node){
        nodes.insert({v, node});
        numbers.insert({node, v});
        // v is the number associated with the node
        // n is the number associated the the child for which v is a proper ancestor
        // vertex[n] = v && semi[v] = n
        semi[v] = ++n; // starting value for semi[v]
        vertex[n] = v;
        label[v] = v;
        ancestor[v] = 0;
        v++;
        for(const auto &child : node->nexts){
            if(numbers.find(child) == numbers.end()){
                parent[v]  = numbers.find(node)->second;
                CreateDFS(child);
            }
            pred.find(numbers.find(child)->second)->second.insert(numbers.find(node)->second);
        }
    }

    // Lengauer and Tarjan algorithm
    void Compress(int _v){
        if(ancestor[ancestor[_v]]){
            Compress(ancestor[_v]);
            if(semi[label[ancestor[_v]]] < semi[label[_v]]){
                label[_v] = label[ancestor[_v]];
            }
            ancestor[_v] = ancestor[ancestor[_v]];
        }
    }

    // Lengauer and Tarjan algorithm
    int Eval(int _v){
        if(ancestor[_v] == 0){
            return _v;
        } else {
            Compress(_v);
            return label[_v];
        }
    }

    // Lengauer and Tarjan algorithm
    void Link(int _v, int _w){
        ancestor[_w] = _v;
    }

    void CreateDomTree (){
        std::shared_ptr<basicblock> basic_block = nodes.find(0)->second;
        std::shared_ptr<DOMNode> node = std::make_shared<DOMNode>(DOMNode(basic_block, nullptr));
        DOMTree.insert({basic_block, node});
        root = node;
        for(int i = 1; i < n; i++){
            basic_block = nodes.find(i)->second;
            auto idom_block = nodes.find(idom[numbers.find(basic_block)->second])->second;
            node = std::make_shared<DOMNode>(DOMNode(basic_block, DOMTree.find(idom_block)->second));
            node->parent.lock()->AddChild(node);
            DOMTree.insert({basic_block, node});
        }
    }

    // Andrew Appel algorithm
    void CreateDominanceFrontier(const std::shared_ptr<basicblock> &node){
        std::vector<std::shared_ptr<basicblock>> S = {};
        for(const auto& child : node->nexts){
            auto idom_block = nodes.find(idom[numbers.find(child)->second])->second;
            if(idom_block != node){
                S.emplace_back(child);
            }
        }
        for(const auto& child : DOMTree.find(node)->second->children){
            CreateDominanceFrontier(child->basic_block);
            for(const auto& _w : DF.find(child->basic_block)->second){
                std::shared_ptr<basicblock> w = _w;
                if (nodes.find(idom[numbers.find(w)->second])->second != node) {
                    S.emplace_back(w);
                }
            /*
                bool dominates = false;
                while (w && !dominates){
                    auto idom_block = nodes.find(idom[numbers.find(child->basic_block)->second])->second;
                    if(idom_block == n){
                        dominates = true;
                    }
                    w = idom_block;
                }
                if(!dominates){
                    S.emplace_back(w);
                }*/
            }
        }
        DF.insert({node, S});
    }

/*
    void CreateDominanceFrontier(const std::shared_ptr<CCFG>& ccfg){
        for(auto child : ccfg->nodes){
            DF.insert({child, std::vector<std::shared_ptr<basicblock>>()});
        }
        for(int i = 0; i < n; i++){
            std::shared_ptr<basicblock> node = nodes.find(i)->second;
            std::shared_ptr<basicblock> dom = nodes.find(idom[i])->second;
            DF.find(dom)->second.emplace_back(node);
        }
        //DF.insert({n, S});
    }*/

    void PrintIdom(){
        for(int i = 0; i < n; i++){
            std::cout << "idom" + std::to_string(i) + ": ";
            if(idom[i]) {
                for(const auto &stmt : nodes.find(idom[i])->second->statements) {
                    std::cout << stmt->to_string();
                }
            } else {
                std::cout << "none";
            }
            std::cout << " dominates: ";
            if(i){
                for(const auto& stmt : nodes.find(i)->second->statements){
                    std::cout << stmt->to_string();
                }
            } else {
                std::cout << "none";
            }

            std::cout << std::endl;
        }
    }
};

#endif //ANTLR_CPP_TUTORIAL_LENGAUERTARJAN_HPP
