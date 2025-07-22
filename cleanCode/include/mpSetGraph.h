#ifndef MP_SET_GRAPHS_H 
#define MP_SET_GRAPHS_H 

#include <utility>
#include <set>
#include <map>
#include <cstdlib>
#include <iostream>
#include <memory>
#include "mpGraphInterface.h"


class mpSetEdgeIterator : public EdgeIterator{
public:
    mpSetEdgeIterator( const std::set<Edge>& edges) : edgesRef(edges), it(edges.begin()) {}
    void next() override;
    bool has_next() const override;
    std::pair<VertexIndex,VertexIndex> current() const override;
private:
    std::set<Edge>::iterator it;
    const std::set<Edge>& edgesRef;
};

// Class to stores the graph instances
class MultipartiteSetGraph : public MPGraphInterface{
public:
    // Copy assignment operator
    MultipartiteSetGraph& operator=(const MultipartiteSetGraph& other);

    int getLayers() const override;
    int getElementsInLayer(int layer) const override;
    float getValue(const VertexIndex& v) const override;
    std::unique_ptr<EdgeIterator> getEdgeIterator() const override {return std::make_unique<mpSetEdgeIterator>(mpSetEdgeIterator(edges));}

    int layers;
    std::map<int,int> elementsPerLayer;
    std::set<Edge> edges;
    std::map<VertexIndex,float> values;
    
    
};

void display_graph(const MultipartiteSetGraph& G);


#endif // MP_SET_GRAPHS_H