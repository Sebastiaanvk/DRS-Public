#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <utility>
#include <list>
#include <chrono>
#include <vector>
#include <random>
#include <Python.h> 
#include <string>

#include "../include/mpSetGraph.h"
#include "../include/SFMLGraphs.h"
#include "../include/singlePathDyn.h"
#include "../include/mpGraphInterface.h"
#include "../include/gurobiModelWrapper.h"
#include "../include/greedyWithDynPaths.h"
#include "../include/matplotlibcpp.h"
//#include "../include/comparingAlgorithms.h"
#include "../include/graphGeneration.h"

struct RandomInput{
    int nr_layers;
    std::map<int,int> elementsPerLayer;
    int max_weight;
    float probZeroValue;
    float edge_density; 
    unsigned int seed;
};

MultipartiteSetGraph defaultGraph(){
    unsigned int seed =18;

    int nr_layers = 6; 
    int layer_size = 6;
    int max_weight = 999;
    float probZeroValue = 0.5;
    float edge_density = 0.4;

    std::vector<int> eltsLayer = {8,7,6,7,8,9};

    std::map<int,int> elementsPerLayer;
    for(int i=0; i<nr_layers; i++){

//        elementsPerLayer[i] = layer_size;
        elementsPerLayer[i] = eltsLayer[i];
    }

    MultipartiteSetGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,probZeroValue,edge_density,seed);

    return G;
}

MultipartiteSetGraph messedUpGraph(){
    unsigned int seed =1324761701;

    int nr_layers = 7; 
    int max_weight = 999;
    float probZeroValue = 0.0685562;
    float edge_density = 0.122815;

    std::vector<int> eltsLayer = {7, 11, 7, 16, 15, 10, 16};

    std::map<int,int> elementsPerLayer;
    for(int i=0; i<nr_layers; i++){

        elementsPerLayer[i] = eltsLayer[i];
    }
//    MultipartiteSetGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight, probZeroValue, edge_density,seed);

    MultipartiteSetGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,edge_density,seed);

    return G;
}



bool checkSolution(const MPGraphInterface& mpgraph, const GreedyWithDynPathsSol& sol, bool twoDir){

    std::unique_ptr<EdgeIterator> it = mpgraph.getEdgeIterator();
    int nr_layers = mpgraph.getLayers();
    float sum_of_weights = 0;

    std::set<VertexIndex> hasForwardPath;
    std::set<VertexIndex> hasBackwardPath;
    
    for(auto vi : sol.picked_nodes){
        sum_of_weights += mpgraph.getValue(vi);
    }

    std::set<VertexIndex> neighbor_set;
    neighbor_set.insert(sol.picked_nodes.begin(),sol.picked_nodes.end());

    while(it->has_next()){
        Edge e = it->current();
        VertexIndex vi1 = e.first;
        VertexIndex vi2 = e.second;
        if(sol.picked_nodes.count(vi1) && sol.picked_nodes.count(vi2)){
            hasForwardPath.insert(vi1);
            hasBackwardPath.insert(vi2);
        } else if(sol.picked_nodes.count(vi1) || sol.picked_nodes.count(vi2)){
            VertexIndex node = vi1;
            VertexIndex nb = vi2;
            if(sol.picked_nodes.count(vi2)){
                node = vi2;
                nb = vi1;
            }

            if(node.layer>nb.layer || twoDir){
               neighbor_set.insert(nb);
            } 
        } 
        it->next();
    }
    for(auto vi:sol.picked_nodes){
        if(vi.layer<nr_layers-1 && hasForwardPath.count(vi)==0){
            std::cout << "Element at (" << vi.layer << ", " << vi.element << ") has no forward path!" << std::endl;
            return false;
        }
        if(vi.layer>0 && hasBackwardPath.count(vi)==0){
            std::cout << "Element at (" << vi.layer << ", " << vi.element << ") has no backward path!" << std::endl;
            return false;
        }
    }

    for(VertexIndex nb:neighbor_set){
        if(!sol.picked_nbs.count(nb)){
            std::cout << "The neighbor (" << nb.layer << ", " << nb.element << ") is not in the neighbor set!" << std::endl;
//              return false;
        }
    }


    if(neighbor_set!=sol.picked_nbs){
        std::cout << "The neighbor set is incorrect!" << std::endl;
        return false;
    }
    std::cout << "The node and neighbor set appear to be correct. The given value is: " << sol.value; 
//    std::cout << " and recalculated value is: " << sum_of_weights/neighbor_set.size() << " with sum of weights:" << sum_of_weights << " and nr nbs:" << neighbor_set.size();
    std::cout << std::endl;
    return true;
}


RandomInput generate_input_par(){
    int min_layers = 5;
    int max_layers = 10;
    int min_elements = 5;
    int max_elements = 15;
    RandomInput ri;
    ri.nr_layers = rand()%(max_layers+1-min_layers)+min_layers;
    for(int i=0; i<ri.nr_layers; i++){
        ri.elementsPerLayer[i] = rand()%(max_elements+1-min_elements)+min_elements;
    }
    ri.max_weight = 999;
    ri.edge_density = static_cast<float>(rand())/RAND_MAX;
    ri.probZeroValue = static_cast<float>(rand())/RAND_MAX;
    ri.seed = rand();
    return ri;
}

