#ifndef GRAPH_GENERATION_H 
#define GRAPH_GENERATION_H 



#include <map>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <string>
#include <iostream>
#include <fstream>
//#include <omp.h>
         

#include "mpGraphInterface.h"
#include "mpSetGraph.h"




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

void saveGraphsUniformCoinflip1(unsigned int seed, 
    std::vector<int> nrLayers,
    std::vector<int> nrNodes,
    std::vector<float> edgesRatios,
    std::vector<float> weightCoinFlipOdds,
    int minEltsPerLayer, int maxWeight, std::string folderPath);

void saveGraphsUniformCoinflip1DefaultValues(unsigned int seed);

void saveGraphsConstVarsUniformEdges(unsigned int seed, int nrLayers, int nrNodes,
    float minEdgesRatios,float maxEdgesRatios, float minWeightCoinFlipOdds,float maxWeightCoinFlipOdds,
    int minEltsPerLayer, int maxWeight, std::string folderPath, int nrGraphs);

void saveGraphsConstVarsUniformEdgesDefaultValues(unsigned int seed);

void saveDoubleGraphs(unsigned int seed, std::vector<int> nrLayers, std::vector<int> nrNodes, float minLayerRatio, 
      std::vector<float> oddsSet1, std::vector<float> edgeRatio1, std::vector<float> edgeRatio2, std::vector<float> edgeRatioCross,
      std::vector<float> weightRatio, int graphsPerPars, std::string folderPath);

void saveDoubleGraphsDefaultValues(unsigned int seed);

void saveDoubleGraphsCustomWeight(unsigned int seed, std::vector<int> nrLayers, std::vector<int> nrNodes, float minLayerRatio, 
      std::vector<float> oddsSet1, std::vector<float> edgeRatio1, std::vector<float> edgeRatio2, std::vector<float> edgeRatioCross,
      std::function<float(std::mt19937&)> weightFunction1,std::function<float(std::mt19937&)> weightFunction2, int graphsPerPars, std::string folderPath);

void saveDoubleGraphsCoinFlipDefaultValues(unsigned int seed);

void doubleGraphsDefaultUniformRanges(unsigned int seed);

#endif // GRAPH_GENERATION_H