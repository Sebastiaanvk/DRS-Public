#ifndef GRAPH_INTERFACE_H
#define GRAPH_INTERFACE_H

#include <utility>
#include <memory>
#include <set>
#include <iostream>

struct VertexIndex{
    int layer;
    int element;
};

struct PickedSet{
    std::set<VertexIndex> nodes;
    std::set<VertexIndex> nbs;
};

inline bool operator<(const VertexIndex& v1, const VertexIndex& v2){
    if(v1.layer==v2.layer)
        return v1.element<v2.element;
    return v1.layer<v2.layer;
}

inline std::ostream& operator<<(std::ostream& os, const VertexIndex& vi)
{
    os << "(" << vi.layer << "," << vi.element << ")"; 
    return os;
}

inline bool operator==(const VertexIndex& v1, const VertexIndex& v2){
    return v1.layer==v2.layer && v1.element==v2.element;
}
template<>
struct std::hash<VertexIndex> {
        size_t operator()(const VertexIndex& s) const {
            return hash<int>()(s.layer) ^ (hash<int>()(s.element) << 1);
        }
    };

typedef std::pair<VertexIndex,VertexIndex> Edge;

class EdgeIterator{
public:
    virtual void next() = 0;
    virtual bool has_next() const =0;
    virtual Edge current() const =0; 
};

class MPGraphInterface{
public:
    virtual int getLayers() const = 0;
    virtual int getElementsInLayer(int layer) const = 0;
    virtual float getValue(const VertexIndex& v) const = 0;
    virtual std::unique_ptr<EdgeIterator> getEdgeIterator() const = 0;
};

#endif //GRAPH_INTERFACE_H