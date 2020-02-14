//
// Created by CMG on 14/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#define ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
#include <basicblockTreeConstructor.hpp>

static std::string DrawCCFG(CCFG ccfg){

}

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

    std::string to_string(){
        std::string result;
        result = "\\node[state] (" + name + ") [text width = " + std::to_string(node_size) + "pt, rectangle";
        if(parent){
            if(distance == 0){
                result += "below = " + std::to_string(vertical_padding) + "pt of ";
            } else if(distance < 0){
                result+= "below left = " + std::to_string(vertical_padding) + "pt and " + std::to_string(distance*-1) + "pt of ";
            } else {
                result+= "below right = " + std::to_string(vertical_padding) + "pt and " + std::to_string(distance) + "pt of ";
            }
            result +=  parent->name;
        }
        result += "] { \\texttt{" + content +"}};";
        for(const auto& child : children){
            result += child->to_string();
        }
        return result;
    }

    void add_child(const std::shared_ptr<CCFGNode>& child){
        children.emplace_back(child);
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
            left = count/2;
            int mid = count /2+1;
            right = count/2+2;
            dist = children[mid]->get_size()/2+padding;
            children[mid]->distance = 0;
        } else {
            left = count/2;
            right = count/2+1;
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

#endif //ANTLR_CPP_TUTORIAL_CCFGILLUSTRATOR_HPP
