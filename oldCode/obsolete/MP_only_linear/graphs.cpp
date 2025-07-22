#include "graphs.h"

bool operator<(const VertexIndex& v1, const VertexIndex& v2){
    if(v1.layer==v2.layer)
        return v1.element<v2.element;
    return v1.layer<v2.layer;
}

bool operator==(const VertexIndex& v1, const VertexIndex& v2){
    return v1.layer==v2.layer && v1.element==v2.element;
}

MultipartiteGraph generate_random_graph(int nr_layers, std::map<int,int> elementsPerLayer,  int max_weight, float edge_density, unsigned int seed){
    srand(seed);
    MultipartiteGraph G;
    G.layers = nr_layers;
    G.elementsPerLayer = elementsPerLayer;
    for( int i=0; i<nr_layers-1; i++){
        for( int j=0; j<elementsPerLayer.at(i); j++){
            for( int k=0; k<elementsPerLayer.at(i+1); k++){
                if(rand()/(RAND_MAX +1.0)<=edge_density){
                    G.edges.insert({{i,j},{i+1,k}});
                }
            }
        }
    }

    for(int i=1; i<nr_layers; i++){
        for(int j=0; j<elementsPerLayer[i]; j++){
            G.values[{i,j}] = (rand()%(max_weight+1));
        }
    }

    return G;
}


void display_graph(const MultipartiteGraph& G){
    std::cout << "Number of layers: " << G.layers << std::endl;
    std::cout << "Values, starting with layer 1:\n";
    for(int layer=1; layer<G.layers; layer++){
        std::cout << "Layer " << layer << ":\n";
        for(int j=0; j<G.elementsPerLayer.at(layer); j++){
            std::cout << G.values.at({layer,j}) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Edges:\n";
    for(auto& e : G.edges){
        VertexIndex v1 = e.first;
        VertexIndex v2 = e.second;
        printf("(%d,%d)->(%d,%d)\n",v1.layer,v1.element,v2.layer,v2.element);
    }
}
