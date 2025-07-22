#include "../include/experiments.h"




void plotProcessingSpeedEdgeIncrease(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm,
            int nrGraphs, int nrIterations,int nrNodes, int nrLayers, int minLayerSize,
            std::function<float(std::mt19937&)> weightFunction, unsigned int seed){
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
        std::cout << j << " out of " << maxEdges << std::endl;
        float totalTime = 0.0;
        for(int graphInd = 0; graphInd<nrGraphs; graphInd += 1){
            graphs[graphInd].edges = {};
            graphs[graphInd].edges.insert(totalEdges[graphInd].begin(),totalEdges[graphInd].begin()+j);

//            displayGraphPNG(graphs[graphInd],1000,800, "graph" + std::to_string(graphInd) + "_edges_"+ std::to_string(j));
            auto startDyn = std::chrono::high_resolution_clock::now();
            Algorithms::Solution sol = algorithm(graphs[graphInd]);
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


        
void demo1(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm){
    int nrGraphs = 7;
    int nrIterations = 10;
    int nrNodes = 150;
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
    unsigned int seed = 0;
    plotProcessingSpeedEdgeIncrease(algorithm,nrGraphs,nrIterations,nrNodes,nrLayers,minLayerSize,weightFunction,seed);
}


void demo2(){
    std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm = Algorithms::dynSinglePathUni;
    int nrGraphs = 10;
    int nrIterations = 20;
    int nrNodes = 300;
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
    unsigned int seed = 0;
    plotProcessingSpeedEdgeIncrease(algorithm,nrGraphs,nrIterations,nrNodes,nrLayers,minLayerSize,weightFunction,seed);
}



std::string convFileName(int index){
    std::string str = std::to_string(index);
    while(str.size()<4){
        str = "0"+str;
    }
    return "graph"+str+ ".txt";
}

void runAlgorithmOnFolder(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm, std::string inputFolderPath, std::string outputFileName){

    int index = 0;
    std::string fileName = convFileName(index); // all the graph .txt files are named "graph%%%%.txt", where %%%% are four digits indicating the index.
    std::ifstream input(inputFolderPath + "/"+fileName);
    std::ofstream output(outputFileName);
    if(!output){
        std::cout << "Failed to open output!\n";
        return;
    }
    output.clear();
    output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion\n";
    while(input){
        input.close();
        std::cout << index << '\n';
        MultipartiteSetGraph graph = readGraph(inputFolderPath + "/"+fileName);
        
        auto start = std::chrono::high_resolution_clock::now();
        Algorithms::Solution sol = algorithm(graph);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        int nrNodes = 0;
        for(int i=0; i<graph.layers; i++){
            nrNodes += graph.elementsPerLayer[i];
        }
        output << index << ',' << graph.layers << ',' << nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") <<  '\n';//std::endl;
        index++;
        fileName = convFileName(index);
        input.open(inputFolderPath + "/" + fileName);
    }
    output.close();
}

//This function was written for a specific situations where I had to add extra results to the existing results.
void runAlgorithmOnFolderAlternative(std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)> algorithm, std::string inputFolderPath, std::string outputFileName){

    int index = 1584;
    std::string fileName = convFileName(index); // all the graph .txt files are named "graph%%%%.txt", where %%%% are four digits indicating the index.
    std::ifstream input(inputFolderPath + "/"+fileName);
    std::ofstream output(outputFileName,std::ios::app);
    if(!output){
        std::cout << "Failed to open output!\n";
        return;
    }
    while(input){
        input.close();
        std::cout << index << '\n';
        MultipartiteSetGraph graph = readGraph(inputFolderPath + "/"+fileName);
        
        auto start = std::chrono::high_resolution_clock::now();
        Algorithms::Solution sol = algorithm(graph);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        int nrNodes = 0;
        for(int i=0; i<graph.layers; i++){
            nrNodes += graph.elementsPerLayer[i];
        }
        output << index << ',' << graph.layers << ',' << nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") <<  '\n';//std::endl;
        index++;
        fileName = convFileName(index);
        input.open(inputFolderPath + "/" + fileName);
    }
    output.close();
}



void runAllAlgorithmsOnFolder(std::string inputFolderName, std::string outputFolderName){

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  algorithms = {
        Algorithms::altGurobiUni,Algorithms::altGurobiBi,Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};
    
    std::vector<std::string> algorithmNames = {"altGurobiUni","altGurobiBi", "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string inputFolder = "data/graphs/"+inputFolderName;

    for(int i=0; i<algorithms.size(); i++){
        std::string outputFileName = "data/results/" + outputFolderName + "/" + algorithmNames[i] + ".txt";
        runAlgorithmOnFolder(algorithms[i],inputFolder,outputFileName);
    }
}


struct GraphParametersDegreeGA{
    int nrLayers;
    int nrNodes;
    float oddsSet1;
    float edgeDegree1;
    float edgeDegree2;
    float edgeDegreeCross;
};

std::ostream& operator<<(std::ostream& out, const GraphParametersDegreeGA& pars){
    out << "nrLayers: " << pars.nrLayers << ", ";
    out << "nrNodes: "<< pars.nrNodes << ", ";
    out << "oddsSet1: " << pars.oddsSet1 << ", ";
    out << "edgeDegree1: "<< pars.edgeDegree1 << ", ";
    out << "edgeDegree2: "<< pars.edgeDegree2 << ", ";
    out << "edgeDegreeCross: "<< pars.edgeDegreeCross;
    return out;
}

void generateAndAlgorithms(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 7);
    std::uniform_int_distribution<int> nrNodesRand(100, 260);

    std::uniform_real_distribution<float> oddsSet1Rand(0.30,0.50);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,5.5);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,3.5);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(0.6,2);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;


        std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
        outputParsData << "index,nrLayers,nrNodes,oddsSet1,edgeDegree1,edgeDegree2,edgeDegreeCross\n";

    for(int j=0; j<algorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + algorithmNames[j] + ".txt",std::ios::app);
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion\n";
    }
    
    int index = 0;

    while(true){
        GraphParametersDegreeGA pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index+seed);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
            outputParsData << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;

            for(int j=0; j<algorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = algorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + algorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << pars.nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}



