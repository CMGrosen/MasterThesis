//
// Created by CMG on 14/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#define ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#include <basicblockTreeConstructor.hpp>

class CCFGNode {
    static const int horizontal_padding = 10;
    static const int vertical_padding = 30;
    static constexpr float symbol_height = 6.667;
    static const int symbol_width = 6;

public:
    std::string name;
    std::string content;
    float node_width;
    float node_height;
    float right_width;
    float left_width;
    float distance;
    float v_distance;
    std::vector<std::shared_ptr<CCFGNode>> children;
    std::shared_ptr<CCFGNode> parent;
    std::shared_ptr<basicblock> basicblockInfo;

    CCFGNode(std::shared_ptr<CCFGNode> _parent, std::shared_ptr<basicblock> blk) : basicblockInfo{blk}, parent{std::move(_parent)}, name{blk->get_name()} {
        content = blk->to_string();
        node_width = blk->get_stmt_length() * symbol_width;
        node_height = blk->get_stmt_count() * symbol_height*2;
        left_width = node_width/2;
        right_width = node_width/2;
        distance = 0;
        v_distance = 0;
    }

    void add_child(const std::shared_ptr<CCFGNode> child){
        children.emplace_back(child);
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

    void resizeAll(std::set<std::shared_ptr<CCFGNode>> *resizedBlocks, std::set<std::shared_ptr<CCFGNode>> *placedBlocks){
        for(const auto& child : children){
            if (resizedBlocks->insert(child).second) {
                child->resizeAll(resizedBlocks, placedBlocks);
            } else {

            }
        }
        resize(placedBlocks);
    }
    void resize(std::set<std::shared_ptr<CCFGNode>> *placedBlocks) {
        std::pair<float, float> temp = calc_children_size(placedBlocks);
        if(temp.first > left_width) {
            left_width = temp.first;
        }
        if(temp.second > right_width) {
            right_width = temp.second;
        }
    }

    std::string to_string(const std::unordered_set<edge> *edges, const CCFG *ccfg){
        std::set<CCFGNode *> drawnBlocks;
        std::set<std::shared_ptr<CCFGNode>> resizedBlocks;
        std::set<std::shared_ptr<CCFGNode>> placeddBlocks;
        std::string result = std::string("\\usetikzlibrary{automata,positioning}\n") +
                             std::string("\\begin{tikzpicture}[shorten >=1pt, node distance=2cm, on grid, auto]\n");
        resizeAll(&resizedBlocks, &placeddBlocks);
        result += draw_node(&drawnBlocks, ccfg);
        result += draw_edges(edges);

        result += "\\end{tikzpicture}\n";
        return result;
    }

    std::string draw_node(std::set<CCFGNode *> *drawnBlocks, const CCFG *ccfg){
        //If this block has already been visited (while loop), then stop recursion
        if (!drawnBlocks->insert(this).second) return "";
        std::string result;
        result = "\\node[state] (" + name + ") [text width = " + std::to_string(node_width) + "pt, rectangle";
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
    std::string draw_edges(const std::unordered_set<edge> *edges){
        std::string result;
        for(const auto& ed : *edges){
            if(ed.type == conflict){
                result += "\\path[->, red] (";
            } else {
                result += "\\path[->] (" ;
            }
            result += ed.neighbours[0]->get_name() + ") edge (" + ed.neighbours[1]->get_name() + ");\n";
        }
        return result;
    }

private:

    std::pair<float, float> calc_children_size(std::set<std::shared_ptr<CCFGNode>> *placedBlocks){
        if(children.empty()){
            return{0,0};
        }
        int count = children.size();
        float _left_width = 0, _right_width = 0;

        float dist = 0, left_dist = 0, right_dist = 0;
        int left = 0, right = 0;

        if(count == 1){
            if(placedBlocks->insert(children.front()).second){
                children.front()->distance = 0;
                children.front()->v_distance =  node_height/2 + vertical_padding + children.front()->node_height/2;
                _left_width = children.front()->left_width;
                _right_width = children.front()->right_width;
            } else {
                _left_width = _right_width = 0;
            }
        } else {
            if(count % 2 == 1){
                left = count/2-1;
                int mid = count /2;
                right = count/2+1;
                dist = (children[mid]->left_width + children[mid]->right_width) / 2;
                children[mid]->distance = 0;
                children[mid]->v_distance = node_height/2 + vertical_padding + children[mid]->node_height/2;
                left_dist = dist + horizontal_padding;
                right_dist = dist + horizontal_padding;
            } else {
                left = count/2-1;
                right = count/2;
                left_dist = horizontal_padding/2;
                right_dist = horizontal_padding/2;
            }
            while(left >= 0){
                left_dist += children[left]->right_width;
                right_dist += children[right]->left_width;
                children[left]->distance = (left_dist)*-1;
                children[right]->distance = right_dist;
                children[left]->v_distance =  node_height/2 + vertical_padding + children[left]->node_height/2;
                children[right]->v_distance =  node_height/2 + vertical_padding + children[right]->node_height/2;
                left_dist += horizontal_padding + children[left]->left_width;
                right_dist += horizontal_padding + children[right]->right_width;
                left--;
                right++;
            }
            _left_width = left_dist;
            _right_width = right_dist;
        }
        return {_left_width, _right_width};
    }

};

class CCFGTree {

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

