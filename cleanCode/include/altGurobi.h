#ifndef ALTGUROBI_H 
#define ALTGUROBI_H

#include "../include/mpGraphInterface.h"
#include "../include/mpSetGraph.h"
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <list>

struct GurobiAltSolution{
    float value;
    std::set<VertexIndex> nodes;
    std::set<VertexIndex> neighbors;
    bool solvedToCompletion;
};

struct ProcessedGraph{
    int nr_layers;
    std::set<VertexIndex> nodes;
    std::set<VertexIndex> neighbors;
    std::map<VertexIndex,std::list<VertexIndex>> forward_neighbor;
    std::map<VertexIndex,std::list<VertexIndex>> backward_neighbor;
    std::map<VertexIndex, std::list<VertexIndex>> neighbor_connections;
    std::map<VertexIndex,float> node_values;
};

// Processes the MPSet graph and stores it in the right data structure to run the gurobi algorithm
// biDirectional=false for PDRS and biDirectional=true for NDRS
ProcessedGraph minimalProcessing(const MultipartiteSetGraph& G, bool biDirectional);

// Applies the exact MILP formulation on the processed graph.
// Terminates after maxTimeout seconds.
GurobiAltSolution altGurobiOptimize(const ProcessedGraph& graph, float maxTimeout);

#endif // ALTGUROBI_H