void generateAndHeuristicAlgorithms(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 10);
    std::uniform_int_distribution<int> nrNodesRand(100, 1800);


    std::uniform_real_distribution<float> oddsSet1Rand(0.30,0.50);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,10);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,6);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(0.6,3.5);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;


    std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt");
    outputParsData.clear();
    outputParsData << "index,nrLayers,nrNodes,oddsSet1,edgeDegree1,edgeDegree2,edgeDegreeCross\n";

    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion\n";
    }
    
    int index = 0;

    while(true){
        GraphParametersDegreeGA pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index+seed);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
            outputParsData << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;
            std::cout << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << pars.nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}

void generateAndHeuristicAlgorithmsContinue(unsigned int seed,std::string folderName, int index){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 10);
    std::uniform_int_distribution<int> nrNodesRand(100, 1800);


    std::uniform_real_distribution<float> oddsSet1Rand(0.30,0.50);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,10);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,6);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(0.6,3.5);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;

    
    std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt");
    outputParsData.clear();
    outputParsData << "index,nrLayers,nrNodes,oddsSet1,edgeDegree1,edgeDegree2,edgeDegreeCross\n";

    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion\n";
    }

    while(true){
        GraphParametersDegreeGA pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index+seed);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
            outputParsData << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;
            std::cout << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << pars.nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}


void generateAndHeuristicAlgorithmsContinue2(unsigned int seed,std::string folderName, int index){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 8);
    std::uniform_int_distribution<int> nrNodesRand(200, 1600);


    std::uniform_real_distribution<float> oddsSet1Rand(0.15,0.30);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,6);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,3);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(0.6,2);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;

    
    std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt");
    // outputParsData.clear();
    outputParsData << "index,nrLayers,nrNodes,oddsSet1,edgeDegree1,edgeDegree2,edgeDegreeCross"<<std::endl;

    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        // output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }

    while(true){
        GraphParametersDegreeGA pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index+seed);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
            outputParsData << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;
            std::cout << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << pars.nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}




void generateAndHeuristicAlgorithmsOnlyFast(unsigned int seed,std::string folderName, int index){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 10);
    std::uniform_int_distribution<int> nrNodesRand(100, 2500);


    std::uniform_real_distribution<float> oddsSet1Rand(0.30,0.50);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,10);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,6);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(0.6,3.5);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, 
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", 
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;

    
    std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt");
    outputParsData.clear();
    outputParsData << "index,nrLayers,nrNodes,oddsSet1,edgeDegree1,edgeDegree2,edgeDegreeCross"<<std::endl;;

    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }

    while(true){
        GraphParametersDegreeGA pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index+seed);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
            outputParsData << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;
            std::cout << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << pars.nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}

