#ifndef MODEL_INTERFACE_H
#define MODEL_INTERFACE_H

#include <set>
#include "mpGraphInterface.h"



struct Solution{
    float value;
    std::set<VertexIndex> nodes;
    std::set<VertexIndex> neighbors;
};

class OptimizableInterface {
public:
    virtual double optimize() = 0;
    virtual bool vertexPicked(const VertexIndex& v) const = 0;
    virtual bool vertexPickedAsNeighbor(const VertexIndex& v) const = 0;
};
#endif //MODEL_INTERFACE_H