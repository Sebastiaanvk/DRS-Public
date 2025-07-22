#include <iostream>
#include <set>
#include <map>
#include <utility>
#include <list>
#include <chrono>
#include "../include/mpSetGraph.h"
#include "../include/SFMLGraphs.h"
#include "../include/singlePathDyn.h"
#include "../include/mpGraphInterface.h"
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
    unsigned int seed =18;

    int nr_layers = 30; 
    int layer_size = 30;
    int max_weight = 999;
    float probZeroValue = 0.5;
    float edge_density = 0.4;


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

void test_single_path_dyn(){
    MultipartiteSetGraph G = defaultGraph();
    SinglePathDynSol sol = singlePathDynHeur(G);
    std::cout << "The solution value is: "<< sol.max_value << std::endl;
    displayGraphPNGNodesNbsPicked(G, 1000,800, sol.picked_nodes, sol.picked_neighbors, "PickedPath");
}

void test_gurobi_single_path(){
    MultipartiteSetGraph G = defaultGraph();
    GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addForwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addSinglePathConstraint();
    modelWrapper.addStandardObjectiveFunction();
    double gurobiOptimum = modelWrapper.optimize();
    std::cout << "The gurobi solution value is: "<< gurobiOptimum << std::endl;

    displayGraphPNGPostOpt(G,1000,800,&modelWrapper, "GurobiSinglePath");
}

void test_single_path_dyn_nb_dir(){
    MultipartiteSetGraph G = defaultGraph();
    SinglePathDynSol sol = singlePathDyn1DirNbs(G);
    std::cout << "The solution value is: "<< sol.max_value << std::endl;
    displayGraphPNGNodesNbsPicked(G, 1000,800, sol.picked_nodes, sol.picked_neighbors, "PickedPath");

}


void test_dyn_1dir_vs_brute_force(){
    MultipartiteSetGraph G = defaultGraph();

    auto startDyn = std::chrono::high_resolution_clock::now();
    SinglePathDynSol dynSol = singlePathDyn1DirNbs(G);
    auto endDyn = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedDyn = endDyn-startDyn;

    auto startBf = std::chrono::high_resolution_clock::now();
    SinglePathDynSol bfSol = singlePath1DirNbsBruteForce(G);
    auto endBf = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedBf = endBf-startBf;

    std::cout << "The dynamic solution value is: "<< dynSol.max_value << " and it took " << elapsedDyn.count() << " seconds." << std::endl;
    std::cout << "The brute force solution value is: "<< bfSol.max_value << " and it took " << elapsedBf.count() << " seconds." << std::endl;

    displayGraphPNGNodesNbsPicked(G, 1000,800, dynSol.picked_nodes, dynSol.picked_neighbors, "PickedPathDyn");
    displayGraphPNGNodesNbsPicked(G, 1000,800, bfSol.picked_nodes, bfSol.picked_neighbors, "PickedPathBF");
}

void test_2dir_brute_force(){
    MultipartiteSetGraph G = defaultGraph();

    auto startBf = std::chrono::high_resolution_clock::now();
    SinglePathDynSol bfSol = singlePathBruteForce(G);
    auto endBf = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedBf = endBf-startBf;

    std::cout << "The brute force solution value is: "<< bfSol.max_value << " and it took " << elapsedBf.count() << " seconds." << std::endl;

    displayGraphPNGNodesNbsPicked(G, 1000,800, bfSol.picked_nodes, bfSol.picked_neighbors, "PickedPathBF");

}

void test_2dir_brute_force_vs_gurobi(){

// 11 layers and 11 elements per layer take a couple of seconds
// 12 layers and 12 elements per layer takes ~50 secs for the brute force and ~2 secs for gurobi.
// Seems to work, but there is a slight difference since gurobi and its current form doesnt
// pick a node from the first layer, do it wont take its right forward neighbors as neighbors.
    MultipartiteSetGraph G = defaultGraph();

    auto startBf = std::chrono::high_resolution_clock::now();
    SinglePathDynSol bfSol = singlePathBruteForce(G);
    auto endBf = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedBf = endBf-startBf;


    auto startGr = std::chrono::high_resolution_clock::now();
    GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addForwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addSinglePathConstraint();
    modelWrapper.addStandardObjectiveFunction();
    double gurobiOptimum = modelWrapper.optimize();
    auto endGr = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedGr = endGr-startGr;

    std::cout << "The brute force solution value is: "<< bfSol.max_value << " and it took " << elapsedBf.count() << " seconds." << std::endl;
    std::cout << "The gurobi solution value is: "<< gurobiOptimum << " and it took " << elapsedGr.count() << " seconds." << std::endl;

    displayGraphPNGPostOpt(G,1000,800,&modelWrapper, "GurobiSinglePath");
    displayGraphPNGNodesNbsPicked(G, 1000,800, bfSol.picked_nodes, bfSol.picked_neighbors, "PickedPathBF");

}