void generateAndHeuristicAlgorithmsVariableEdges(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(6, 6);
    std::uniform_int_distribution<int> nrNodesRand(800, 800);


    std::uniform_real_distribution<float> oddsSet1Rand(0.25);
    std::uniform_real_distribution<float> edgeDegree1Rand(1,12);
    std::uniform_real_distribution<float> edgeDegree2Rand(1,9);
    std::uniform_real_distribution<float> edgeDegreeCrossRand(0.6,4);

    // float maxEdges = (5.0f/6.0f)*900*13; //Estimate for total number of edges

    std::uniform_int_distribution<int> goalEdges = std::uniform_int_distribution<int>(1000,10000);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;


    std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt");
    outputParsData.clear();
    outputParsData << "index,nrLayers,nrNodes,oddsSet1,edgeDegree1,edgeDegree2,edgeDegreeCross"<<std::endl;

    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }
    
    int index = 0;

    while(true){
        GraphParametersDegreeGA pars;
        pars.nrLayers = nrLayersRand(gen);
        pars.nrNodes = nrNodesRand(gen);
        pars.oddsSet1 = oddsSet1Rand(gen);
        pars.edgeDegree1 = edgeDegree1Rand(gen);
        pars.edgeDegree2 = edgeDegree2Rand(gen);
        pars.edgeDegreeCross = edgeDegreeCrossRand(gen);
        float expectedEdges = (((float)pars.nrLayers-1)/pars.nrLayers)*pars.nrNodes*(0.25*pars.edgeDegree1+0.75*pars.edgeDegree2 + pars.edgeDegreeCross);
        float degreeScalingFactor = goalEdges(gen)/expectedEdges;
        pars.edgeDegree1 *= degreeScalingFactor;
        pars.edgeDegree2 *= degreeScalingFactor;
        pars.edgeDegreeCross *= degreeScalingFactor;

        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateDoubleGraphDegree(pars.nrNodes,pars.nrLayers, minLayerRatio, pars.oddsSet1,pars.edgeDegree1,pars.edgeDegree2,pars.edgeDegreeCross,weightFunction1,weightFunction2,index+seed);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            std::ofstream outputParsData(outputFolder + "/" + "parsData" + ".txt",std::ios::app);
            outputParsData << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;
            std::cout << index << ',' << pars.nrLayers << ',' << pars.nrNodes << ',' << pars.oddsSet1 << ',' << pars.edgeDegree1 << ',' << pars.edgeDegree2<< ',' << pars.edgeDegreeCross<< std::endl;

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << pars.nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}


void generateAndHeuristicAlgorithmsVariableEdgesNew(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    int nrLayers = 5;
    int nrNodes = 600;


    float oddsSet1 = 0.25;


    std::uniform_int_distribution<int> goalEdges = std::uniform_int_distribution<int>(500,6000);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;



    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }
    
    int index = 0;

    while(true){

        int nrEdges = goalEdges(gen);

        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateGraphDoubleFixedEdges(nrNodes, nrLayers, minLayerRatio, nrEdges, oddsSet1,weightFunction1,weightFunction2, seed+index);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << nrLayers << ',' << nrNodes << ',' << nrEdges << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}

void generateAndHeuristicAlgorithmsVariableNodes(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    int nrLayers = 5;
    int nrEdges = 2500;


    float oddsSet1 = 0.25;


    std::uniform_int_distribution<int> goalNodes = std::uniform_int_distribution<int>(200,2000);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;



    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }
    
    int index = 0;

    while(true){

        int nrNodes = goalNodes(gen);

        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateGraphDoubleFixedEdges(nrNodes, nrLayers, minLayerRatio, nrEdges, oddsSet1,weightFunction1,weightFunction2, seed+index);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << nrLayers << ',' << nrNodes << ',' << nrEdges << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}



