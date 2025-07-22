#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "../include/mpSetGraph.h"
#include "../include/SFMLGraphs.h"
#include "../include/greedyHNSNPathDirected.h"

MultipartiteSetGraph defaultGraph(){
    unsigned int seed =21;

    int nr_layers = 5;
    int layer_size = 7;
    int max_weight = 999;
    float probZeroValue = 0.2;
    float edge_density = 0.2;


    std::map<int,int> elementsPerLayer;

    for(int i=0; i<nr_layers; i++){
        elementsPerLayer[i] = layer_size;
    }

    MultipartiteSetGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,probZeroValue,edge_density,seed);

    return G;
}

void testMPGraphToGreedy(){
    MultipartiteSetGraph mpGraph = defaultGraph();
    displayGraphPNG( mpGraph, 800,600,"testImage" );

    MPGraphAsGreedyGraph G = mpGraphToGreedyGraph( &mpGraph, true);

    for(int i=0; i<G.greedyGraph.nr_nodes; i++){
        std::cout << "Value node:" << G.greedyGraph.node_values[i] << std::endl;
        std::cout << "Forward arcs: ";

        for(int x : G.greedyGraph.forward_arcs[i]){
            std:: cout << x << " ";
        }
        std::cout << std::endl;
    }


    std::list<int> unreachables = find_unreachables(G.greedyGraph);
    std::cout << "Unreachables:\n";
    for(int i : unreachables){
        std::cout << "layer: " << G.intToVertexIndex[i].layer << " elt:" << G.intToVertexIndex[i].element << std::endl;
    }


}

std::string indexToLayElt(Vertex x, const MPGraphAsGreedyGraph& G){
    VertexIndex vi = G.intToVertexIndex[x];
    return "Layer: " + std::to_string(vi.layer) + " elt: " + std::to_string(vi.element);
}

void testGreedyInitialization(bool print_unreachables, bool print_dependencies, bool print_nb_values){
    MultipartiteSetGraph mpGraph = defaultGraph();
    displayGraphPNG(mpGraph, 800, 600, "testImage");

    MPGraphAsGreedyGraph G = mpGraphToGreedyGraph(&mpGraph, true);

    if(print_unreachables){
        std::list<Vertex> unreachables = find_unreachables(G.greedyGraph);
        std::cout << "Unreachables:\n";
        for(Vertex i : unreachables){
            std::cout << "layer: " << G.intToVertexIndex[i].layer << " elt:" << G.intToVertexIndex[i].element << std::endl;
        }
    }

    GreedyAlgorithmVariables vars = greedyInitialize(G.greedyGraph);

    if(print_dependencies){
        std::cout << "\nLooking at dependencies:\n";
        for(Vertex x : vars.current_node_set){
            if(vars.node_implications[x].size()>1){
                std::cout << indexToLayElt(x, G) << std::endl; 
                std::cout << "Dependencies: ";
                bool first = true;
                for( Vertex y : vars.node_implications[x]){
                    if(!first){
                        std::cout << ", ";
                    }
                    std::cout << indexToLayElt(y, G);
                    first = false;
                }
                std::cout << std::endl;
            }
        }
    }
    if(print_nb_values){
        for(Vertex nb:vars.current_nb_set){
            std::cout << indexToLayElt(nb, G) << ": " << vars.nb_values_map[nb] << std::endl;
            std::cout << "With neighbors:";
            bool first = true;
            for( Vertex x : G.greedyGraph.nb_to_nodes[nb]){
                if(!first){
                    std::cout << ", ";
                }
                std::cout << indexToLayElt(x, G);
                first = false;
            }
            std::cout << std::endl;

        }
    }
    
}

int main(){

//    testMPGraphToGreedy();
    testGreedyInitialization(false,false,true);
    return 0;
}