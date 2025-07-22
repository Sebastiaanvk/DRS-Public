#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "../include/mpSetGraph.h"
#include "../include/SFMLGraphs.h"
#include "../include/greedyHNSNPathDirected.h"
#include "../include/gurobiModelWrapper.h"

struct RandomInput{
    int nr_layers;
    std::map<int,int> elementsPerLayer;
    int max_weight;
    float probZeroValue;
    float edge_density; 
    int seed;
};

MultipartiteSetGraph defaultGraph(){
    unsigned int seed =14;

    int nr_layers = 10;
    int layer_size = 10;
    int max_weight = 999;
    float probZeroValue = 0.5;
    float edge_density = 0.2;


    std::map<int,int> elementsPerLayer;

    for(int i=0; i<nr_layers; i++){
        elementsPerLayer[i] = layer_size;
    }

    MultipartiteSetGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,probZeroValue,edge_density,seed);

    return G;
}


RandomInput generate_input_par(){
    int min_layers = 5;
    int max_layers = 15;
    int min_elements = 5;
    int max_elements = 20;
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

void testCompareGurobiGreedy(int nr_comparisons, unsigned int seed){
    srand(seed);

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight, ri.edge_density, ri.seed);

        GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
        modelWrapper.addBackwardNeighborConstraints();
        modelWrapper.addForwardNeighborConstraints();
        modelWrapper.addPathConstraints();
        modelWrapper.addStandardObjectiveFunction();
        double gurobiOptimum = modelWrapper.optimize();

        MPGraphAsGreedyGraph greedyGraph = mpGraphToGreedyGraph(&G, true);
        std::vector<float> acc_neigbor_values;
        GreedySolution solution = greedyAlgorithmPrintIterations(greedyGraph.greedyGraph, acc_neigbor_values, G, false);

        print_graph_input(ri);
        std::cout << "The gurobi optimal value is: " << gurobiOptimum << std::endl;
        std::cout <<"The greedy solution value is: " << solution.value << std::endl;
    }
}

int main(){

    testCompareGurobiGreedy(20, 0);    

    return 0;
}