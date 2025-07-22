#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "graphs.h"
#include "modelWrapper.h"
#include "SFMLGraphs.h"


MultipartiteGraph defaultGraph(){
    unsigned int seed =12;

    int nr_layers = 8;
    int layer_size = 12;
    int max_weight = 999;
    float probZeroValue = 0.5;
    float edge_density = 0.2;


    std::map<int,int> elementsPerLayer;

    for(int i=0; i<nr_layers; i++){
        elementsPerLayer[i] = layer_size;
    }

    MultipartiteGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,probZeroValue,edge_density,seed);

    return G;
}


void testWrapper(const MultipartiteGraph& G){
    int width = 1000;
    int height = 800;

    display_graph(G);
    displayGraphPNG(G,width,height, "initialGraph");

    ModelWrapper modelWrapper(G, false, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addStandardObjectiveFunction();
    double optimum = modelWrapper.optimize();

    modelWrapper.printVariables();

    std::cout << "The optimal value is: " << optimum << std::endl;

    displayGraphPNGPostOpt(G, width, height, modelWrapper, "GraphPostOpt");

}



void compareAddingForwardConstraints(const MultipartiteGraph& G,bool relaxed){
    int width = 1000;
    int height = 800;

//    display_graph(G);
    displayGraphPNG(G,width,height, "initialGraph");

    ModelWrapper modelWrapper(G, relaxed, initialEnvironment());
    modelWrapper.addBackwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addStandardObjectiveFunction();
    double optimum = modelWrapper.optimize();

    ModelWrapper modelWrapperWithForward(G, relaxed, initialEnvironment());
    modelWrapperWithForward.addBackwardNeighborConstraints();
    modelWrapperWithForward.addForwardNeighborConstraints();
    modelWrapperWithForward.addPathConstraints();
    modelWrapperWithForward.addStandardObjectiveFunction();
    double optimumFoward = modelWrapperWithForward.optimize();

    std::cout << "The optimal value is: " << optimum << std::endl;
    std::cout << "The optimal value with forward neighbor constraints: " << optimumFoward << std::endl;

    displayGraphPNGPostOpt(G, width, height, modelWrapper, "GraphPostOpt");
    displayGraphPNGPostOpt(G, width, height, modelWrapperWithForward, "GraphPostOptWithForward" );


}



int main(){
    MultipartiteGraph G = defaultGraph(); 

    compareAddingForwardConstraints(G,false);
//    testWrapper(G);
    return 0;
}