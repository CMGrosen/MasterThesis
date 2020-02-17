//
// Created by CMG on 14/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#define ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#include <basicblockTreeConstructor.hpp>

class CCFGNode {
    static const int padding = 10;
    static const int vertical_padding = 50;
public:
    std::string name;
    int distance;

    CCFGNode(std::shared_ptr<CCFGNode> _parent, std::string _name, std::string _content, int count) : parent{std::move(_parent)}, name{std::move(_name)}, content{std::move(_content)} {
        node_size = (count * 6) + 6;
        distance = 0;
        size = node_size;
    }

    CCFGNode(std::shared_ptr<CCFGNode> _parent, std::shared_ptr<basicblock> blk) : basicblockInfo{blk}, parent{std::move(_parent)}, name{blk->get_name()} {
        //addedBlocks->insert(blk.get());
        int longestStr = 0;
        std::string stmtStr;
        for (auto stmt : blk->statements) {
            stmtStr = stmt->to_string();
            if (longestStr < stmtStr.length()) longestStr = stmtStr.length();
            content += (stmtStr + "\\\\");
        }
        node_size = (longestStr * 6) + 6;
        distance = 0;
        size = node_size;
        /*
        for (auto child : blk->nexts) {
            if (addedBlocks->insert(child.get()).second) {
                add_child(std::make_shared<CCFGNode>(CCFGNode(std::shared_ptr<CCFGNode>(this), child, addedBlocks)));
            }
        }*/
    }

    void construct_graph(std::shared_ptr<CCFGNode> parent, std::set<basicblock *> *addedBlocks) {
        for (auto child : basicblockInfo->nexts) {
            add_child(std::make_shared<CCFGNode>(CCFGNode(parent, child)));
        }
        for (auto child : children) {
            //If child has already been visited (while loop), then don't add again, thus stopping recursion
            if (addedBlocks->insert(child->basicblockInfo.get()).second)
                child->construct_graph(child,addedBlocks);
        }
    }

    std::string to_string(const std::unordered_set<edge> *edges){
        std::set<CCFGNode *> drawnBlocks;
        std::set<CCFGNode *> resizedBlocks;
        std::string result = std::string("\\usetikzlibrary{automata,positioning}\n") +
                std::string("\\begin{tikzpicture}[shorten >=1pt, node distance=2cm, on grid, auto]\n");
        resizeAll(&resizedBlocks);
        result += draw_node(&drawnBlocks);

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

    std::string draw_node(std::set<CCFGNode *> *drawnBlocks){
        //If this block has already been visited (while loop), then stop recursion
        if (!drawnBlocks->insert(this).second) return "";
        std::string result;
        result = "\\node[state] (" + name + ") [text width = " + std::to_string(node_size) + "pt, rectangle";
        if(parent){
            if(distance == 0){
                result += ", below = " + std::to_string(vertical_padding) + "pt of ";
            } else if(distance < 0){
                result+= ", below left = " + std::to_string(vertical_padding) + "pt and " + std::to_string(distance*-1) + "pt of ";
            } else {
                result+= ", below right = " + std::to_string(vertical_padding) + "pt and " + std::to_string(distance) + "pt of ";
            }
            result +=  parent->name;
        }
        result += "] { \\texttt{" + content +"}};\n";

        for(const auto& child : children){
            result += child->draw_node(drawnBlocks);
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

    void add_child(const std::shared_ptr<CCFGNode>& child){
        children.emplace_back(child);
    }

    void resizeAll(std::set<CCFGNode *> *resizedBlocks){
        for(const auto& child : children){
            if (resizedBlocks->insert(this).second) {
                child->resizeAll(resizedBlocks);
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
private:
    std::string content;
    int node_size;
    int size;
    std::vector<std::shared_ptr<CCFGNode>> children;
    std::shared_ptr<CCFGNode> parent;
    std::shared_ptr<basicblock> basicblockInfo;

    int calc_children_size(){
        int result;
        if(children.empty()){
            return 0;
        }
        int count = children.size();

        if(count == 1){
            children.front()->distance = 0;
            return children.front()->get_size();
        }
        int dist;
        int left, right;
        if(count % 2 == 1){
            left = count/2-1;
            int mid = count /2;
            right = count/2+1;
            dist = children[mid]->get_size()/2+padding;
            children[mid]->distance = 0;
        } else {
            left = count/2-1;
            right = count/2;
            dist = 0;
        }
        while(left >= 0){
            dist += children[left]->get_size()<children[right]->get_size() ?
                    children[right]->get_size():
                    children[left]->get_size();
            children[left]->distance = (dist/2 + padding)*-1;
            children[right]->distance = dist/2 + padding;
            dist += padding;
            left--;
            right++;
        }
        result = children.front()->get_size()/2 + children.back()->get_size()/2 + dist;
        return result;
    }

};

std::string DrawCCFG(CCFG ccfg){
    /*CCFGNode start {nullptr, "start", "asdad", 5};


    std::string result = "\\usetikzlibrary{automata,positioning}\n "
                         "\\begin{tikzpicture}[shorten >=1pt, node distance=2cm, on grid, auto]\n" +
            start.to_string() +
            "\\end{tikzpicture}";*/
}

#endif //ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
