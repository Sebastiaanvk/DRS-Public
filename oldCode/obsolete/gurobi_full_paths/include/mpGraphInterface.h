#ifndef GRAPH_INTERFACE_H
#define GRAPH_INTERFACE_H

#include <utility>
#include <memory>

struct VertexIndex{
    int layer;
    int element;
};

inline bool operator<(const VertexIndex& v1, const VertexIndex& v2){
    if(v1.layer==v2.layer)
        return v1.element<v2.element;
    return v1.layer<v2.layer;
}

inline bool operator==(const VertexIndex& v1, const VertexIndex& v2){
    return v1.layer==v2.layer && v1.element==v2.element;
}

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