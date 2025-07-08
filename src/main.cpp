
#include "../include/algorithmWrappers.h"
#include "../include/experiments.h"
#include "../include/graphGeneration.h"
#include "../include/algorithmWrappers.h"
#include <random>
#include <string>
#include <chrono>
#include <functional>


void testStoringGraph(unsigned int seed){
    int nr_layers = 10;
    int nr_nodes = 120;
    int layer_min_size = 5;
    int nrEdges = 500;
    std::mt19937 gen(seed);

    MultipartiteSetGraph graph = generateGraph(nr_nodes,nr_layers, layer_min_size, nrEdges, uniform_distribution_default, gen);

    std::string fileName = "data/graphs/test_method/test.txt";

    storeGraph(graph, seed, fileName);
    std::cout << "Done storing\n";


    MultipartiteSetGraph graph2 = readGraph(fileName);
    
    displayGraphPNG(graph, 1000,800, "atest1");
    displayGraphPNG(graph2, 1000,800, "atest2");

}



void testing_MILP_speed(unsigned int seed){
    int nrLayers = 7;
    int nrNodes = 200;
    int minLayerSize = 8;
    float percentage_edges = 0.5;
    int max_weight = 1024;
    float weight_coin_flip = 0.7;

    std::function<float(std::mt19937&)> weightFunction = [max_weight, weight_coin_flip](std::mt19937& gen) -> float {
        std::uniform_int_distribution<> dist(0,max_weight);
        std::bernoulli_distribution distb(weight_coin_flip);
        if(distb(gen))
            return dist(gen);
        else
            return 0;
    };

    MultipartiteSetGraph graph = generateGraphEdgeRatio(nrNodes, nrLayers, minLayerSize, percentage_edges, weightFunction, seed);

    auto start = std::chrono::high_resolution_clock::now();
    Algorithms::Solution s = Algorithms::normalGurobiBi(graph);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "The MILP took " << elapsed.count() << " seconds\n";
}

void test_algorithm(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm, std::string fileName, std::string folderName){
    std::string inputFolder = "data/graphs/"+folderName;
    std::string outputFile = "data/results/"+ folderName + "/"+ fileName + ".txt";
    runAlgorithmOnFolder(algorithm, inputFolder, outputFile);
}




void quick_compare_algorithsm(){
    MultipartiteSetGraph graph = readGraph("data/graphs/mini_set/graph0496.txt");
    Algorithms::Solution solGreedyPaths = Algorithms::greedySinglePathsUni(graph);
    std::cout << "Greedy paths solution: " << solGreedyPaths.objectiveValue << std::endl;
    if(Algorithms::checkSolution(solGreedyPaths,graph, false)){
        std::cout << "The greedy paths solution is correct.\n";
    } else {
        std::cout << "The greedy paths solution is incorrect.\n";
    }
    std::cout << (solGreedyPaths.solvedToCompletion?"Greedy Paths solved to completion.":"Greedy Paths terminated early.") << '\n';
    Algorithms::Solution solGurobi = Algorithms::altGurobiUni(graph);
    std::cout << "Gurobi solution: " << solGurobi.objectiveValue << std::endl;
    if(Algorithms::checkSolution(solGurobi,graph, false)){
        std::cout << "The Gurobi solution is correct.\n";
    } else {
        std::cout << "The Gurobi solution is incorrect.\n";
    }

    std::cout << (solGurobi.solvedToCompletion?"Gurobi solved to completion.":"Gurobi terminated early.") << '\n';

    displayGraphPNGNodesNbsPicked(graph,1000,800,solGurobi.pickedNodes,solGurobi.neighbors,"Gurobi Solution");
    displayGraphPNGNodesNbsPicked(graph,1000,800,solGreedyPaths.pickedNodes,solGreedyPaths.neighbors,"Greed Paths Solution");


}



