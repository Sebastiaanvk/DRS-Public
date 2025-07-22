#ifndef EXPERIMENTS_H 
#define EXPERIMENTS_H

#include <functional>
#include <random>
#include <chrono>
#include <string>
#include <sstream>

#include "algorithmWrappers.h"
#include "graphGeneration.h"
#include "mpSetGraph.h"
#include "matplotlibcpp.h"




void plotProcessingSpeedEdgeIncrease(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm,
int nrGraphs, int nrIterations,int nrNodes, int nrLayers, int minLayerSize,
std::function<float(std::mt19937&)> weightFunction, unsigned int seed);





void demo1(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm);


void demo2();



void runAlgorithmOnFolder(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm, std::string inputFolderPath, std::string outputFileName);


void runAlgorithmOnFolderAlternative(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm, std::string inputFolderPath, std::string outputFileName);



void runAllAlgorithmsOnFolder(std::string inputFolderName, std::string outputFolderName);

void generateAndAlgorithms(unsigned int seed,std::string folderName);

void generateAndHeuristicAlgorithms(unsigned int seed,std::string folderName);
void generateAndHeuristicAlgorithmsContinue(unsigned int seed,std::string folderName, int index);

void generateAndHeuristicAlgorithmsContinue2(unsigned int seed,std::string folderName, int index);

void generateAndHeuristicAlgorithmsOnlyFast(unsigned int seed,std::string folderName, int index);
void generateAndHeuristicAlgorithmsVariableEdges(unsigned int seed,std::string folderName);

void generateAndHeuristicAlgorithmsVariableEdgesNew(unsigned int seed,std::string folderName);
void generateAndHeuristicAlgorithmsVariableNodes(unsigned int seed,std::string folderName);
void generateAndHeuristicAlgorithmsVariableLayers(unsigned int seed,std::string folderName);
void generateAllAlgorithmsFastNrEdges(unsigned int seed,std::string folderName);
void finishUnterminated(std::string folderName,std::string fileName);

#endif // EXPERIMENTS_H