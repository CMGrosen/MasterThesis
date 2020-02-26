//
// Created by CMG on 14/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#define ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#include <basicblockTreeConstructor.hpp>

class CCFGTree {
    class CCFGNode {
        static const int horizontal_padding = 10;
        static const int vertical_padding = 50;
        static constexpr float symbol_height = 6.667;
        static const int symbol_width = 6;

    public:
        std::string name;
        int distance;
        int v_distance;


        CCFGNode(std::shared_ptr<CCFGNode> _parent, std::string _name, std::string _content, int count) : parent{std::move(_parent)}, name{std::move(_name)}, content{std::move(_content)} {
            node_size = (count+1) * symbol_width;
            distance = 0;
            size = node_size;
        }

        CCFGNode(std::shared_ptr<CCFGNode> _parent, std::shared_ptr<basicblock> blk) : basicblockInfo{blk}, parent{std::move(_parent)}, name{blk->get_name()} {
            //addedBlocks->insert(blk.get());
            int longestStr = 0;
            std::string stmtStr;
            for (auto stmt : blk->statements) {
                stmtStr = stmt->to_string();
                const int stmtLength = stmtStr.length();
                int finalLength = stmtLength;
                if (stmt->isSSAForm()) {
                    if (stmt->getNodeType() == Phi) finalLength += (1 + dynamic_cast<phiNode*>(stmt.get())->get_variables()->size());
                    else if (stmt->getNodeType() == Pi) finalLength += (1 + dynamic_cast<piNode*>(stmt.get())->get_variables()->size());
                    for (int i = 0; i < stmtLength; ++i) {
                        if (stmtStr[i] == '$') {
                            --finalLength;
                            for (int j = i+1; j < stmtLength; ++j) {
                                if (stmtStr[j] == '$') {
                                    --finalLength;
                                    i = j;
                                    break;
                                }
                                --finalLength;
                            }
                        }
                    }
                }
                if (longestStr < finalLength) longestStr = finalLength;
                content += (stmtStr + "\\\\");
            }
            v_padding = blk->statements.size()* symbol_height + vertical_padding;
            node_size = (longestStr+1) * symbol_width;
            distance = 0;
            v_distance = vertical_padding;
            size = node_size;
        }


        void construct_graph(std::shared_ptr<CCFGNode> parent, std::set<basicblock *> *addedBlocks, std::unordered_map<std::shared_ptr<basicblock>,std::shared_ptr<CCFGNode>> *nodesCreated) {
            for (auto child : basicblockInfo->nexts) {
                if(nodesCreated->find(child) == nodesCreated->end()) {
                    auto _child = std::make_shared<CCFGNode>(CCFGNode(parent, child));
                    add_child(_child);
                    nodesCreated->insert({child, _child});
                } else {
                    add_child(nodesCreated->find(child)->second);
                }
            }
            for (auto child : children) {
                //If child has already been visited (while loop), then don't add again, thus stopping recursion
                if (addedBlocks->insert(child->basicblockInfo.get()).second)
                    child->construct_graph(child,addedBlocks, nodesCreated);
            }
        }

        std::string to_string(const std::unordered_set<edge> *edges, const CCFG *ccfg){
            std::set<CCFGNode *> drawnBlocks;
            std::set<std::shared_ptr<CCFGNode>> resizedBlocks;
            std::string result = std::string("\\usetikzlibrary{automata,positioning}\n") +
                                 std::string("\\begin{tikzpicture}[shorten >=1pt, node distance=2cm, on grid, auto]\n");
            resizeAll(&resizedBlocks);
            result += draw_node(&drawnBlocks, ccfg);

            /*
            result += draw_edges();
            for(const auto& child : children){
                result += child->draw_edges();
            }
            */

            for(const auto& ed : *edges){
                if(ed.type == conflict){
                    result += "\\path[->, red] (";
                } else {
                    result += "\\path[->] (" ;
                }
                result += ed.neighbours[0]->get_name() + ") edge (" + ed.neighbours[1]->get_name() + ");\n";
            }

            result += "\\end{tikzpicture}\n";
            return result;
        }