void generateAndHeuristicAlgorithmsVariableLayers(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    int nrEdges = 3000;
    int nrNodes = 450;


    float oddsSet1 = 0.25;


    std::uniform_int_distribution<int> layersRand = std::uniform_int_distribution<int>(3,25);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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

    

    std::vector<std::function<Algorithms::Solution(const MultipartiteSetGraph& graph)>>  heuristicAlgorithms = {
        Algorithms::greedySinglePathsUni, Algorithms::greedySinglePathsBi,
        Algorithms::greedyPeelingUni, Algorithms::greedyPeelingBi, Algorithms::baseLinePickEverythingUni, Algorithms::baseLinePickEverythingBi,
    Algorithms::dynSinglePathUni, Algorithms::dynSinglePathBi};

    std::vector<std::string> heuristicAlgorithmNames = { "greedySinglePathsUni", "greedySinglePathsBi",
        "greedyPeelingUni","greedyPeelingBi","baselineEverythingUni","baselineEverythingBi",
    "dynSinglePathsUni","dynSinglePathsBi"};

    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;



    for(int j=0; j<heuristicAlgorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }
    
    int index = 0;

    while(true){

        int nrLayers = layersRand(gen);

        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateGraphDoubleFixedEdges(nrNodes, nrLayers, minLayerRatio, nrEdges, oddsSet1,weightFunction1,weightFunction2, seed+index);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            

            for(int j=0; j<heuristicAlgorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = heuristicAlgorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + heuristicAlgorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << nrLayers << ',' << nrNodes << ',' << nrEdges << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}


void generateAllAlgorithmsFastNrEdges(unsigned int seed,std::string folderName){

    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> nrLayersRand(3, 10);
    std::uniform_int_distribution<int> nrNodesRand(100, 300);


    std::uniform_real_distribution<float> oddsSet1Rand(0.15,0.25);

    std::uniform_int_distribution<int> edgesRand = std::uniform_int_distribution<int>(200,1500);

    float minLayerRatio = 0.80;
    int graphsPerPars = 5;

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


    std::string graphsFolder = "data/graphs/"+folderName;
    std::string outputFolder = "data/results/"+folderName;

    
    for(int j=0; j<algorithms.size(); j++){
        std::ofstream output(outputFolder + "/" + algorithmNames[j] + ".txt");
        output.clear();
        output << "index,nrLayers,nrNodes,nrEdges,objectiveValue,timeElapsed,solvedToCompletion"<<std::endl;
    }

    int index = 0;

    while(true){
        int nrLayers = nrLayersRand(gen);
        int nrNodes = nrNodesRand(gen);
        float oddsSet1 = oddsSet1Rand(gen);
        int nrEdges = edgesRand(gen);
        for(int i=0; i<graphsPerPars; i++){
            MultipartiteSetGraph graph = generateGraphDoubleFixedEdges(nrNodes, nrLayers, minLayerRatio, nrEdges, oddsSet1,weightFunction1,weightFunction2, seed+index);

            storeGraph(graph,index+seed,graphsFolder + "/graph"+ std::to_string(index) + ".txt");
            
            for(int j=0; j<algorithms.size(); j++){
                auto start = std::chrono::high_resolution_clock::now();  
                Algorithms::Solution sol = algorithms[j](graph);
                auto end = std::chrono::high_resolution_clock::now();  
                std::chrono::duration<double> elapsed = end - start;
                std::ofstream output(outputFolder + "/" + algorithmNames[j] + ".txt",std::ios::app);
                output << index << ',' << graph.layers << ',' << nrNodes << ',' << graph.edges.size() << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
            }


            index += 1;
        }
    }
}

//This code is copied from chatgpt btw
std::vector<std::string> split_line(const std::string& line, char delimiter = ',') {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


void finishUnterminated(std::string folderName,std::string fileName){
    std::string graphFolder = "data/graphs/" + folderName + "/";

    std::string newFileName = fileName;
    newFileName.erase(newFileName.size()-4);
    newFileName = newFileName + "Fixed.txt";

    std::ifstream results("data/results/"+folderName + "/" + fileName);
    std::ofstream newResults("data/results/"+folderName + "/"+ newFileName);



    std::string line;
    int index=0;
    std::getline(results,line);
    newResults << line << std::endl;
    while(std::getline(results, line) ){
        std::cout << index << std::endl;
        std::vector<std::string> tokens = split_line(line, ',');
        if( tokens[6]=="1"){
            newResults << line << std::endl;
        } else {
            MultipartiteSetGraph graph = readGraph(graphFolder + "graph" + std::to_string(index) + ".txt");
            Algorithms::Solution sol;
            auto start = std::chrono::high_resolution_clock::now();  
            if(fileName=="altGurobiUni.txt"){
                sol = Algorithms::altGurobiUni(graph);
            } else if(fileName == "altGurobiBi.txt"){
                sol = Algorithms::altGurobiBi(graph);
            }
            auto end = std::chrono::high_resolution_clock::now();  
            std::chrono::duration<double> elapsed = end - start;

            newResults << tokens[0] << ',' << tokens[1] << ',' << tokens[2] << ',' << tokens[3] << ',' <<sol.objectiveValue << ',' << elapsed.count() << ',' << (sol.solvedToCompletion?"1":"0") << std::endl;
        }
        index += 1;
    }

    
}