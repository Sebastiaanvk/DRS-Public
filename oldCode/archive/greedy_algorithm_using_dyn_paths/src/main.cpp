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
#include "../include/greedyWithDynPaths.h"

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


void testGreedy1dirWithPrintingIterations(){
    MultipartiteSetGraph mpGraph = defaultGraph();
    displayGraphPNG(mpGraph, 800, 600, "Initial Graph");

    GreedyWithDynPathsSol solution = greedyDynPaths1dir(mpGraph, true);


    displayGraphPNGNodesNbsPicked(mpGraph, 1000, 800, solution.picked_nodes, solution.picked_nbs, "Best Solution");

    std::cout <<"Solution value: " << solution.value << std::endl;

    checkSolution(mpGraph, solution, false);
}

void testGreedy1dirInfinite(unsigned int seed){
    srand(seed);
    RandomInput ri = generate_input_par();

    print_graph_input(ri);

    MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight, ri.edge_density, ri.seed);

    GreedyWithDynPathsSol solution = greedyDynPaths1dir(G, false);

    displayGraphPNGNodesNbsPicked(G, 1000, 800, solution.picked_nodes, solution.picked_nbs, "Best Solution");
    

    std::cout <<"The greedy solution value is: " << solution.value << std::endl;

    

    checkSolution(G, solution, false);

}



void greedy1dirVsGurobi(int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<float> gurobiResults;
    std::vector<float> greedyResults;
    bool allGreedyCorrect = true;

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));


        GurobiModelWrapper modelWrapper(G, false, initialEnvironment());

        modelWrapper.turnOffOutput();
        modelWrapper.addBackwardNeighborConstraints();
//        modelWrapper.addForwardNeighborConstraints();
        modelWrapper.addPathConstraints();
        modelWrapper.addStandardObjectiveFunction();
        double gurobiOptimum = modelWrapper.optimize();


        GreedyWithDynPathsSol solution = greedyDynPaths1dir(G, false);

        displayGraphPNGPostOpt(G, 1000, 800, &modelWrapper, "gurobi solution " + std::to_string(i));


        displayGraphPNGNodesNbsPicked(G, 1000, 800, solution.picked_nodes, solution.picked_nbs, "greedy solution " + std::to_string(i) );

        std::cout << "########################################################\n";
        std::cout << "The gurobi optimal value is: " << gurobiOptimum << std::endl;
        std::cout <<"The greedy solution value is: " << solution.value << std::endl;
        allGreedyCorrect = allGreedyCorrect && (G,solution,false);
        std::cout << "########################################################\n";

        gurobiResults.push_back(gurobiOptimum);
        greedyResults.push_back(solution.value);
    }

    std::cout << "Results:\n";
    for(int round = 0; round<nr_comparisons; round++){
        std::cout << "Round "<<round<<":\n";
        std::cout << "Gurobi: " << gurobiResults[round] << std::endl;
        std::cout << "Greedy: " << greedyResults[round] << std::endl;
    }
}

void testGreedyVsGurobiMessedUp(){
    MultipartiteSetGraph G =  messedUpGraph();

    displayGraphPNG(G, 1000,800, "Initial graph");

    GurobiModelWrapper modelWrapper(G, false, initialEnvironment());
    modelWrapper.turnOffOutput();
    modelWrapper.addBackwardNeighborConstraints();
//        modelWrapper.addForwardNeighborConstraints();
    modelWrapper.addPathConstraints();
    modelWrapper.addStandardObjectiveFunction();
//    double gurobiOptimum = modelWrapper.optimize();

    GreedyWithDynPathsSol solution = greedyDynPaths1dir(G, false);


        displayGraphPNGNodesNbsPicked(G, 1000, 800, solution.picked_nodes, solution.picked_nbs, "Best Solution");

    std::cout << "########################################################\n";
//   std::cout << "The gurobi optimal value is: " << gurobiOptimum << std::endl;
    std::cout <<"The greedy solution value is: " << solution.value << std::endl;
    checkSolution(G,solution,false);
    std::cout << "########################################################\n";

}

void testGraphGeneration(){
    MultipartiteSetGraph G1 = messedUpGraph();
    displayGraphPNG(G1, 1000,800, "Initial graph 1");
    MultipartiteSetGraph G2 = messedUpGraph();
    displayGraphPNG(G2, 1000,800, "Initial graph 2");


}


void greedy2dirVsGurobi(int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<float> gurobiResults;
    std::vector<float> greedyResults;
    bool allGreedyCorrect = true;

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));


        GurobiModelWrapper modelWrapper(G, false, initialEnvironment());

        modelWrapper.turnOffOutput();
        modelWrapper.addBackwardNeighborConstraints();
        modelWrapper.addForwardNeighborConstraints();
        modelWrapper.addPathConstraints();
        modelWrapper.addStandardObjectiveFunction();
        double gurobiOptimum = modelWrapper.optimize();


        GreedyWithDynPathsSol solution = greedyDynPaths2dir(G, false);

        displayGraphPNGPostOpt(G, 1000, 800, &modelWrapper, "gurobi solution " + std::to_string(i));


        displayGraphPNGNodesNbsPicked(G, 1000, 800, solution.picked_nodes, solution.picked_nbs, "greedy solution " + std::to_string(i) );

        std::cout << "########################################################\n";
        std::cout << "The gurobi optimal value is: " << gurobiOptimum << std::endl;
        std::cout <<"The greedy solution value is: " << solution.value << std::endl;
        allGreedyCorrect = allGreedyCorrect && checkSolution(G,solution,false);
        std::cout << "########################################################\n";

        gurobiResults.push_back(gurobiOptimum);
        greedyResults.push_back(solution.value);
    }

    std::cout << "Results:\n";
    for(int round = 0; round<nr_comparisons; round++){
        std::cout << "Round "<<round<<":\n";
        std::cout << "Gurobi: " << gurobiResults[round] << std::endl;
        std::cout << "Greedy: " << greedyResults[round] << std::endl;
    }
}

void testGreedy(int nr_graphs, unsigned int seed, bool twoDir){

    srand(seed);

    bool allGreedyCorrect = true;

    for(int i=0; i<nr_graphs; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

//        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));


        GreedyWithDynPathsSol solution = greedyDynPaths2dir(G, false);



//        displayGraphPNGNodesNbsPicked(G, 1000, 800, solution.picked_nodes, solution.picked_nbs, "greedy solution " + std::to_string(i) );

        std::cout << "########################################################\n";
        std::cout <<"The greedy solution value is: " << solution.value << std::endl;
        bool algorithm_correct = checkSolution(G,solution,twoDir);
        if(!algorithm_correct){
            std::cout << "Oh no, the greedy implementation for graph " << i << " is incorrect!\n";
        } else {
            std::cout << "Implementation seems to be correct.\n";
        }
        std::cout << "########################################################\n";

    }

}

int main(){

    testGreedy1dirWithPrintingIterations();
//    greedy1dirVsGurobi(20,12);
//    testGreedy1dirInfinite(0);
//    testGreedyVsGurobiMessedUp();
//    testGraphGeneration();
//    greedy2dirVsGurobi(20,12);

//    testGreedy(100,10,true);
    return 0;
}