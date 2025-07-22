#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <string>
#include <Python.h> 
#include <functional>

#include "../include/mpSetGraph.h"
#include "../include/SFMLGraphs.h"
#include "../include/singlePathDyn.h"
#include "../include/mpGraphInterface.h"
#include "../include/greedyWithDynPaths.h"
#include "../include/matplotlibcpp.h"
#include "../include/graphGeneration.h"


void plotTestEdgeIncreaseSinglePath(int nrGraphs, int nrIterations, unsigned int seed){
    int nrNodes = 500;
    int nrLayers = 10;
    int minLayerSize = 5;
    std::function<float(std::mt19937&)> weightFunction = [](std::mt19937& gen) -> float {
        std::uniform_int_distribution<> dist(0,1024);
        std::bernoulli_distribution distb(0.7);
        if(distb(gen))
            return dist(gen);
        else
            return 0;
    };
    
    std::mt19937 gen(seed);
    std::vector<MultipartiteSetGraph> graphs(nrGraphs);
    std::vector<std::vector<Edge>> totalEdges(nrGraphs);
    int maxEdges = 1000000;
    for(int i=0; i<nrGraphs; i++){
        std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers,minLayerSize,gen);
        totalEdges[i] = totalEdgeVector(partition);
        std::shuffle(totalEdges[i].begin(),totalEdges[i].end(),gen);
        graphs[i].values = genWeights(partition, weightFunction, gen);
        graphs[i].layers = partition.size();
        for(int j=0;j<partition.size(); j++){
            graphs[i].elementsPerLayer[j] = partition[j];
        }
//        graphs[i] = generateGraph(nrNodes, nrLayers,minLayerSize, 100000, weightFunction, gen);
        maxEdges = std::min(maxEdges,int(totalEdges[i].size()));
    }

    std::cout << "The maximum number of edges is: " << maxEdges << std::endl;

    int steps = maxEdges / (nrIterations+1);
    std::map<int,float> elapsedTimes;

    for(int j = steps; j<=maxEdges; j+=steps ){
        float totalTime = 0.0;
        for(int graphInd = 0; graphInd<nrGraphs; graphInd += 1){
            graphs[graphInd].edges = {};
            graphs[graphInd].edges.insert(totalEdges[graphInd].begin(),totalEdges[graphInd].begin()+j);

//            displayGraphPNG(graphs[graphInd],1000,800, "graph" + std::to_string(graphInd) + "_edges_"+ std::to_string(j));
            auto startDyn = std::chrono::high_resolution_clock::now();
            SinglePathDynSol dynSol = singlePathDyn1DirNbs(graphs[graphInd]);
            auto endDyn = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedDyn = endDyn-startDyn;
            totalTime += elapsedDyn.count();
        }
        elapsedTimes[j] = totalTime/nrGraphs;
    }
    for(auto x : elapsedTimes ){
        std::cout << x.first << ": " << x.second << std::endl;
    }
    std::vector<int> xValues;
    std::vector<float> yValues;
    for(auto x : elapsedTimes){
        xValues.push_back(x.first);
        yValues.push_back(x.second);
    }
    matplotlibcpp::plot(xValues,yValues);
    matplotlibcpp::show(); 

}


void plotTestEdgeIncrease(int nrGraphs, int nrIterations, unsigned int seed){
    int nrNodes = 300;
    int nrLayers = 20;
    int minLayerSize = 5;
    std::function<float(std::mt19937&)> weightFunction = [](std::mt19937& gen) -> float {
        std::uniform_int_distribution<> dist(0,1024);
        std::bernoulli_distribution distb(0.7);
        if(distb(gen))
            return dist(gen);
        else
            return 0;
    };
    
    std::mt19937 gen(seed);
    std::vector<MultipartiteSetGraph> graphs(nrGraphs);
    std::vector<std::vector<Edge>> totalEdges(nrGraphs);
    int maxEdges = 1000000;
    for(int i=0; i<nrGraphs; i++){
        std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers,minLayerSize,gen);
        totalEdges[i] = totalEdgeVector(partition);
        std::shuffle(totalEdges[i].begin(),totalEdges[i].end(),gen);
        graphs[i].values = genWeights(partition, weightFunction, gen);
        graphs[i].layers = partition.size();
        for(int j=0;j<partition.size(); j++){
            graphs[i].elementsPerLayer[j] = partition[j];
        }
//        graphs[i] = generateGraph(nrNodes, nrLayers,minLayerSize, 100000, weightFunction, gen);
        maxEdges = std::min(maxEdges,int(totalEdges[i].size()));
    }

    std::cout << "The maximum number of edges is: " << maxEdges << std::endl;

    int steps = maxEdges / (nrIterations+1);
    std::map<int,float> elapsedTimes;

    for(int j = steps; j<=maxEdges; j+=steps ){
        float totalTime = 0.0;
        for(int graphInd = 0; graphInd<nrGraphs; graphInd += 1){
            graphs[graphInd].edges = {};
            graphs[graphInd].edges.insert(totalEdges[graphInd].begin(),totalEdges[graphInd].begin()+j);

//            displayGraphPNG(graphs[graphInd],1000,800, "graph" + std::to_string(graphInd) + "_edges_"+ std::to_string(j));
            auto startDyn = std::chrono::high_resolution_clock::now();
            GreedyWithDynPathsSol dynSol = greedyDynPaths1dir(graphs[graphInd],false);
            auto endDyn = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedDyn = endDyn-startDyn;
            totalTime += elapsedDyn.count();
        }
        elapsedTimes[j] = totalTime/nrGraphs;
    }
    for(auto x : elapsedTimes ){
        std::cout << x.first << ": " << x.second << std::endl;
    }
    std::vector<int> xValues;
    std::vector<float> yValues;
    for(auto x : elapsedTimes){
        xValues.push_back(x.first);
        yValues.push_back(x.second);
    }
    matplotlibcpp::plot(xValues,yValues);
    matplotlibcpp::show(); 

}





int main(){
    plotTestEdgeIncrease(10,20, 0);

    return 0;
}