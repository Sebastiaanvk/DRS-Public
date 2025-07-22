#ifndef GRAPH_GENERATION_H 
#define GRAPH_GENERATION_H 



#include <map>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <functional>

#include "mpGraphInterface.h"
#include "mpSetGraph.h"




std::vector<int> generatePartition(int n, int k, std::mt19937& gen);
std::vector<int> generatePartitionMinSize(int n, int k, int minSize, std::mt19937& gen);

std::vector<std::pair<VertexIndex,VertexIndex>> generateEdges(const std::vector<int>& partition, int nrEdges, std::mt19937& gen);


float uniform_distribution_default(std::mt19937& gen);

MultipartiteSetGraph generateGraph(int nrNodes, int nrLayers, int minLayerSize, int nrEdges, std::function<float(std::mt19937&)> weightFunction, std::mt19937& gen);






#endif // GRAPH_GENERATION_H