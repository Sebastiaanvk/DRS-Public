#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "graphs.h"
#include "optimization.h"

void normal_vs_relaxed(const MultipartiteGraph& G){
    double normal_opt = find_optimum_quadratic(G,false,true,true);
    double relaxed_opt = find_optimum_quadratic(G,true,true,true);
    std::cout << "Comparing optimum:\n";
    std::cout << "Normal optimal value: " << normal_opt << std::endl;
    std::cout << "Relaxed optimal value: " << relaxed_opt << std::endl;

}

void normal_vs_relaxed_no_path_constraints(const MultipartiteGraph& G ){
    double normal_opt = find_optimum_quadratic(G,false,false,true);
    double relaxed_opt = find_optimum_quadratic(G,true,false,true);
    std::cout << "Comparing optimum without path constraints\n";
    std::cout << "Normal optimal value: " << normal_opt << std::endl;
    std::cout << "Relaxed optimal value: " << relaxed_opt << std::endl;
}

void quadratic_vs_linear(const MultipartiteGraph& G, bool relaxed ){
    double normal_opt = find_optimum_quadratic(G,relaxed,true,true);
    double linearized_opt = find_optimum_linearized(G,relaxed,true,true);
    std::cout << "Comparing optimum quadratic and linearized\n";
    std::cout << "Normal optimal value: " << normal_opt << std::endl;
    std::cout << "Linearized optimal value: " << linearized_opt << std::endl;
}

void linear_vs_relaxed_vs_quadratic_relaxed(const MultipartiteGraph& G, bool print_variables){
    double linear_opt = find_optimum_linearized(G,false,true,print_variables);
    double linear_relaxed = find_optimum_linearized(G,true, true, print_variables);
    double quadr_relaxed = find_optimum_quadratic(G, true, true, print_variables);
    std::cout << "Linearized optimal value: " << linear_opt << std::endl;
    std::cout << "Linearized relaxed:" << linear_relaxed << std::endl;
    std::cout << "Quadr relaxed:" << quadr_relaxed << std::endl;

}



int main(){
    unsigned int seed =10;

    int nr_layers = 6;
    int layer_size = 20;
    int max_weight = 5;
    float edge_density = 0.3;


    std::map<int,int> elementsPerLayer;

    for(int i=0; i<nr_layers; i++){
        elementsPerLayer[i] = layer_size;
    }

    MultipartiteGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,edge_density,seed);

    display_graph(G);

//    normal_vs_relaxed_no_path_constraints(G);

//    normal_vs_relaxed(G);

    quadratic_vs_linear(G,false);
//    linear_vs_relaxed_vs_quadratic_relaxed(G,true);

    return 0;
}
