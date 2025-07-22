#ifndef GREEDY_HNSN_PATH_DIRECTED_H
#define GREEDY_HNSN_PATH_DIRECTED_H

#include <list>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include "mpGraphInterface.h"
#include "SFMLGraphs.h"

typedef int Vertex;

struct GreedyGraph {
    int nr_nodes;
    std::vector<std::set<Vertex>> forward_arcs;
    std::vector<std::set<Vertex>> backward_arcs;
    std::vector<float> node_values;

    int nr_neighbors;   
    std::vector<std::set<Vertex>> nb_to_nodes;
    std::vector<std::set<Vertex>> node_to_nbs;

    std::list<Vertex> first_layer;
    std::list<Vertex> last_layer;

};

struct GreedyAlgorithmVariables {

    std::map<Vertex,float> nb_values_map;
    std::set<std::pair<float,Vertex>> nb_values_set;
    float sum_of_nodes;

    std::set<Vertex> current_nb_set;
    std::set<Vertex> current_node_set;

    std::vector<float> acc_neighbor_values;
};

struct GreedySolution{
    float value;
    std::set<Vertex> nodes;
    std::set<Vertex> nbs;
};

struct MPGraphAsGreedyGraph{
    GreedyGraph greedyGraph;

    std::map<VertexIndex,Vertex> vertexIndexToInt;
    std::vector<VertexIndex> intToVertexIndex;

};

// Stores the graph in the right data structure for the greedy peeling algorithm
// forwardNeighborConstraints=false for PDRS and forwardNeighborConstraints=true for NDRS
MPGraphAsGreedyGraph mpGraphToGreedyGraph(const MPGraphInterface* mpGraphPointer, bool forwardNeighborConstraints);

// Finds the nodes that can never be part of a reachable subset
std::list<Vertex> find_unreachables(const GreedyGraph& graph);

// Runs the Greedy Peeling algorithm implementation on the graph instance.
GreedySolution greedyAlgorithm(GreedyGraph graph, std::vector<float>& accNeighborValues);
// Runs the Greedy Peeling algorithm implementation on the graph instance and saves the intermediate results as sfml images.
GreedySolution greedyAlgorithmPrintIterations(GreedyGraph graph, std::vector<float>& accNeighborValues, const MPGraphInterface& mpGraph, bool printEachIteration);


#endif //GREEDY_HNSN_PATH_DIRECTED_H