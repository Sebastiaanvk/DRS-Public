#include "../include/mpSetGraph.h"


void mpSetEdgeIterator::next(){
    it++;
} 

bool mpSetEdgeIterator::has_next() const{
    return it!=edgesRef.end();
}

std::pair<VertexIndex,VertexIndex> mpSetEdgeIterator::current() const{
    return *it;
} 


int MultipartiteSetGraph::getLayers() const {
    return layers;
}

int MultipartiteSetGraph::getElementsInLayer(int layer) const {
    return elementsPerLayer.at(layer);
}
float MultipartiteSetGraph::getValue(const VertexIndex& v) const {
//I added this to this mpSetGraph version only!!!!!!!!!!!!!!!!!!!
    if(values.count(v)==0){
        return 0;
    }
    return values.at(v);
}

MultipartiteSetGraph& MultipartiteSetGraph::operator=(const MultipartiteSetGraph& other){
    if (this != &other) {
        layers = other.layers;
        elementsPerLayer = other.elementsPerLayer;
        edges = other.edges;
        values = other.values;
    }
    return *this;
}


MultipartiteSetGraph generate_random_graph(int nr_layers, std::map<int,int> elementsPerLayer,  int max_weight, float edge_density, unsigned int seed){
    srand(seed);
    MultipartiteSetGraph G;
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

MultipartiteSetGraph generate_random_graph(int nr_layers, std::map<int,int> elementsPerLayer,  int max_weight, float probZeroValue, float edge_density, unsigned int seed){
    srand(seed);
    MultipartiteSetGraph G;
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
            if(static_cast<double>(rand())/ RAND_MAX<probZeroValue){
                G.values[{i,j}] = 0;
            } else {
                G.values[{i,j}] = (rand()%(max_weight+1));
            }
        }
    }

    return G;
}

void display_graph(const MultipartiteSetGraph& G){
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