void quick_check_actual_results_mini( unsigned int seed){
    std::srand(seed);
    std::string folderName = "compare/";
    for(int i=0; i<10; i++){
        int index = rand()%600;
        MultipartiteSetGraph graph = readGraph("data/graphs/mini_set",index);
        Algorithms::Solution solGP = Algorithms::greedyPeelingUni(graph);
        Algorithms::Solution solGPB = Algorithms::greedyPeelingBi(graph);
        Algorithms::Solution solGSP = Algorithms::greedySinglePathsUni(graph);
        Algorithms::Solution solGSPB = Algorithms::greedySinglePathsBi(graph);
        Algorithms::Solution solGur = Algorithms::altGurobiUni(graph);
        Algorithms::Solution solGurB = Algorithms::altGurobiBi(graph);

        if(!Algorithms::checkSolution(solGP,graph, false)){
            std::cout << "solGP is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGP.pickedNodes,solGP.neighbors,folderName+"solGP Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGPB,graph, true)){
            std::cout << "solGPB is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGPB.pickedNodes,solGPB.neighbors,folderName+"solGPB Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGSP,graph, false)){
            std::cout << "solGSP is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGSP.pickedNodes,solGSP.neighbors,folderName+"solGSP Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGSPB,graph, true)){
            std::cout << "solGSPB is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGSPB.pickedNodes,solGSPB.neighbors,folderName+"solGSPB Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGur,graph, false)){
            std::cout << "solGur is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGur.pickedNodes,solGur.neighbors,folderName+"solGur Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGurB,graph, true)){
            std::cout << "solGurB is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGurB.pickedNodes,solGurB.neighbors,folderName+"solGurB Solution"+std::to_string(index));
    }
}

void quick_check_actual_results_for_Acc( unsigned int seed){
    std::srand(seed);
    std::string folderName = "compareAcc/";
    for(int i=0; i<10; i++){
        int index = rand()%1584;
        MultipartiteSetGraph graph = readGraph("data/graphs/forAccuracy",index);
        Algorithms::Solution solGP = Algorithms::greedyPeelingUni(graph);
        Algorithms::Solution solGPB = Algorithms::greedyPeelingBi(graph);
        Algorithms::Solution solGSP = Algorithms::greedySinglePathsUni(graph);
        Algorithms::Solution solGSPB = Algorithms::greedySinglePathsBi(graph);
        Algorithms::Solution solGur = Algorithms::altGurobiUni(graph);
        Algorithms::Solution solGurB = Algorithms::altGurobiBi(graph);

        if(!Algorithms::checkSolution(solGP,graph, false)){
            std::cout << "solGP is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGP.pickedNodes,solGP.neighbors,folderName+"solGP Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGPB,graph, true)){
            std::cout << "solGPB is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGPB.pickedNodes,solGPB.neighbors,folderName+"solGPB Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGSP,graph, false)){
            std::cout << "solGSP is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGSP.pickedNodes,solGSP.neighbors,folderName+"solGSP Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGSPB,graph, true)){
            std::cout << "solGSPB is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGSPB.pickedNodes,solGSPB.neighbors,folderName+"solGSPB Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGur,graph, false)){
            std::cout << "solGur is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGur.pickedNodes,solGur.neighbors,folderName+"solGur Solution"+std::to_string(index));

        if(!Algorithms::checkSolution(solGurB,graph, true)){
            std::cout << "solGurB is incorrect!\n";
        }
        displayGraphPNGNodesNbsPicked(graph,1000,800,solGurB.pickedNodes,solGurB.neighbors,folderName+"solGurB Solution"+std::to_string(index));
    }
}

void generateImagesForPresentation(unsigned int seed){
    int maxWeight = 1000;
    float coinOdds = 0.5;
    std::function<float(std::mt19937&)> weightFunction = [maxWeight, coinOdds](std::mt19937& gen) -> float {
        std::uniform_int_distribution<> dist(0,maxWeight);
        std::bernoulli_distribution distb(coinOdds);
        if(distb(gen))
            return dist(gen);
        else
            return 0;
    };
    MultipartiteSetGraph graph = generateGraphEdgeRatio(40,6,6,0.5,weightFunction, seed);
    // greedyDynPaths1dir(graph,true);
    displayGraphPNG(graph, 1000, 600, "Initial Graph");

    MPGraphAsGreedyGraph G = mpGraphToGreedyGraph(&graph, false);
    std::vector<float> acc_neigbor_values;
    GreedySolution solution = greedyAlgorithmPrintIterations(G.greedyGraph, acc_neigbor_values, graph, false);

//    Algorithms::Solution sol = Algorithms::dynSinglePathUni(graph);
    
//    displayGraphPNG(graph,1000,600,"presentation/smallBeforeSingle");

//    displayGraphPNGNodesNbsPicked(graph,1000,600, sol.pickedNodes, sol.neighbors  , "presentation/smallAfterSingle");

//    Algorithms::Solution sol = Algorithms::dynSinglePathBi(graph);
    
//    displayGraphPNG(graph,1000,600,"presentation/smallBeforeBi");

//    displayGraphPNGNodesNbsPicked(graph,1000,600, sol.pickedNodes, sol.neighbors  , "presentation/smallAfterBi");
}


void testGreedyPeeling(){
    int index = 236;
    std::string folderName = "compareAcc/";
    MultipartiteSetGraph graph = readGraph("data/graphs/forAccuracy",index);

    int sum = 0;
    int nr_elts = 0;
    
    for (int i=0;i<graph.getLayers(); i++){
        for(int j=0; j<graph.getElementsInLayer(i); j++){
            sum += graph.getValue({i,j});
            nr_elts += 1;
        }
    }

    std::cout << "sum of weights: " << sum << std::endl;
    std::cout << "number of elements: " << nr_elts << std::endl;

    std::cout << "The value of picking everything is: " << (float) sum/nr_elts << std::endl;

    MPGraphAsGreedyGraph greedyGraph = mpGraphToGreedyGraph(&graph, true);

    int sumGreedy = 0;
    for(int i=0; i<greedyGraph.greedyGraph.nr_nodes; i++){
        sumGreedy += greedyGraph.greedyGraph.node_values[i];
    }
    
    std::cout << "greedy sum of weights: " << sumGreedy << std::endl;
    std::cout << "greedy number of neighbors: " << greedyGraph.greedyGraph.nr_neighbors << std::endl;
    std::cout << "The greedyGraph value of picking everything is: " << (float) sumGreedy/greedyGraph.greedyGraph.nr_neighbors << std::endl;

    Algorithms::Solution solGPB = Algorithms::greedyPeelingBi(graph);

    Algorithms::Solution solGurB = Algorithms::altGurobiBi(graph);

    std::cout << "Optimal value Greedy Peeling Bi: " << solGPB.objectiveValue << std::endl;
    std::cout << "Optimal value Gurobi Bi: " << solGurB.objectiveValue << std::endl;
}

void fixTwentyPerc(){
    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  algorithms = {
        Algorithms::altGurobiUni,Algorithms::altGurobiBi,Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi};
    
    std::vector<std::string> algorithmNames = {"altGurobiUni","altGurobiBi", "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi"};
    std::string inputFolderPath = "data/graphs/forAccuracyTwentyPerc";
    std::string outputFolderPath = "data/results/forAccuracy/";
    for(int i=0; i<algorithms.size(); i++){
        runAlgorithmOnFolderAlternative(algorithms[i],inputFolderPath,outputFolderPath+algorithmNames[i]+".txt");
    }
}


void testDoubleGraphs(unsigned int seed, int nrGraphs){

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  algorithms = {
        Algorithms::altGurobiUni,Algorithms::altGurobiBi,Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};
    
    std::vector<std::string> algorithmNames = {"altGurobiUni","altGurobiBi", "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};


    const std::string inputFolderPath = "data/graphs/doubleMini";
    const int GRAPHS_IN_FOLDER = 7680;
//    const std::string outputImageFolderPath = "output/images/doubleMiniTest";
    const std::string outputImageFolderPath = "doubleMiniTest";


    std::srand(seed);


    for(int i=0; i<nrGraphs; i++){
        int index = rand()%GRAPHS_IN_FOLDER;
        MultipartiteSetGraph graph = readGraph(inputFolderPath,index);
        displayGraphPNG(graph, 2000, 1200, outputImageFolderPath + "/graph" + std::to_string(index));
        std::cout << "For index " << index << ":" << std::endl;
        std::vector<float> results;
        for(int j=0; j<algorithms.size(); j++){
            Algorithms::Solution sol = algorithms[j](graph);
            results.push_back(sol.objectiveValue);
            displayGraphPNGNodesNbsPicked(graph,2000,1200,sol.pickedNodes,sol.neighbors,outputImageFolderPath + "/graph" + std::to_string(index)+algorithmNames[j]);

        }

        for(int j=0; j<algorithms.size(); j++){
            std::cout <<"Accuracy " << algorithmNames[j] << ": " << results[j]/results[j%2] << std::endl;
        }
    }
}

void testDoubleGraphs(unsigned int seed, int nrGraphs, 
            std::string inputFolderPath, int GRAPHS_IN_FOLDER, std::string outputImageFolderPath){

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  algorithms = {
        Algorithms::altGurobiUni,Algorithms::altGurobiBi,Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};
    
    std::vector<std::string> algorithmNames = {"altGurobiUni","altGurobiBi", "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};


//     const std::string inputFolderPath = "data/graphs/doubleMini";
//     const int GRAPHS_IN_FOLDER = 7680;
//    const std::string outputImageFolderPath = "output/images/doubleMiniTest";
//     const std::string outputImageFolderPath = "doubleMiniTest";


    std::srand(seed);
    

    for(int i=0; i<nrGraphs; i++){
        int index = rand()%GRAPHS_IN_FOLDER;
        MultipartiteSetGraph graph = readGraph(inputFolderPath,index);
        displayGraphPNG(graph, 2000, 1200, outputImageFolderPath + "/graph" + std::to_string(index));
        std::cout << "For index " << index << ":" << std::endl;
        std::vector<float> results;
        for(int j=0; j<algorithms.size(); j++){
            Algorithms::Solution sol = algorithms[j](graph);
            results.push_back(sol.objectiveValue);
            displayGraphPNGNodesNbsPicked(graph,2000,1200,sol.pickedNodes,sol.neighbors,outputImageFolderPath + "/graph" + std::to_string(index)+algorithmNames[j]);

        }

        for(int j=0; j<algorithms.size(); j++){
            std::cout <<"Accuracy " << algorithmNames[j] << ": " << results[j]/results[j%2] << std::endl;
        }
    }
}

void testDoubleGraphsCoinFlip(unsigned int seed, int nrGraphs){

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  algorithms = {
        Algorithms::altGurobiUni,Algorithms::altGurobiBi,Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};
    
    std::vector<std::string> algorithmNames = {"altGurobiUni","altGurobiBi", "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};


    const std::string inputFolderPath = "data/graphs/doubleMiniCoinFlip";
    const int GRAPHS_IN_FOLDER = 4319;
//    const std::string outputImageFolderPath = "output/images/doubleMiniTest";
    const std::string outputImageFolderPath = "doubleMiniCoinFlipTest";


    std::srand(seed);


    for(int i=0; i<nrGraphs; i++){
        int index = rand()%GRAPHS_IN_FOLDER;
        MultipartiteSetGraph graph = readGraph(inputFolderPath,index);
        displayGraphPNG(graph, 2000, 1200, outputImageFolderPath + "/graph" + std::to_string(index));
        std::cout << "For index " << index << ":" << std::endl;
        std::vector<float> results;
        for(int j=0; j<algorithms.size(); j++){
            Algorithms::Solution sol = algorithms[j](graph);
            results.push_back(sol.objectiveValue);
            displayGraphPNGNodesNbsPicked(graph,2000,1200,sol.pickedNodes,sol.neighbors,outputImageFolderPath + "/graph" + std::to_string(index)+algorithmNames[j]);

        }

        for(int j=0; j<algorithms.size(); j++){
            std::cout <<"Accuracy " << algorithmNames[j] << ": " << results[j]/results[j%2] << std::endl;
        }
    }
}

struct GraphParametersDegree{
    int nrLayers;
    int nrNodes;
    float oddsSet1;
    float edgeDegree1;
    float edgeDegree2;
    float edgeDegreeCross;
};

std::ostream& operator<<(std::ostream& out, const GraphParametersDegree& pars){
    out << "nrLayers: " << pars.nrLayers << ", ";
    out << "nrNodes: "<< pars.nrNodes << ", ";
    out << "oddsSet1: " << pars.oddsSet1 << ", ";
    out << "edgeDegree1: "<< pars.edgeDegree1 << ", ";
    out << "edgeDegree2: "<< pars.edgeDegree2 << ", ";
    out << "edgeDegreeCross: "<< pars.edgeDegreeCross;
    return out;
}

void testDoubleGraphFixedDegree( unsigned int seed, int nrGraphs){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 8);
    std::uniform_int_distribution<int> nrNodesRand(120, 220);

    std::uniform_real_distribution<float> oddsSet1Rand(0.30,0.50);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,4);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,3);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(1,2);
    // std::uniform_int_distribution<int> nrLayersRand(nrLayersMin, nrLayersMax);
    // std::uniform_int_distribution<int> nrNodesRand(nrNodesMin, nrNodesMax);

    // std::uniform_real_distribution<float> oddsSet1Rand(oddsSet1Min,oddsSet1Max);
    // std::uniform_real_distribution<float> edgeDegree1Rand(edgeRatio1Min,edgeRatio1Max);
    // std::uniform_real_distribution<float> edgeDegree2Rand(edgeRatio2Min,edgeRatio2Max);
    // std::uniform_real_distribution<float> edgeDegreeCrossRand(edgeRatioCrossMin,edgeRatioCrossMax);

    float minLayerRatio = 0.80;

    int maxWeight1 = 1000;
    float coinflip1 = 0.5f;
    int maxWeight2 = 100;
    float coinflip2 = 0.5f;

    std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1,coinflip1](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip1);
        std::uniform_int_distribution<> dist(0,maxWeight1);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };

    std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2,coinflip2](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip2);
        std::uniform_int_distribution<> dist(0,maxWeight2);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };


    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  algorithms = {
        Algorithms::altGurobiUni,Algorithms::altGurobiBi,Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> algorithmNames = {"altGurobiUni","altGurobiBi", "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string outputImageFolderPath = "quickDoubleDegreeTest";

    std::vector<GraphParametersDegree> totalPars;
    std::vector<std::vector<float>> totalResults;
    std::vector<std::vector<float>> totalTimes;

    for(int index=0; index<nrGraphs; index++){
        GraphParametersDegree pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index);
        
        displayGraphPNG(graph, 2000, 1200, outputImageFolderPath + "/graph" + std::to_string(index));
        std::vector<float> results;
        std::vector<float> times;
        for(int j=0; j<algorithms.size(); j++){
            auto start = std::chrono::high_resolution_clock::now();  
            Algorithms::Solution sol = algorithms[j](graph);
            auto end = std::chrono::high_resolution_clock::now();  
            std::chrono::duration<double> elapsed = end - start;
            results.push_back(sol.objectiveValue);
            times.push_back(elapsed.count());
            displayGraphPNGNodesNbsPicked(graph,2000,1200,sol.pickedNodes,sol.neighbors,outputImageFolderPath + "/graph" + std::to_string(index)+algorithmNames[j]);
        }
        for(int j=0; j<algorithms.size(); j++){
            std::cout <<"Accuracy " << algorithmNames[j] << ": " << results[j]/results[j%2] << ", time elapsed: " << times[j] << std::endl;
        }
        totalPars.push_back(pars);
        totalResults.push_back(results);
        totalTimes.push_back(times);
    }
    for(int index=0; index<nrGraphs; index++){
        std::cout << "Graph " << index << std::endl;
        std::cout << totalPars[index] << std::endl;
        for(int j=0; j<algorithms.size(); j++){
            std::cout <<"Accuracy " << algorithmNames[j] << ": " << totalResults[index][j]/totalResults[index][j%2] << ", time elapsed: " << totalTimes[index][j] << std::endl;
        }
        
    }
    
}

void checkIndividualTimes(unsigned int seed, const std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm,const std::string& AlgName){
    GraphParametersDegree pars ;

    pars.nrLayers = 8;
    pars.nrNodes = 3000;
    pars.oddsSet1 = 0.35;
    pars.edgeDegree1 = 6;
    pars.edgeDegree2 = 6;
    pars.edgeDegreeCross = 2;


    float minLayerRatio = 0.80;
    int graphsPerPars = 1;

    int maxWeight1 = 1000;
    float coinflip1 = 0.5f;
    int maxWeight2 = 100;
    float coinflip2 = 0.5f;

    std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1,coinflip1](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip1);
        std::uniform_int_distribution<> dist(0,maxWeight1);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };

    std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2,coinflip2](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip2);
        std::uniform_int_distribution<> dist(0,maxWeight2);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };
    std::cout << AlgName << std::endl;
    for(int i=0; i<graphsPerPars; i++){
        MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,seed);
        auto start = std::chrono::high_resolution_clock::now();  
        Algorithms::Solution sol = algorithm(graph);
        auto end = std::chrono::high_resolution_clock::now();  
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Graph " << i << " time: " << elapsed.count() << std::endl;

    }

}

void testNewGraphGeneration(){
    int maxWeight1 = 1000;
    float coinflip1 = 0.5f;
    int maxWeight2 = 100;
    float coinflip2 = 0.5f;

    std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1,coinflip1](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip1);
        std::uniform_int_distribution<> dist(0,maxWeight1);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };

    std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2,coinflip2](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip2);
        std::uniform_int_distribution<> dist(0,maxWeight2);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };

    MultipartiteSetGraph graph = generateGraphDoubleFixedEdges(600,5,0.80,2500,0.25,weightFunction1,weightFunction2,10);
    displayGraphPNG(graph, 10000,10000, "graphTest1");
}

void generateImagesForPresentation2(unsigned int seed){

    int nrLayers = 15;
    int nrNodes = 800;

    float oddsSet1 = 0.30;
    float edgeDegree1 = 10;
    float edgeDegree2 = 5;
    float edgeDegreeCross = 2;

    float minLayerRatio = 0.80;

    int maxWeight1 = 1000;
    float coinflip1 = 0.5f;
    int maxWeight2 = 100;
    float coinflip2 = 0.5f;

    std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1,coinflip1](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip1);
        std::uniform_int_distribution<> dist(0,maxWeight1);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };

    std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2,coinflip2](std::mt19937& gen) -> float {
        std::bernoulli_distribution distb(coinflip2);
        std::uniform_int_distribution<> dist(0,maxWeight2);
        if(distb(gen)){
            return dist(gen);
        }
        return 0;
    };




    MultipartiteSetGraph graph = generateDoubleGraphDegree(nrNodes, nrLayers, minLayerRatio, oddsSet1, edgeDegree1, edgeDegree2,edgeDegreeCross, weightFunction1, weightFunction2, seed);
    displayGraphPNG(graph, 10000, 6000, "Cool Initial Graph");

    Algorithms::Solution sol= Algorithms::greedySinglePathsUni(graph);


    displayGraphPNGNodesNbsPicked(graph,10000,6000, sol.pickedNodes, sol.neighbors  , "cool sol");

}