        std::string draw_node(std::set<CCFGNode *> *drawnBlocks, const CCFG *ccfg){
            //If this block has already been visited (while loop), then stop recursion
            if (!drawnBlocks->insert(this).second) return "";
            std::string result;
            result = "\\node[state] (" + name + ") [text width = " + std::to_string(node_size) + "pt, rectangle";
            if(parent){
                if(distance == 0){
                    result += ", below = " + std::to_string(v_distance) + "pt of ";
                } else if(distance < 0){
                    result+= ", below left = " + std::to_string(v_distance) + "pt and " + std::to_string(distance*-1) + "pt of ";
                } else {
                    result+= ", below right = " + std::to_string(v_distance) + "pt and " + std::to_string(distance) + "pt of ";
                }
                result +=  parent->name;
            }
            result += "] { \\texttt{" + content +"}};\n";

            for(const auto& child : children){
                result += child->draw_node(drawnBlocks, ccfg);
            }
            return result;
        }

        std::string draw_edges(){
            std::string result;
            for(const auto& child : children){
                result += "\\path[->] (" + name + ") edge (" + child->name + ");\n";
            }
            return result;
        }

        void add_child(const std::shared_ptr<CCFGNode> child){
            children.emplace_back(child);
        }

        void resizeAll(std::set<std::shared_ptr<CCFGNode>> *resizedBlocks){
            for(const auto& child : children){
                if (resizedBlocks->insert(child).second) {
                    child->resizeAll(resizedBlocks);
                } else {
                    std::cout << "";
                }
            }
            resize();
        }
        void resize(){
            int temp = calc_children_size();
            if(temp > node_size){
                size = temp;
            } else {
                size = node_size;
            }
        }
        int get_node_size(){
            return node_size;
        }
        int get_size(){
            return size;
        }
        std::string get_content(){
            return content;
        }
        std::vector<std::shared_ptr<CCFGNode>> children;
        std::shared_ptr<CCFGNode> parent;
        std::shared_ptr<basicblock> basicblockInfo;

    private:
        std::string content;
        int  v_padding;
        int node_size;
        int size;

        int calc_children_size(){
            int result;
            if(children.empty()){
                return 0;
            }
            int count = children.size();

            if(count == 1){
                children.front()->distance = 0;
                children.front()->v_distance =  v_padding;
                return children.front()->get_size();
            }
            int dist;
            int left, right;
            if(count % 2 == 1){
                left = count/2-1;
                int mid = count /2;
                right = count/2+1;
                dist = children[mid]->get_size()/2+horizontal_padding;
                children[mid]->distance = 0;
                children[mid]->v_distance =  v_padding;
            } else {
                left = count/2-1;
                right = count/2;
                dist = 0;
            }
            while(left >= 0){
                dist += children[left]->get_size()<children[right]->get_size() ?
                        children[right]->get_size():
                        children[left]->get_size();
                children[left]->distance = (dist/2 + horizontal_padding)*-1;
                children[right]->distance = dist/2 + horizontal_padding;
                children[left]->v_distance =  v_padding;
                children[right]->v_distance =  v_padding;
                dist += horizontal_padding;
                left--;
                right++;
            }
            result = children.front()->get_size()/2 + children.back()->get_size()/2 + dist;
            return result;
        }

    };

    const CCFG _ccfg;
    std::shared_ptr<CCFGNode> root;
    std::string printed_tree = "";
    std::unordered_map<std::shared_ptr<basicblock>,std::shared_ptr<CCFGNode>> creatednodes;

public:
    CCFGTree(CCFG ccfg) : _ccfg{std::move(ccfg)} {
        root = std::make_shared<CCFGNode>(CCFGNode(nullptr, _ccfg.startNode));
        std::set<basicblock *> addedBlocks = std::set<basicblock *>{_ccfg.startNode.get()};
        root->construct_graph(root, &addedBlocks, &creatednodes);
    }

    ~CCFGTree() {
        for (auto it : creatednodes) {
            it.second->children.clear();
            it.second->basicblockInfo = nullptr;
            it.second->parent = nullptr;
        }
        root = nullptr;
    }

    std::string DrawCCFG() {
        if (printed_tree.empty()) {
            printed_tree = std::move(root->to_string(&_ccfg.edges, &_ccfg));
        }
        return printed_tree;
    }
};

#endif //ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP

