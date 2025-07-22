#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "../include/mpSetGraph.h"
#include "../include/gurobiModelWrapper.h"
#include "../include/SFMLGraphs.h"


MultipartiteSetGraph defaultGraph(){
    unsigned int seed =10;

    int nr_layers = 8;
    int layer_size = 12;
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


void testWrapper(const MultipartiteSetGraph& G){
    int width = 1000;
    int height = 800;

    display_graph(G);
    displayGraphPNG(G,width,height, "initialGraph");

    GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addStandardObjectiveFunction();
    double optimum = modelWrapper.optimize();

    modelWrapper.printVariables();

    std::cout << "The optimal value is: " << optimum << std::endl;

    displayGraphPNGPostOpt(G, width, height, &modelWrapper, "GraphPostOpt");

}



void compareAddingForwardConstraints(const MultipartiteSetGraph& G,bool relaxed){
    int width = 1000;
    int height = 800;

//    display_graph(G);
    displayGraphPNG(G,width,height, "initialGraph");

    GurobiModelWrapper modelWrapper(G, relaxed, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addStandardObjectiveFunction();
    double optimum = modelWrapper.optimize();

    GurobiModelWrapper modelWrapperWithForward(G, relaxed, initialEnvironment());
    modelWrapperWithForward.addBackwardNeighborConstraints();
    modelWrapperWithForward.addForwardNeighborConstraints();
    modelWrapperWithForward.addPathConstraints();
    modelWrapperWithForward.addStandardObjectiveFunction();
    double optimumFoward = modelWrapperWithForward.optimize();

    std::cout << "The optimal value is: " << optimum << std::endl;
    std::cout << "The optimal value with forward neighbor constraints: " << optimumFoward << std::endl;

    displayGraphPNGPostOpt(G, width, height, &modelWrapper, "GraphPostOpt");
    displayGraphPNGPostOpt(G, width, height, &modelWrapperWithForward, "GraphPostOptWithForward" );


}




int main(){
    MultipartiteSetGraph G = defaultGraph(); 

    compareAddingForwardConstraints(G,false);
//    testWrapper(G);
    return 0;
}