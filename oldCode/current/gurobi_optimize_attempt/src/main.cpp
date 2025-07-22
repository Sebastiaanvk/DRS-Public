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
#include "../include/gurobiAlgorithms.h"

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
    int max_layers = 13;
    int min_elements = 5;
    int max_elements = 18;
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


void newVsOldGurobi(int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<float> oldResults;
    std::vector<std::chrono::duration<double>> oldTimes;
    std::vector<float> newResults;
    std::vector<std::chrono::duration<double>> newTimes;

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

//        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));

        auto startOld = std::chrono::high_resolution_clock::now();

        GurobiModelWrapper modelWrapper(G, false, initialEnvironment());

        modelWrapper.turnOffOutput();
        modelWrapper.addBackwardNeighborConstraints();
//        modelWrapper.addForwardNeighborConstraints();
        modelWrapper.addPathConstraints();
        modelWrapper.addStandardObjectiveFunction();
        double gurobiOptimum = modelWrapper.optimize();

        auto endOld = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedOld = endOld-startOld;

        std::cout << "########################################################\n";
        std::cout << "The old gurobi optimal value is: " << gurobiOptimum << std::endl;


        auto startNew = std::chrono::high_resolution_clock::now();
        double newGurobiOptimum = new_gurobi_test_1dir_direct(G,false);
        auto endNew = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedNew = endNew-startNew;

        std::cout <<"The new gurobi optimal value is: " << newGurobiOptimum << std::endl;
        std::cout << "########################################################\n";

        oldResults.push_back(gurobiOptimum);
        oldTimes.push_back(elapsedOld);
        newResults.push_back(newGurobiOptimum);
        newTimes.push_back(elapsedNew);
    }

    std::cout << "Results:\n";
    for(int round = 0; round<nr_comparisons; round++){
        std::cout << "Round "<<round<<":\n";
        std::cout << "Old Gurobi: " << oldResults[round] << " and it took: " << oldTimes[round].count() << std::endl;
        std::cout << "New Gurobi: " << newResults[round] <<  " and it took: " << newTimes[round].count()  <<  std::endl;
    }
}

void testNewGurobi(unsigned int seed){
    srand(seed);

    RandomInput ri = generate_input_par();

    MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

    double newGurobiOptimum = new_gurobi_test_1dir_direct(G,true);

    std::cout << "Optimal value: " << newGurobiOptimum << std::endl;


}



void testingPriorityValues( int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<float> noPrioResults;
    std::vector<std::chrono::duration<double>> noPrioTimes;
    std::vector<float> withPrioResults;
    std::vector<std::chrono::duration<double>> withPrioTimes;

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

//        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));

        auto startNoPrio = std::chrono::high_resolution_clock::now();

        double noPrioOptimum = new_gurobi_test_1dir_direct(G,false);


        auto endNoPrio = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedNoPrio = endNoPrio-startNoPrio;

        auto startPrio = std::chrono::high_resolution_clock::now();
        double withPrioOptimum = priority_test(G,false) ;
        auto endPrio = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedPrio = endPrio-startPrio;

        noPrioResults.push_back(noPrioOptimum);
        noPrioTimes.push_back(elapsedNoPrio);
        withPrioResults.push_back(withPrioOptimum);
        withPrioTimes.push_back(elapsedPrio);
    }

    std::cout << "Results:\n";
    for(int round = 0; round<nr_comparisons; round++){
        std::cout << "Round "<<round<<":\n";
        std::cout << "Prioriy values Gurobi: " << noPrioResults[round] << " and it took: " << noPrioTimes[round].count() << std::endl;
        std::cout << "No priority values Gurobi: " << withPrioResults[round] <<  " and it took: " << withPrioTimes[round].count()  <<  std::endl;
    }
}


