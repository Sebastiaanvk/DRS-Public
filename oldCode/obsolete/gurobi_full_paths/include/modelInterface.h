#ifndef MODEL_INTERFACE_H
#define MODEL_INTERFACE_H

#include "mpGraphInterface.h"


class OptimizableInterface {
public:
    virtual double optimize() = 0;
    virtual bool vertexPicked(const VertexIndex& v) const = 0;
    virtual bool vertexPickedAsNeighbor(const VertexIndex& v) const = 0;
};
#endif //MODEL_INTERFACE_H