#ifndef GRAPH_GENERATION_H 
#define GRAPH_GENERATION_H 


#include "mpGraphInterface.h"
#include "mpSetGraph.h"
#include <map>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <string>
#include <iostream>
#include <fstream>


std::vector<int> generatePartition(int n, int k, std::mt19937& gen);
std::vector<int> generatePartitionMinSize(int n, int k, int minSize, std::mt19937& gen);

std::vector<Edge> totalEdgeVector(const std::vector<int>& partition);
std::vector<std::pair<VertexIndex,VertexIndex>> generateEdges(const std::vector<int>& partition, int nrEdges, std::mt19937& gen);


float uniform_distribution_default(std::mt19937& gen);

std::map<VertexIndex,float> genWeights(const std::vector<int>& partition, std::function<float(std::mt19937&)> weightFunction, std::mt19937& gen);
MultipartiteSetGraph generateGraph(int nrNodes, int nrLayers, int minLayerSize, int nrEdges, std::function<float(std::mt19937&)> weightFunction, std::mt19937& gen);
MultipartiteSetGraph generateGraphDoubleFixedEdges(int nrNodes, int nrLayers, float minLayerRatio, int nrEdges, float oddsSet1, std::function<float(std::mt19937&)> weightFunction1, std::function<float(std::mt19937&)> weightFunction2, unsigned int seed);
MultipartiteSetGraph generateGraphEdgeRatio(int nrNodes, int nrLayers, int minLayerSize, float edgeRatio, std::function<float(std::mt19937&)> weightFunction, unsigned int seed);


MultipartiteSetGraph generateDoubleGraph(int nrNodes, int nrLayers, float minLayerRatio, float oddsSet1, float edgeRatio1, float edgeRatio2, float edgeRatioCross, std::function<float(std::mt19937&)> weightFunction1, std::function<float(std::mt19937&)> weightFunction2, unsigned int seed);
MultipartiteSetGraph generateDoubleGraphDegree(int nrNodes, int nrLayers, float minLayerRatio, float oddsSet1,  float degree1, float degree2, float degreeCross, std::function<float(std::mt19937&)> weightFunction1, std::function<float(std::mt19937&)> weightFunction2, unsigned int seed);

void storeGraph(const MultipartiteSetGraph& graph, unsigned int seed_to_generate, std::string fileName);

MultipartiteSetGraph readGraph(std::string fileName);
MultipartiteSetGraph readGraph(std::string folderName, int index);


#endif // GRAPH_GENERATION_H