int main(){
//    demo2();
//    demo1(Algorithms::greedySinglePathsUni);
//    testStoringGraph(0);
//    testing_MILP_speed(0);
//    testing_MILP_speed(1);
//    testing_MILP_speed(2);
//    saveGraphsUniformCoinflip1DefaultValues(3);
//     test_algorithm(Algorithms::altGurobiUni,"altGurobiUni", "mini_set");
//     test_algorithm(Algorithms::altGurobiBi,"altGurobiBi", "mini_set");
//     test_algorithm(Algorithms::normalGurobiUni,"normalGurobiUni", "mini_set");
//     test_algorithm(Algorithms::greedySinglePathsUni,"greedySinglePathsUni", "mini_set");
//     test_algorithm(Algorithms::greedyPeelingUni,"greedyPeelingUni", "mini_set");
//     test_algorithm(Algorithms::greedyPeelingBi,"greedyPeelingBi", "mini_set");
//     test_algorithm(Algorithms::greedyPeelingUni,"greedyPeelingUni2", "mini_set");
//     test_algorithm(Algorithms::greedyPeelingBi,"greedyPeelingBi2", "mini_set");
//    quick_compare_algorithsm();
//      test_algorithm(Algorithms::greedySinglePathsUni,"greedySinglePathsUni", "medium_set");
//      test_algorithm(Algorithms::greedySinglePathsBi,"greedySinglePathsBi", "medium_set");
//      test_algorithm(Algorithms::greedyPeelingUni,"greedyPeelingUni", "medium_set");
//      test_algorithm(Algorithms::greedyPeelingBi,"greedyPeelingBi", "medium_set");


//    saveGraphsConstVarsUniformEdgesDefaultValues(6);
    // test_algorithm(Algorithms::greedySinglePathsUni,"greedySinglePathsUni", "edgesUniformMini");
    // test_algorithm(Algorithms::greedySinglePathsBi,"greedySinglePathsBi", "edgesUniformMini");
    // test_algorithm(Algorithms::greedyPeelingUni,"greedyPeelingUni", "edgesUniformMini");
    // test_algorithm(Algorithms::greedyPeelingBi,"greedyPeelingBi", "edgesUniformMini");

    // test_algorithm(Algorithms::altGurobiUni,"altGurobiUni", "edgesUniformMini");
    // test_algorithm(Algorithms::altGurobiBi,"altGurobiBi", "edgesUniformMini");

//    generateImagesForPresentation(300);
//    quick_check_actual_results_mini(100);


//    saveGraphsUniformCoinflip1DefaultValues(7);

    // std::cout << "Starting Gur Uni\n";
    // test_algorithm(Algorithms::altGurobiUni,"altGurobiUni", "forAccuracy");
    // std::cout << "Starting Gur Bi\n";
    // test_algorithm(Algorithms::altGurobiBi,"altGurobiBi", "forAccuracy");

    // std::cout << "Starting GSP Uni\n";
    // test_algorithm(Algorithms::greedySinglePathsUni,"greedySinglePathsUni", "forAccuracy");
    // std::cout << "Starting GSPBI \n";
    // test_algorithm(Algorithms::greedySinglePathsBi,"greedySinglePathsBi", "forAccuracy");

//    std::cout << "Starting GP Uni\n";

//    test_algorithm(Algorithms::greedyPeelingUni,"greedyPeelingUni", "forAccuracy");
//    std::cout << "Starting GP Bi\n";
//    test_algorithm(Algorithms::greedyPeelingBi,"greedyPeelingBi", "forAccuracy");

    // quick_check_actual_results_for_Acc(100);
//    testGreedyPeeling();

    // test_algorithm(Algorithms::baseLinePickEverythingUni,"baselineEverythingUni", "forAccuracy");
    // test_algorithm(Algorithms::baseLinePickEverythingBi,"baselineEverythingBi", "forAccuracy");

//    saveGraphsUniformCoinflip1DefaultValues(7);

//    fixTwentyPerc();
//    saveDoubleGraphsDefaultValues(5);


    // testDoubleGraphs(7,20);
    
    // saveDoubleGraphsCoinFlipDefaultValues(7);

    // doubleGraphsDefaultUniformRanges(9);

    // testDoubleGraphsCoinFlip(8,20);

    // runAllAlgorithmsOnFolder("doubleGraphsCoinFlipUniform","doubleGraphsCoinFlipUniform");

    // runAllAlgorithmsOnFolder("doubleGraphsCoinFlipUniform2","doubleGraphsCoinFlipUniform2");


    // testDoubleGraphs(10, 20,"data/graphs/doubleGraphsCoinFlipUniform",3000,"doubleGraphsCoinFlipUniformTest");

    // testDoubleGraphFixedDegree(6,20);
    // generateAndHeuristicAlgorithms(5,"doubleDegreeAllNightHeuristic");
    // generateAndHeuristicAlgorithmsContinue(50000,"doubleDegreeAllNightHeuristic3",1415);
    // generateAndHeuristicAlgorithmsContinue2(12000,"allNightHeurNew",0);
    // checkIndividualTimes(10,Algorithms::greedySinglePathsUni, "greedySinglePathsUni");
    // checkIndividualTimes(10,Algorithms::greedyPeelingUni,"greedyPeelingUni");
    // checkIndividualTimes(10,Algorithms::greedyPeelingBi, "greedyPeelingBi");
    // generateAndHeuristicAlgorithmsOnlyFast(532,"allNightOnlyFast", 0);
    // generateAndHeuristicAlgorithmsVariableEdges(315,"allNightVariableEdges");

    // generateAndHeuristicAlgorithmsVariableEdgesNew(128,"allNightVariableEdgesNew");
    // generateAndHeuristicAlgorithmsVariableNodes(129,"allNightVariableNodes");
    // generateAndHeuristicAlgorithmsVariableLayers(130,"allNightVariableLayers");
    // generateAllAlgorithmsFastNrEdges(131,"allNightAllAlgorithmsFast");


    // generateImagesForPresentation2(5);
    // finishUnterminated("allNightAllAlgorithmsFast","altGurobiBi.txt");
    finishUnterminated("allNightAllAlgorithmsFast","altGurobiUni.txt");



    return 0;
}