#ifndef GRAPHS_H 
#define GRAPHS_H 

#include <set>
#include <map>
#include <cstdlib>
#include <iostream>


struct VertexIndex{
    int layer;
    int element;
};

bool operator<(const VertexIndex& v1, const VertexIndex& v2);
bool operator==(const VertexIndex& v1, const VertexIndex& v2);

struct MultipartiteGraph{
    int getLayers() const;
    int getElementsInLayer(int layer) const;
    float getValue(const VertexIndex& v) const;
    // Copy assignment operator
    MultipartiteGraph& operator=(const MultipartiteGraph& other);

    int layers;
    std::map<int,int> elementsPerLayer;
    std::set<std::pair<VertexIndex,VertexIndex>> edges;
    std::map<VertexIndex,float> values;
};




MultipartiteGraph generate_random_graph(int nr_layers, std::map<int,int> elementsPerLayer,  int max_weight, float edge_density, unsigned int seed);
MultipartiteGraph generate_random_graph(int nr_layers, std::map<int,int> elementsPerLayer, int max_weight, float probZeroValue, float edge_density, unsigned int seed);


void display_graph(const MultipartiteGraph& G);


#endif // GRAPHS_H