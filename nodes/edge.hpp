//
// Created by hu on 18/02/2020.
//

#ifndef ANTLR_CPP_TUTORIAL_EDGE_HPP
#define ANTLR_CPP_TUTORIAL_EDGE_HPP

#include <vector>
#include <memory>

struct basicblock;

enum edgeType {flow, conflict};

class edge {
public:
    edgeType type;
    std::vector<std::shared_ptr<basicblock>> neighbours;
    bool operator<(const edge& s) const;
    bool operator==(const edge& s) const;
    edge();
    edge(std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB);
    edge(edgeType typeOfEdge, std::shared_ptr<basicblock> lB, std::shared_ptr<basicblock> rB);
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


namespace std {

    template<> struct hash<edge> {
        size_t operator()(const edge& s) const {
            size_t seed = 0;
            if (s.neighbours[0] < s.neighbours[1]) hash_combine(seed, s.neighbours[0]);
            else hash_combine(seed, s.neighbours[1]);
            hash_combine(seed, s.type);
            return seed;
        }
    };
}

#endif //ANTLR_CPP_TUTORIAL_EDGE_HPP
