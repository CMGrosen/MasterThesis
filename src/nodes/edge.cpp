//
// Created by hu on 18/02/2020.
//

#include "edge.hpp"

bool edge::operator<(const edge& s) const {
    return std::hash<edge>{}(*this) < std::hash<edge>{}(s);
}
bool edge::operator==(const edge& s) const {
    return neighbours[0] == s.neighbours[0] && neighbours[1] == s.neighbours[1] && type == s.type;
}
bool edge::operator!=(const edge& s) const {
    return !(*this == s);
}

edge::edge() : type{flow} {neighbours.reserve(2);}
edge::edge(std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
        type{flow}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}
edge::edge(edgeType typeOfEdge, std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
        type{typeOfEdge}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}

std::shared_ptr<basicblock> edge::from() {
    return neighbours[0];
}

std::shared_ptr<basicblock> edge::to() {
    return neighbours[1];
}