void test_2dir_dynamic_vs_gurobi(){

    MultipartiteSetGraph G = defaultGraph();

    auto startDyn = std::chrono::high_resolution_clock::now();
    SinglePathDynSol dynSol = singlePathDynNbs(G);
    auto endDyn = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedDyn = endDyn-startDyn;


    auto startGr = std::chrono::high_resolution_clock::now();
    GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addForwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addSinglePathConstraint();
    modelWrapper.addStandardObjectiveFunction();
    double gurobiOptimum = modelWrapper.optimize();
    auto endGr = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedGr = endGr-startGr;

    std::cout << "The dynamic solution value is: "<< dynSol.max_value << " and it took " << elapsedDyn.count() << " seconds." << std::endl;
    std::cout << "The gurobi solution value is: "<< gurobiOptimum << " and it took " << elapsedGr.count() << " seconds." << std::endl;

    displayGraphPNGPostOpt(G,1000,800,&modelWrapper, "GurobiSinglePath");
    displayGraphPNGNodesNbsPicked(G, 1000,800, dynSol.picked_nodes, dynSol.picked_neighbors, "PickedPathBF");

}

void test_dyn_2dir_vs_brute_force(){
    MultipartiteSetGraph G = defaultGraph();

    auto startDyn = std::chrono::high_resolution_clock::now();
    SinglePathDynSol dynSol = singlePathDynNbs(G);
    auto endDyn = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedDyn = endDyn-startDyn;

    auto startBf = std::chrono::high_resolution_clock::now();
    SinglePathDynSol bfSol = singlePathBruteForce(G);
    auto endBf = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedBf = endBf-startBf;

    std::cout << "The dynamic solution value is: "<< dynSol.max_value << " and it took " << elapsedDyn.count() << " seconds." << std::endl;
    std::cout << "The brute force solution value is: "<< bfSol.max_value << " and it took " << elapsedBf.count() << " seconds." << std::endl;

    displayGraphPNGNodesNbsPicked(G, 1000,800, dynSol.picked_nodes, dynSol.picked_neighbors, "PickedPathDyn");
    displayGraphPNGNodesNbsPicked(G, 1000,800, bfSol.picked_nodes, bfSol.picked_neighbors, "PickedPathBF");
}


void test_single_path_dyn_heur_vs_exact(){
    MultipartiteSetGraph G = defaultGraph();

    auto startDyn = std::chrono::high_resolution_clock::now();
    SinglePathDynSol dynSol = singlePathDynNbs(G);
    auto endDyn = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedDyn = endDyn-startDyn;

    auto startHeur = std::chrono::high_resolution_clock::now();
    SinglePathDynSol heurSol = singlePathDynHeur(G);
    auto endHeur = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedHeur = endHeur-startHeur;

    std::cout << "The dynamic solution value is: "<< dynSol.max_value << " and it took " << elapsedDyn.count() << " seconds." << std::endl;
    checkSinglePath(G,dynSol, true);
    std::cout << "The heuristic solution value is: "<< heurSol.max_value << " and it took " << elapsedHeur.count() << " seconds." << std::endl;
    checkSinglePath(G,heurSol, true);

    displayGraphPNGNodesNbsPicked(G, 1000,800, dynSol.picked_nodes, dynSol.picked_neighbors, "PickedPathDyn");
    displayGraphPNGNodesNbsPicked(G, 1000,800, heurSol.picked_nodes, heurSol.picked_neighbors, "PickedPathHeur");


}

int main(){

//    test_single_path_dyn();
//    test_gurobi_single_path();
//    test_single_path_dyn_nb_dir();
//    test_dyn_1dir_vs_brute_force();
//    test_2dir_brute_force_vs_gurobi();
//    test_2dir_dynamic_vs_gurobi();
//    test_dyn_2dir_vs_brute_force();
    test_single_path_dyn_heur_vs_exact();

    return 0;
}