void preprocessingTest( unsigned int seed, int iterations){
    srand(seed);
    for(int i=0; i<iterations; i++){
        RandomInput ri = generate_input_par();
        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

        displayGraphPNG(G, 1000,800, "Initial_graph_" + std::to_string(i));
    
        ProcessedGraph graph = properPreprocessing(G,false);

        displayGraphPNGNodesNbsPicked(G, 1000, 800, graph.nodes, graph.neighbors, "Processed_graph_" + std::to_string(i));
    }
}



void preprocessingVsNormal(int nr_comparisons, unsigned int seed ){
    srand(seed);

    std::vector<float> normalResults;
    std::vector<std::chrono::duration<double>> normalTimes;
    std::vector<float> newResults;
    std::vector<std::chrono::duration<double>> newTimes;

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

//        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));

        auto startNormal = std::chrono::high_resolution_clock::now();

        double normalOptimum = new_gurobi_test_1dir_direct(G,false);


        auto endNormal = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedNoPrio = endNormal-startNormal;

        auto startNew = std::chrono::high_resolution_clock::now();
        double newOptimum = warm_start_heuristic_test(G) ;
        auto endNew = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedNew = endNew-startNew;

        normalResults.push_back(normalOptimum);
        normalTimes.push_back(elapsedNoPrio);
        newResults.push_back(newOptimum);
        newTimes.push_back(elapsedNew);
    }

    std::cout << "Results:\n";
    for(int round = 0; round<nr_comparisons; round++){
        std::cout << "Round "<<round<<":\n";
        std::cout << "Normal Gurobi: " << normalResults[round] << " and it took: " << normalTimes[round].count() << std::endl;
        std::cout << "With preprocessing Gurobi: " << newResults[round] <<  " and it took: " << newTimes[round].count()  <<  std::endl;
    }

}


void gurobi_with_heuristic_test(int nr_comparisons, unsigned int seed){
    srand(seed);

    std::vector<float> normalResults;
    std::vector<std::chrono::duration<double>> normalTimes;
    std::vector<float> newResults;
    std::vector<std::chrono::duration<double>> newTimes;

    for(int i=0; i<nr_comparisons; i++){
        RandomInput ri = generate_input_par();
        std::cout << "\ngraph " << i << ":" <<std::endl;
        print_graph_input(ri);

        MultipartiteSetGraph G = generate_random_graph(ri.nr_layers, ri.elementsPerLayer, ri.max_weight,ri.probZeroValue, ri.edge_density, ri.seed);

//        displayGraphPNG(G, 1000,800, "initial graph " + std::to_string(i));

        auto startNormal = std::chrono::high_resolution_clock::now();

        double normalOptimum = new_gurobi_test_1dir_direct(G,false);


        auto endNormal = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedNoPrio = endNormal-startNormal;

        auto startNew = std::chrono::high_resolution_clock::now();
        double newOptimum = preprocessingTest(G,false) ;
        auto endNew = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedNew = endNew-startNew;

        normalResults.push_back(normalOptimum);
        normalTimes.push_back(elapsedNoPrio);
        newResults.push_back(newOptimum);
        newTimes.push_back(elapsedNew);
    }

    std::cout << "Results:\n";
    for(int round = 0; round<nr_comparisons; round++){
        std::cout << "Round "<<round<<":\n";
        std::cout << "Normal Gurobi: " << normalResults[round] << " and it took: " << normalTimes[round].count() << std::endl;
        std::cout << "With preprocessing Gurobi: " << newResults[round] <<  " and it took: " << newTimes[round].count()  <<  std::endl;
    }

}

int main(){

//    testGreedy1dirWithPrintingIterations();
//    greedy1dirVsGurobi(20,12);
//    testGreedy1dirInfinite(0);
//    testGreedyVsGurobiMessedUp();
//    testGraphGeneration();
//    greedy2dirVsGurobi(20,12);

//    testGreedy(100,10,true);

//    newVsOldGurobi(10,11);
//    testNewGurobi(10);
    testingPriorityValues(10,10000);

//    preprocessingTest(100, 10);
//    preprocessingVsNormal(15,10);

//    gurobi_with_heuristic_test(20,10);

    return 0;
}