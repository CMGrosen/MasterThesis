//
// Created by CMG on 14/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_CCFGNODE_HPP
#define ANTLR_CPP_TUTORIAL_CCFGNODE_HPP



class CCFGNode {
    static const padding = 10;
public:
    std::string name;

    CCFGNode(std::shared<CCFGNode> _parent, std::string _name, std::string _content, int count) : parent{_parent}, name{_name}, content{_content} {
        node_size = (count * 6) + 6;
    }

    void add_child(std::shared<CCFGNode> child){
        children.emplace_back(child);
    }

    void resize(){
        int temp = calc_children_size();
        if(temp > node_size){
            size = temp
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
        return content
    }
private:
    std::string content;
    int node_size;
    int size;
    std::vector<std::shared<CCFGNode>> children;
    std::shared<CCFGNode> parent;

    int calc_children_size(){
        if(children.empty()){
            return 0;
        }
        if(children.size() == 1){
            return children.front()->get_size();
        }
        int result = children[0]->get_size()/2;
        for(int i = 0; i < children.size()-1; i++){
            result += children[i]->get_size() < children[i+1]->get_size() ?
                    children[i+1]->get_size() + padding*2:
                    children[i]->get_size() + padding*2;
        }
        result += children.back()->get_size() / 2;
        return result;
    }

};


#endif //ANTLR_CPP_TUTORIAL_CCFGNODE_HPP
