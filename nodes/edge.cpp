//
// Created by hu on 18/02/2020.
//

#include "edge.hpp"

std::vector<std::shared_ptr<basicblock>> neighbours;
bool edge::operator<(const edge& s) const { return neighbours[0] < s.neighbours[1];
    if (neighbours[0] == s.neighbours[1] || neighbours[1] == s.neighbours[0])
        return neighbours[0] < s.neighbours[1] && neighbours[1] < s.neighbours[0] && type < s.type;
    else
        return neighbours[0] < s.neighbours[0] && neighbours[1] < s.neighbours[1] && type < s.type;
}
bool edge::operator==(const edge& s) const {
    if (neighbours[0] == s.neighbours[1])
        return neighbours[0] == s.neighbours[1] && neighbours[1] == s.neighbours[0] && type == s.type;
    else
        return neighbours[0] == s.neighbours[0] && neighbours[1] == s.neighbours[1] && type == s.type;
}
edge::edge() : type{flow} {neighbours.reserve(2);}
edge::edge(std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
        type{flow}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}
edge::edge(edgeType typeOfEdge, std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB) :
        type{typeOfEdge}, neighbours{std::vector<std::shared_ptr<basicblock>>{std::move(lB), std::move(rB)}} {}