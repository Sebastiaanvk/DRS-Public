#include "mpGraphInterface.h"
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <unordered_set>
#include <map>
#include <utility>
#include <list>
#include "mpSetGraph.h"
#include "modelInterface.h"
#include "greedyWithDynPaths.h"


struct ProcessedGraph{
    int nr_layers;
    std::set<VertexIndex> nodes;
    std::set<VertexIndex> neighbors;
    std::map<VertexIndex,std::list<VertexIndex>> forward_neighbor;
    std::map<VertexIndex,std::list<VertexIndex>> backward_neighbor;
    std::map<VertexIndex, std::list<VertexIndex>> neighbor_connections;
    std::map<VertexIndex,float> node_values;
};

ProcessedGraph properPreprocessing(const MultipartiteSetGraph& G, bool two_dir_nbs);


double new_gurobi_test_1dir_direct(const MultipartiteSetGraph& graph, bool print_variables);

double priority_test(const MultipartiteSetGraph& G, bool print_variables);

double preprocessingTest(const MultipartiteSetGraph& G, bool print_variables);


double warm_start_heuristic_test(const MultipartiteSetGraph& G);