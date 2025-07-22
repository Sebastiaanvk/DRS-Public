#include "../include/algorithmWrappers.h"


namespace Algorithms{

    const double MAX_TIMEOUT = 600;

    bool checkSolution(const Solution& sol, const MultipartiteSetGraph& mpgraph, bool twoDir){
        bool properSolution = true;

        std::unique_ptr<EdgeIterator> it = mpgraph.getEdgeIterator();
        int nr_layers = mpgraph.getLayers();
        float sum_of_weights = 0;

        std::set<VertexIndex> hasForwardPath;
        std::set<VertexIndex> hasBackwardPath;

        for(auto vi : sol.pickedNodes){
            sum_of_weights += mpgraph.getValue(vi);
        }

        std::set<VertexIndex> neighbor_set;
        neighbor_set.insert(sol.pickedNodes.begin(),sol.pickedNodes.end());

        while(it->has_next()){
            Edge e = it->current();
            VertexIndex vi1 = e.first;
            VertexIndex vi2 = e.second;
            if(sol.pickedNodes.count(vi1) && sol.pickedNodes.count(vi2)){
                hasForwardPath.insert(vi1);
                hasBackwardPath.insert(vi2);
            } else if(sol.pickedNodes.count(vi1) || sol.pickedNodes.count(vi2)){
                VertexIndex node = vi1;
                VertexIndex nb = vi2;
                if(sol.pickedNodes.count(vi2)){
                    node = vi2;
                    nb = vi1;
                }

                if(node.layer>nb.layer || twoDir){
                neighbor_set.insert(nb);
                } 
            } 
            it->next();
        }
        for(auto vi:sol.pickedNodes){
            if(vi.layer<nr_layers-1 && hasForwardPath.count(vi)==0){
                std::cout << "Element at (" << vi.layer << ", " << vi.element << ") has no forward path!" << std::endl;
                properSolution =  false;
            }
            if(vi.layer>0 && hasBackwardPath.count(vi)==0){
                std::cout << "Element at (" << vi.layer << ", " << vi.element << ") has no backward path!" << std::endl;
                properSolution =  false;
            }
        }

        for(VertexIndex nb:neighbor_set){
            if(!sol.neighbors.count(nb)){
                std::cout << "The neighbor (" << nb.layer << ", " << nb.element << ") is not in the neighbor set!" << std::endl;
                properSolution = false;
            }
        }


        if(neighbor_set!=sol.neighbors){
            std::cout << "The neighbor set is not equal!" << std::endl;
            properSolution = false;
        }
        if(properSolution){
            std::cout << "The node and neighbor set appear to be correct. The given value is: " << sol.objectiveValue << std::endl;; 
        }
        return properSolution;
    }



    Solution altGurobiUni(const MultipartiteSetGraph& graph){
        ProcessedGraph pGraph = minimalProcessing(graph, false);
        GurobiAltSolution altSol = altGurobiOptimize(pGraph,MAX_TIMEOUT);
        return {altSol.value,altSol.nodes,altSol.neighbors,altSol.solvedToCompletion};
    }

    Solution altGurobiBi(const MultipartiteSetGraph& graph){
        ProcessedGraph pGraph = minimalProcessing(graph, true);
        GurobiAltSolution altSol = altGurobiOptimize(pGraph,MAX_TIMEOUT);
        return {altSol.value,altSol.nodes,altSol.neighbors,altSol.solvedToCompletion};
    }


    Solution dynSinglePathUni(const MultipartiteSetGraph& graph){
        SinglePathDynSol sol = singlePathDyn1DirNbs(graph);
        return {sol.max_value,sol.picked_nodes,sol.picked_neighbors,true};
    }
    Solution dynSinglePathBi(const MultipartiteSetGraph& graph){
        SinglePathDynSol sol = singlePathDynNbs(graph);
        return {sol.max_value,sol.picked_nodes,sol.picked_neighbors,true};
    }

    Solution greedySinglePathsUni(const MultipartiteSetGraph& graph){
        GreedyWithDynPathsSol sol = greedyDynPaths1dir(graph,false);
        return {sol.value,sol.picked_nodes,sol.picked_nbs,true};
    }

    Solution greedySinglePathsBi(const MultipartiteSetGraph& graph){
        GreedyWithDynPathsSol sol = greedyDynPaths2dir(graph,false);
        return {sol.value,sol.picked_nodes,sol.picked_nbs,true};
    }

    Solution greedyPeelingUni(const MultipartiteSetGraph& graph){

        MPGraphAsGreedyGraph G =  mpGraphToGreedyGraph(&graph, false);
        std::vector<float> acc_neigbor_values;
        GreedySolution sol= greedyAlgorithm(G.greedyGraph, acc_neigbor_values);
        std::set<VertexIndex> nodes;
        std::set<VertexIndex> neighbors;
        for(auto v:sol.nodes){
            nodes.insert(G.intToVertexIndex[v]);
        }
        for(auto v:sol.nbs){
            neighbors.insert(G.intToVertexIndex[v]);
        }
       
        return {sol.value,nodes,neighbors,true};
    }
    Solution greedyPeelingBi(const MultipartiteSetGraph& graph){
        MPGraphAsGreedyGraph G =  mpGraphToGreedyGraph(&graph, true);
        std::vector<float> acc_neigbor_values;
        GreedySolution sol= greedyAlgorithm(G.greedyGraph, acc_neigbor_values);
        std::set<VertexIndex> nodes;
        std::set<VertexIndex> neighbors;
        for(auto v:sol.nodes){
            nodes.insert(G.intToVertexIndex[v]);
        }
        for(auto v:sol.nbs){
            neighbors.insert(G.intToVertexIndex[v]);
        }
        return {sol.value,nodes,neighbors,true};
    }


    Solution solutionPickEverything(const MPGraphAsGreedyGraph& G){
        std::list<Vertex> unreachableList = find_unreachables(G.greedyGraph);
        std::set<Vertex> unreachables(unreachableList.begin(),unreachableList.end());

        std::set<VertexIndex> pickedNodes;
        std::set<VertexIndex> pickedNeighbors;
        float objectiveNum = 0;

        for(Vertex v=0; v<G.greedyGraph.nr_nodes; v++){
            if(unreachables.count(v)==0){
                objectiveNum += G.greedyGraph.node_values[v];
                pickedNodes.insert(G.intToVertexIndex[v]);
                for(auto nb : G.greedyGraph.node_to_nbs[v]){
                    pickedNeighbors.insert(G.intToVertexIndex[nb]);
                }
            } 
        }
        if(pickedNeighbors.size()==0){
            return {0,pickedNodes,pickedNeighbors, true};
 
        }

        return {objectiveNum/pickedNeighbors.size(),pickedNodes,pickedNeighbors, true};
    }


    Solution baseLinePickEverythingUni(const MultipartiteSetGraph& graph){
        MPGraphAsGreedyGraph G =  mpGraphToGreedyGraph(&graph, false);
        return solutionPickEverything(G);
    }

    Solution baseLinePickEverythingBi(const MultipartiteSetGraph& graph){
        MPGraphAsGreedyGraph G =  mpGraphToGreedyGraph(&graph, true);
        return solutionPickEverything(G);
    }

}