void print_graph_input(const RandomInput& ri){
    std::cout << "For the graph generated with parameters: " <<"\nnr_layers:" <<  ri.nr_layers << "\nelements_per_layer: " << ri.elementsPerLayer.at(0);
    for(int i=1; i<ri.nr_layers; i++){
        std::cout << ", "<< ri.elementsPerLayer.at(i); 
    }
    std::cout << "\nmax_weight: "<< ri.max_weight<< "\nprobZeroValue: "<< ri.probZeroValue<< "\nedge_density: "<< ri.edge_density<< "\nseed: "<< ri.seed << std::endl;
}




void testCompareTwoAlgorithms1Dir(int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<double> gurobiValues(nr_comparisons);
    std::vector<double> greedyValues(nr_comparisons);

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight, ri.edge_density, ri.seed);

        GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
        modelWrapper.addBackwardNeighborConstraints();
//        modelWrapper.addForwardNeighborConstraints();
        modelWrapper.addPathConstraints();
        modelWrapper.addStandardObjectiveFunction();
        double gurobiOptimum = modelWrapper.optimize();
        std::cout<< "Gurobi optimum: " <<  gurobiOptimum << std::endl;

        GreedyWithDynPathsSol solution = greedyDynPaths1dir(G, false);

        gurobiValues[i] = gurobiOptimum;
        std::cout << gurobiValues[i] << std::endl;
        greedyValues[i] = solution.value;

//        print_graph_input(ri);
//        std::cout << "The gurobi optimal value is: " << gurobiOptimum << std::endl;
//        std::cout <<"The greedy solution value is: " << solution.value << std::endl;

    }

    std::cout << "Ended the optimization:\n";
    std::vector<double> ratios(nr_comparisons);
    for (int i=0; i<nr_comparisons; i++){
        std::cout << gurobiValues[i] << std::endl;
        if(gurobiValues[i]<=0.00001){
            ratios[i] = 0;
        } else {
            ratios[i] = greedyValues[i]/gurobiValues[i];
        }
    }

    std::vector<double> x_axis(ratios.size());
    for(int i=0; i<nr_comparisons; i++){
        std::cout << ratios[i] << std::endl;
        x_axis[i] = i;
    }

    matplotlibcpp::plot(x_axis, ratios);
    matplotlibcpp::show();
}

/*
void dummyProcessGraph(const MPGraphInterface& mpGraph){

    std::vector<std::vector<std::vector<int>>> forwardEdges;

    std::vector<std::vector<std::vector<int>>> backwardEdges;
    
    for(int layer=0; layer<mpGraph.getLayers(); layer++){
        int eltIntLayer = mpGraph.getElementsInLayer(layer);
        forwardEdges[layer] = std::vector<std::vector<int>>(eltIntLayer);
        backwardEdges[layer] = std::vector<std::vector<int>>(eltIntLayer);
    }
    std::unique_ptr<EdgeIterator> it = mpGraph.getEdgeIterator();
    while(it->has_next()){
        Edge e = it->current();
        VertexIndex vi1 = e.first;
        VertexIndex vi2 = e.second;

        forwardEdges[e.first]

        it->next();

    }




}
*/

void timeToCopyGraphvsAlgorithm(int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<std::chrono::duration<double>> graphProcessingTimes;

    std::vector<std::chrono::duration<double>> algorithmTimes;

    for(int i=0; i<nr_comparisons; i++){

        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight, ri.edge_density, ri.seed);




    }

    std::cout << "Ended the optimization:\n";
    for (int i=0; i<nr_comparisons; i++){
        std::cout << "For iteration " << i << ":\n";
        std::cout << "ProcessingTimes:" << graphProcessingTimes[i].count() << std::endl;
        std::cout << "Algorithm times: " << algorithmTimes[i].count() << std::endl;
    }
}


void testPartition(int nr_iterations,unsigned int seed){
    int n = 100;
    int k = 5;

    std::mt19937 gen(seed);

    for(int i=0; i<nr_iterations; i++){
        std::vector<int> parts = generatePartition(n,k,gen);
        for(int j=0; j<parts.size(); j++){
            std::cout << parts[j] << ", ";
        }
        std::cout << std::endl;
    } 

    std::map<int,int> freqs;

    for(int i=0; i<100000; i++){
        std::vector<int> parts = generatePartitionMinSize(20,3,5,gen);
        /*
        for(int j=0; j<parts.size(); j++){
            std::cout << parts[j] << ", ";
        }
        */
       freqs[10000*parts[0]+100*parts[1]+parts[2]] += 1; 
    } 

    for(auto key:freqs){
        std::cout << key.first << ": " << key.second << std::endl;
    }
}

void testGraphGeneration(int nrIterations, unsigned int seed){
    std::mt19937 gen(seed);    

    for(int i=0; i<nrIterations; i++){
        MultipartiteSetGraph graph = generateGraph(200, 10, 5, 1000, uniform_distribution_default,gen);
//        displayGraphPNG(graph, 1000, 800, "generated_graph_"+ std::to_string(i));
    }
}


int main(){
//     testCompareTwoAlgorithms1Dir(10, 10);
//    timeToCopyGraphvsAlgorithm(10,10);
//    testPartition(100,0);
    testGraphGeneration(1000,0);
    return 0;
}