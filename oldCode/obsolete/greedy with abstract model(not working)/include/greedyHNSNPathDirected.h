#include <list>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include "mpGraphInterface.h"
#include "modelInterface.h"

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
// I removed the implication attributes once I realized it does not work :(

    std::vector<std::set<Vertex>> node_implications;
    std::vector<std::set<Vertex>> reverse_node_implications;

    std::map<Vertex,float> nb_values_map;
    std::set<std::pair<float,Vertex>> nb_values_set;
    float sum_of_nodes;

    std::set<Vertex> current_nb_set;
    std::set<Vertex> current_node_set;
};

struct greedySolution{
    float value;
    std::set<Vertex> nodes;
    std::set<Vertex> nbs;
};

struct MPGraphAsGreedyGraph{
    GreedyGraph greedyGraph;

// Kinda unnecessary maybe?
// Makes coding easier, but takes memory and might be slower
    std::map<VertexIndex,Vertex> vertexIndexToInt;
    std::vector<VertexIndex> intToVertexIndex;

};


class GreedyModel : public OptimizableInterface{

private:
    MPGraphInterface* mpGraphPointer;
    MPGraphAsGreedyGraph greedyGraph;
    bool addForwardNeighborConstraints;


public:

    GreedyModel(MPGraphInterface* mpGraphPointer, bool addForwardNeighborConstraints);

    double optimize() override;

    bool vertexPicked(const VertexIndex& v) const override ;
    bool vertexPickedAsNeighbor(const VertexIndex& v) const override;

};


MPGraphAsGreedyGraph mpGraphToGreedyGraph(const MPGraphInterface* mpGraphPointer, bool forwardNeighborConstraints);

GreedyAlgorithmVariables greedyInitialize(GreedyGraph& graph);

    
std::list<Vertex> find_unreachables(const GreedyGraph& graph);

